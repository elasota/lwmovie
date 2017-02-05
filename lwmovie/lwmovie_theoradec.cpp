#include "lwmovie.h"

#ifndef LWMOVIE_NO_THEORA

#include <string.h>
#include <new>

#include <theora/theoradec.h>
#include <limits.h>

#include "lwmovie_theoradec.hpp"
#include "lwmovie.h"

namespace lwmovie
{
	namespace theora
	{
		class CTheoraDecoderInternal
		{
		public:
			explicit CTheoraDecoderInternal(const ogg_allocator *alloc);
			~CTheoraDecoderInternal();

			ogg_int64_t IncrementPacket();
			bool InitDecoder();

			th_info &GetInfo();
			th_comment &GetComment();
			th_setup_info **GetSetupPtr();
			th_setup_info *GetSetup();
			th_dec_ctx *GetContext();

		private:
			th_info m_info;
			th_comment m_comment;
			th_setup_info *m_setup;
			th_dec_ctx *m_ctx;
			ogg_int64_t m_packetCounter;
		};
	}
}

lwmovie::theora::CTheoraDecoder::CTheoraDecoder(lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, lwmEFrameFormat frameFormat,
	lwmMovieState *movieState, lwmSWorkNotifier *workNotifier, bool useThreadedDeslicer)
	: m_alloc(alloc)
	, m_width(width)
	, m_height(height)
	, m_frameFormat(frameFormat)
	, m_movieState(movieState)
	, m_workNotifier(workNotifier)
	, m_useThreadedDeslicer(useThreadedDeslicer)
	, m_dropAggressiveness(lwmDROPAGGRESSIVENESS_None)
	, m_mustCheckHeader(true)
	, m_frameWasDropped(false)
	, m_internal(NULL)
	, m_recon(NULL)
{
}

lwmovie::theora::CTheoraDecoder::~CTheoraDecoder()
{
	if (m_internal)
	{
		m_internal->~CTheoraDecoderInternal();
		m_alloc->Free(m_internal);
		m_internal = NULL;
	}
}

bool lwmovie::theora::CTheoraDecoder::DigestStreamParameters(const void *bytes, lwmUInt32 packetSize)
{
	m_internal = m_alloc->NAlloc<CTheoraDecoderInternal>(1);
	if (!m_internal)
		return false;

	ogg_allocator alloc;
	alloc.reallocfunc = Realloc;
	alloc.ctx = this;

	new (m_internal) CTheoraDecoderInternal(&alloc);

	if (packetSize < 1)
		return false;
	
	const lwmUInt8 *tbytes = static_cast<const lwmUInt8*>(bytes);
	lwmUInt8 numPackets = tbytes[0];
	lwmUInt32 startLoc = 1 + numPackets * 4;

	if (packetSize < startLoc)
		return false;

	const lwmUInt8 *catalogStart = tbytes + 1;
	const lwmUInt8 *payloadStart = tbytes + startLoc;

	lwmUInt32 maxSize = 0xffffffff;

	long lmax = LONG_MAX;
	if (sizeof(lmax) > sizeof(lwmUInt32))
		maxSize = static_cast<lwmUInt32>(LONG_MAX);

	lwmUInt32 offset = startLoc;
	for (lwmUInt8 i = 0; i < numPackets; i++)
	{
		const lwmUInt8 *catalogEntry = catalogStart + i * 4;
		lwmUInt32 size = (catalogEntry[0] | (catalogEntry[1] << 8) | (catalogEntry[2] << 16) | (catalogEntry[3] << 24));

		if (packetSize - offset < size || size > maxSize)
			return false;

		ogg_packet pkt;
		pkt.bytes = static_cast<long>(size);
		pkt.b_o_s = ((i == 0) ? 1 : 0);
		pkt.e_o_s = 0;
		pkt.granulepos = 0;
		pkt.packetno = m_internal->IncrementPacket();
		pkt.packet = const_cast<lwmUInt8*>(tbytes + offset);

		int result = th_decode_headerin(&m_internal->GetInfo(), &m_internal->GetComment(), m_internal->GetSetupPtr(), &pkt);

		if (result <= 0)
			return false;	// Shouldn't return 0 (non-header) or an error;

		offset += size;
	}

	const th_info &info = m_internal->GetInfo();
	if (info.frame_width != m_width || info.frame_height != m_height)
		return false;

	if (info.pixel_fmt == TH_PF_420)
	{
		if (m_frameFormat != lwmFRAMEFORMAT_8Bit_420P_Planar)
			return false;
	}
	else if (info.pixel_fmt == TH_PF_444)
	{
		if (m_frameFormat != lwmFRAMEFORMAT_8Bit_3Channel_Planar)
			return false;
	}
	else
		return false;

	return true;
}

bool lwmovie::theora::CTheoraDecoder::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, lwmUInt32 *outResult)
{
	if (packetSize < 9)
		return false;

	const lwmUInt8 *tbytes = static_cast<const lwmUInt8*>(bytes);

	ogg_int64_t granulePos = 0;
	for (int i = 0; i < 8; i++)
		granulePos |= (static_cast<ogg_int64_t>(tbytes[i]) << i);

	lwmUInt8 numPackets = tbytes[8] & 0x7f;
	lwmUInt32 startLoc = 9 + numPackets * 4;

	bool isEOS = ((tbytes[8] & 0x80) != 0);

	if (packetSize < startLoc)
		return false;

	const lwmUInt8 *catalogStart = tbytes + 9;
	const lwmUInt8 *payloadStart = tbytes + startLoc;

	lwmUInt32 maxSize = 0xffffffff;

	long lmax = LONG_MAX;
	if (sizeof(lmax) == sizeof(lwmUInt32))
	{
		if (static_cast<lwmUInt32>(LONG_MAX) < maxSize)
			maxSize = static_cast<lwmUInt32>(LONG_MAX);
	}
	else if (sizeof(lmax) > sizeof(lwmUInt32))
		maxSize = static_cast<lwmUInt32>(LONG_MAX);

	if (numPackets == 0)
	{
		*outResult = lwmDIGEST_Error;
		return false;
	}

	lwmUInt32 offset = startLoc;
	for (lwmUInt8 i = 0; i < numPackets; i++)
	{
		bool isLastPacket = (i == numPackets - 1);

		const lwmUInt8 *catalogEntry = catalogStart + i * 4;
		lwmUInt32 size = (catalogEntry[0] | (catalogEntry[1] << 8) | (catalogEntry[2] << 16) | (catalogEntry[3] << 24));

		if (packetSize - offset < size || size > maxSize)
		{
			*outResult = lwmDIGEST_Error;
			return false;
		}

		ogg_packet pkt;
		pkt.bytes = static_cast<long>(size);
		pkt.b_o_s = 0;
		pkt.e_o_s = (isEOS && isLastPacket);
		pkt.granulepos = granulePos;
		pkt.packetno = m_internal->IncrementPacket();
		pkt.packet = const_cast<lwmUInt8*>(tbytes + offset);

		if (m_mustCheckHeader)
		{
			int result = th_decode_headerin(&m_internal->GetInfo(), &m_internal->GetComment(), m_internal->GetSetupPtr(), &pkt);
			if (result != 0)
			{
				*outResult = lwmDIGEST_Error;
				return false;
			}

			if (!m_internal->InitDecoder())
			{
				*outResult = lwmDIGEST_Error;
				return false;
			}

			m_mustCheckHeader = false;
		}

		ogg_int64_t outGranPos;
		int decResult = th_decode_packetin(m_internal->GetContext(), &pkt, &outGranPos);

		if (decResult == TH_DUPFRAME)
		{
			if (!isLastPacket)
			{
				*outResult = lwmDIGEST_Error;
				return false;
			}
			m_frameWasDropped = true;
		}
		else if (decResult == 0)
		{
			if (!isLastPacket)
			{
				*outResult = lwmDIGEST_Error;
				return false;
			}
			m_frameWasDropped = false;
		}
		else
		{
			// Currently, there is no valid case where the library returns from decode without a frame
			*outResult = lwmDIGEST_Error;
			return false;
		}

		// TODO: Frame dropping

		offset += size;
	}

	// TODO: YUV444
	th_ycbcr_buffer planes;
	int planesOutResult = th_decode_ycbcr_out(m_internal->GetContext(), planes);

	lwmSVideoFrameProvider *frameProvider = m_recon->GetFrameProvider();
	frameProvider->lockWorkFrameFunc(frameProvider, 0, lwmVIDEOLOCK_Write_Only);

	for (lwmUInt32 i = 0; i < 3; i++)
	{
		lwmUInt32 outPitch;
		lwmUInt32 width = frameProvider->getWorkFramePlaneWidthFunc(frameProvider, i);
		lwmUInt32 height = frameProvider->getWorkFramePlaneHeightFunc(frameProvider, i);
		void *outPlane = frameProvider->getWorkFramePlaneFunc(frameProvider, 0, i, &outPitch);

		lwmUInt32 inPitch = static_cast<lwmUInt32>(planes[i].stride);

		const lwmUInt8 *inRow = planes[i].data;
		lwmUInt8 *outRow = static_cast<lwmUInt8*>(outPlane);
		for (lwmUInt32 row = 0; row < height; row++)
		{
			memcpy(outRow, inRow, width);
			inRow += inPitch;
			outRow += outPitch;
		}
	}


	frameProvider->unlockWorkFrameFunc(frameProvider, 0);

	return true;
}

void lwmovie::theora::CTheoraDecoder::WaitForDigestFinish()
{
}

bool lwmovie::theora::CTheoraDecoder::EmitFrame()
{
	return !m_frameWasDropped;
}

void lwmovie::theora::CTheoraDecoder::SetDropAggressiveness(lwmEDropAggressiveness dropAggressiveness)
{
}

void lwmovie::theora::CTheoraDecoder::Participate()
{
}

void lwmovie::theora::CTheoraDecoder::SetReconstructor(lwmIVideoReconstructor *videoRecon)
{
	m_recon = static_cast<lwmovie::theora::CSoftwareReconstructor*>(videoRecon);
}


void *lwmovie::theora::CTheoraDecoder::Realloc(void *ctx, void *ptr, size_t sz)
{
	CTheoraDecoder *dec = static_cast<CTheoraDecoder*>(ctx);
	return dec->m_alloc->reallocFunc(dec->m_alloc, ptr, sz);
}

////////////////////////////////////////////////////////////////////////////////

bool lwmovie::theora::CSoftwareReconstructor::Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState)
{
	lwmUInt32 width, height, numWFrames, numRWFrames, frameFormat;
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &width);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &height);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumReadWriteWorkFrames, &numRWFrames);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames, &numWFrames);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_VideoFrameFormat, &frameFormat);

	m_frontFrame = 0;
	m_alloc = alloc;
	m_frameProvider = frameProvider;
	m_width = (width + 15) / 16 * 16;
	m_height = (height + 15) / 16 * 16;

	if (numWFrames != 1 || numRWFrames != 0)
		return false;

	if (!m_frameProvider->createWorkFramesFunc(m_frameProvider, 0, 1, m_width, m_height, frameFormat))
		return false;

	return true;
}

lwmovie::theora::CSoftwareReconstructor::~CSoftwareReconstructor()
{
	this->WaitForFinish();
}

void lwmovie::theora::CSoftwareReconstructor::Participate()
{
}

void lwmovie::theora::CSoftwareReconstructor::WaitForFinish()
{
}

void lwmovie::theora::CSoftwareReconstructor::SetWorkNotifier(lwmSWorkNotifier *workNotifier)
{
}

void lwmovie::theora::CSoftwareReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
}

lwmUInt32 lwmovie::theora::CSoftwareReconstructor::GetWorkFrameIndex() const
{
	return 0;
}

void lwmovie::theora::CSoftwareReconstructor::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	this->~CSoftwareReconstructor();
	alloc->Free(this);
}

lwmSVideoFrameProvider *lwmovie::theora::CSoftwareReconstructor::GetFrameProvider() const
{
	return m_frameProvider;
}

////////////////////////////////////////////////////////////////////////////////

lwmovie::theora::CTheoraDecoderInternal::CTheoraDecoderInternal(const ogg_allocator *alloc)
	: m_ctx(NULL)
	, m_setup(NULL)
	, m_packetCounter(0)
{
	th_info_init(&m_info, alloc);
	th_comment_init(&m_comment, alloc);
}

lwmovie::theora::CTheoraDecoderInternal::~CTheoraDecoderInternal()
{
	if (m_ctx)
		th_decode_free(m_ctx);

	if (m_setup)
		th_setup_free(m_setup);

	th_comment_clear(&m_comment);
	th_info_clear(&m_info);
}

ogg_int64_t lwmovie::theora::CTheoraDecoderInternal::IncrementPacket()
{
	return ++m_packetCounter;
}

bool lwmovie::theora::CTheoraDecoderInternal::InitDecoder()
{
	m_ctx = th_decode_alloc(&m_info, m_setup);
	th_setup_free(m_setup);

	m_setup = NULL;

	return m_ctx != NULL;
}

th_info &lwmovie::theora::CTheoraDecoderInternal::GetInfo()
{
	return m_info;
}

th_comment &lwmovie::theora::CTheoraDecoderInternal::GetComment()
{
	return m_comment;
}

th_setup_info **lwmovie::theora::CTheoraDecoderInternal::GetSetupPtr()
{
	return &this->m_setup;
}

th_setup_info *lwmovie::theora::CTheoraDecoderInternal::GetSetup()
{
	return this->m_setup;
}

th_dec_ctx *lwmovie::theora::CTheoraDecoderInternal::GetContext()
{
	return this->m_ctx;
}

#endif
