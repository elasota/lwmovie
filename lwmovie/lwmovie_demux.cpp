/*
 * Copyright (c) 2014 Eric Lasota
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <new>
#include "lwmovie.h"
#include "lwmovie_fillable.hpp"
#include "lwmovie_package.hpp"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_recon_m1vsw.hpp"
#include "../mp2dec2/lwmovie_layer2_decodestate.hpp"

namespace lwmovie
{
	class lwmVidStream;
	class lwmCMP2Decoder;
}


static void lwmPassThroughDigestNotifyAvailable(void *opaque)
{
	lwmMovieState_VideoDigestParticipate(static_cast<lwmMovieState*>(opaque));
}

static void lwmPassThroughJoin(void *opaque)
{
}

enum lwmDemuxState
{
	lwmDEMUX_MovieHeader,
	lwmDEMUX_AudioHeader,
	lwmDEMUX_VideoHeader,
	lwmDEMUX_InitDecoding,
	lwmDEMUX_PacketHeader,
	lwmDEMUX_PacketHeaderFull,
	lwmDEMUX_PacketHeaderCompact,
	lwmDEMUX_PacketStart,
	lwmDEMUX_PacketData,

	lwmDEMUX_FatalError
};

class lwmCAudioBuffer
{
public:
	explicit lwmCAudioBuffer(lwmSAllocator *alloc);
	~lwmCAudioBuffer();
	bool Init(lwmSAllocator *alloc, lwmUInt32 numSamples, lwmUInt8 numChannels);
	lwmUInt8 GetNumChannels() const;
	lwmUInt32 GetNumCommittedSamples() const;
	lwmUInt32 GetNumUncommittedSamples() const;
	void CommitSamples();
	void ClearAll();
	void ClearCommittedSamples();
	lwmUInt32 ReadCommittedSamples(void *output, lwmUInt32 numSamples);

	// Reserves a contiguous block of samples at the end of the ring.  Never fails if the buffer is large enough, may delete samples.
	void *ReserveNewContiguous(lwmUInt32 numSamples, lwmUInt32 &outNumDroppedSamples);
	void SkipSamples(lwmUInt32 numSamples);

private:
	// Synchronization info
	lwmUInt32 m_startPeriod;
	lwmUInt32 m_capacity;

	lwmUInt32 m_topOffset;
	lwmUInt32 m_topSize;
	lwmUInt32 m_bottomOffset;
	lwmUInt32 m_bottomSize;

	lwmUInt32 m_numCommittedSamples;

	lwmSAllocator *m_alloc;
	void *m_samples;
	lwmUInt8 m_numChannels;
	lwmUInt8 m_sampleSizeBytes;

	void *SampleMemAtPos(lwmUInt32 pos);
};

lwmCAudioBuffer::lwmCAudioBuffer(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_startPeriod(0)
	, m_topOffset(0)
	, m_topSize(0)
	, m_bottomOffset(0)
	, m_bottomSize(0)
	, m_numChannels(0)
	, m_sampleSizeBytes(0)
	, m_samples(NULL)
	, m_capacity(0)
	, m_numCommittedSamples(0)
{
}

lwmCAudioBuffer::~lwmCAudioBuffer()
{
	if(m_samples)
		m_alloc->freeFunc(m_alloc, m_samples);
}


bool lwmCAudioBuffer::Init(lwmSAllocator *alloc, lwmUInt32 numSamples, lwmUInt8 numChannels)
{
	lwmUInt32 intMax = ~static_cast<lwmUInt32>(0);
	m_sampleSizeBytes = 2 * numChannels;
	if(intMax / m_sampleSizeBytes < numSamples)
		return false;	// Overflow
	m_alloc = alloc;
	m_samples = alloc->allocFunc(alloc, m_sampleSizeBytes * numSamples);
	m_numChannels = numChannels;
	m_capacity = numSamples;
	if(m_samples == NULL)
		return false;
	return true;
}

lwmUInt8 lwmCAudioBuffer::GetNumChannels() const
{
	return m_numChannels;
}

lwmUInt32 lwmCAudioBuffer::GetNumCommittedSamples() const
{
	return m_numCommittedSamples;
}

lwmUInt32 lwmCAudioBuffer::GetNumUncommittedSamples() const
{
	return m_topSize + m_bottomSize - m_numCommittedSamples;
}

void lwmCAudioBuffer::CommitSamples()
{
	m_numCommittedSamples = m_topSize + m_bottomSize;
}

void lwmCAudioBuffer::ClearAll()
{
	m_numCommittedSamples = m_topSize = m_bottomSize = 0;
}

void lwmCAudioBuffer::ClearCommittedSamples()
{
	SkipSamples(m_numCommittedSamples);
}

lwmUInt32 lwmCAudioBuffer::ReadCommittedSamples(void *output, lwmUInt32 numSamples)
{
	if(m_bottomOffset > m_capacity || m_topOffset > m_capacity || m_bottomOffset + m_bottomSize > m_capacity || m_topOffset + m_topSize > m_capacity)
	{
		__asm { int 3 }
	}

	if(numSamples > m_numCommittedSamples)
		numSamples = m_numCommittedSamples;
	m_numCommittedSamples -= numSamples;

	lwmUInt32 numTopDecoded = 0;
	if(m_topSize)
	{
		const void *topMem = SampleMemAtPos(m_topOffset);
		if(m_topSize >= numSamples)
		{
			m_topSize -= numSamples;
			m_topOffset += numSamples;

			memcpy(output, topMem, m_sampleSizeBytes * numSamples);
			return numSamples;
		}
		else
		{
			memcpy(output, topMem, m_sampleSizeBytes * m_topSize);
			numTopDecoded = m_topSize;
			numSamples -= m_topSize;
			output = static_cast<lwmUInt8*>(output) + m_topSize * m_sampleSizeBytes;
			m_topSize = 0;
		}
	}

	if(m_bottomSize)
	{
		const void *bottomMem = SampleMemAtPos(m_bottomOffset);
		if(m_bottomSize >= numSamples)
		{
			m_bottomSize -= numSamples;
			m_bottomOffset += numSamples;
			memcpy(output, bottomMem, m_sampleSizeBytes * numSamples);
			return numTopDecoded + numSamples;
		}
		else
		{
			memcpy(output, bottomMem, m_sampleSizeBytes * m_bottomSize);
			lwmUInt32 numBottomDecoded = m_bottomSize;
			m_bottomSize = 0;
			return numBottomDecoded + numTopDecoded;
		}
	}
	else
	{
		return numTopDecoded;
	}
}

void *lwmCAudioBuffer::ReserveNewContiguous(lwmUInt32 numSamples, lwmUInt32 &outNumDroppedSamples)
{
	if(m_bottomOffset > m_capacity || m_topOffset > m_capacity || m_bottomOffset + m_bottomSize > m_capacity || m_topOffset + m_topSize > m_capacity)
	{
		__asm { int 3 }
	}
	if(numSamples > m_capacity)
		return NULL;

	lwmUInt32 available = m_capacity - m_topSize - m_bottomSize;
	if(available < numSamples)
	{
		lwmUInt32 numDropped = numSamples - available;
		if(numDropped >= m_numCommittedSamples)
			m_numCommittedSamples = 0;
		else
			m_numCommittedSamples -= numDropped;
		SkipSamples(numDropped);
		outNumDroppedSamples = numDropped;
	}
	else
	{
		outNumDroppedSamples = 0;
	}

	if(m_bottomSize == 0)
		m_bottomOffset = 0;

	if(m_topSize == 0)
	{
		// Single chunk in the ring buffer
		// See if there is space at the end of bottom
		if(m_capacity - m_bottomOffset - m_bottomSize >= numSamples)
		{
			// Reserve at the end of bottom
			void *loc = SampleMemAtPos(m_bottomOffset + m_bottomSize);
			m_bottomSize += numSamples;
			return loc;
		}
		// See if there is space below bottom, which would cause bottom to become top
		if(m_bottomOffset >= numSamples)
		{
			m_topOffset = m_bottomOffset;
			m_topSize = m_bottomSize;
			m_bottomOffset = 0;
			m_bottomSize = numSamples;

			return m_samples;
		}

		// Move the bottom to zero
		memmove(m_samples, SampleMemAtPos(m_bottomOffset), m_bottomSize * m_sampleSizeBytes);
		m_bottomOffset = 0;
		void *loc = SampleMemAtPos(0 + m_bottomSize);
		m_bottomSize += numSamples;
		return loc;
	}
	else
	{
		// 2 chunks in the ring buffer
		// See if there is space between the chunks
		if(m_topOffset - m_bottomOffset - m_bottomSize)
		{
			void *loc = SampleMemAtPos(m_bottomOffset + m_bottomSize);
			m_bottomSize += numSamples;
			return loc;
		}
		// There isn't, need to relocate both chunks to the edges
		// TODO: Might be able to merge by moving top below bottom if the reserve fits above
		memmove(m_samples, SampleMemAtPos(m_bottomOffset), m_bottomSize * m_sampleSizeBytes);
		m_bottomOffset = 0;
		memmove(SampleMemAtPos(m_capacity - m_topSize), SampleMemAtPos(m_topOffset), m_topSize * m_sampleSizeBytes);
		m_topOffset = m_capacity - m_topSize;
		void *loc = SampleMemAtPos(0 + m_bottomSize);
		m_bottomSize += numSamples;
		return loc;
	}
}

void *lwmCAudioBuffer::SampleMemAtPos(lwmUInt32 pos)
{
	return static_cast<lwmUInt8*>(m_samples) + pos * m_sampleSizeBytes;
}

void lwmCAudioBuffer::SkipSamples(lwmUInt32 numSamples)
{
	if(numSamples >= m_numCommittedSamples)
		m_numCommittedSamples = 0;
	else
		m_numCommittedSamples -= numSamples;

	if(m_topSize >= numSamples)
	{
		m_topOffset += numSamples;
		m_topSize -= numSamples;
		return;
	}
	numSamples -= m_topSize;
	m_topSize = 0;

	if(m_bottomSize >= numSamples)
	{
		m_bottomOffset += numSamples;
		m_bottomSize -= numSamples;
		return;
	}
	else
	{
		// TODO: Overrun?
		m_bottomOffset = 0;
		m_bottomSize = 0;
	}
}

class lwmovie::lwmCMP2Decoder
{
public:
	explicit lwmCMP2Decoder(lwmSAllocator *alloc);
	~lwmCMP2Decoder();
	bool Init(const lwmAudioStreamInfo *audioStreamInfo);
	bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
	lwmCAudioBuffer *GetAudioBuffer();

private:
	lwmSAllocator *m_alloc;
	lwmUInt8 m_numChannels;
	lwmovie::layerii::lwmCMP2DecodeState m_decodeState;
	lwmUInt8 m_frameData[lwmovie::layerii::MAX_FRAME_SIZE_BYTES];
	lwmCAudioBuffer m_audioBuffer;
};

lwmovie::lwmCMP2Decoder::lwmCMP2Decoder(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_numChannels(0)
	, m_audioBuffer(alloc)
{
	memset(m_frameData, 0, sizeof(m_frameData));
}

lwmovie::lwmCMP2Decoder::~lwmCMP2Decoder()
{
}

bool lwmovie::lwmCMP2Decoder::Init(const lwmAudioStreamInfo *audioStreamInfo)
{
	if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Mono)
		m_numChannels = 1;
	else if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
		m_numChannels = 2;
	else
		return false;	// Unsupported channel layout

	if(!this->m_audioBuffer.Init(m_alloc, audioStreamInfo->audioReadAhead, m_numChannels))
		return false;

	return true;
}

bool lwmovie::lwmCMP2Decoder::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun)
{
	outOverrun = false;
	if(packetSize > lwmovie::layerii::MAX_FRAME_SIZE_BYTES || packetSize < lwmovie::layerii::HEADER_SIZE_BYTES)
		return false;
	// TODO: If unsafe decodes are permitted, fast decode could skip this copy
	memcpy(m_frameData, bytes, packetSize);
	if(!m_decodeState.ParseHeader(m_frameData))
		return false;
	if(packetSize - lwmovie::layerii::HEADER_SIZE_BYTES != m_decodeState.GetFrameSizeBytes())
		return false;
	if(m_decodeState.GetChannelCount() != m_numChannels)
		return false;	// Real-time switch between mono/stereo not supported
	lwmUInt32 numDroppedSamples;
	void *outputBuf = m_audioBuffer.ReserveNewContiguous(lwmovie::layerii::FRAME_NUM_SAMPLES, numDroppedSamples);
	if(!outputBuf)
		return false;
	if(!m_decodeState.DecodeFrame(m_frameData + lwmovie::layerii::HEADER_SIZE_BYTES, static_cast<lwmSInt16*>(outputBuf)))
		return false;
	if(numDroppedSamples)
		outOverrun = true;
	return true;
}

lwmCAudioBuffer *lwmovie::lwmCMP2Decoder::GetAudioBuffer()
{
	return &m_audioBuffer;
}

struct lwmMovieState
{
	lwmMovieHeader movieInfo;
	lwmVideoStreamInfo videoInfo;
	lwmAudioStreamInfo audioInfo;
	lwmPacketHeader packetInfo;

	lwmUInt8 pkgHeaderBytes[lwmPlanHandler<lwmMovieHeader>::SIZE];
	lwmUInt8 videoInfoBytes[lwmPlanHandler<lwmVideoStreamInfo>::SIZE];
	lwmUInt8 audioInfoBytes[lwmPlanHandler<lwmAudioStreamInfo>::SIZE];
	lwmUInt8 packetHeaderBytes[lwmPlanHandler<lwmPacketHeader>::SIZE];
	lwmUInt8 packetHeaderFullBytes[lwmPlanHandler<lwmPacketHeaderFull>::SIZE];
	lwmUInt8 packetHeaderCompactBytes[lwmPlanHandler<lwmPacketHeaderCompact>::SIZE];

	lwmUInt8 *packetDataBytesEscaped;
	lwmUInt8 *packetDataBytes;

	lwmUInt32 packetSize;
	lwmFillableBuffer headerFillable;
	lwmFillableBuffer videoFillable;
	lwmFillableBuffer audioFillable;

	lwmFillableBuffer packetHeaderFillable;
	lwmFillableBuffer packetHeaderFullFillable;
	lwmFillableBuffer packetHeaderCompactFillable;
	lwmFillableBuffer packetDataFillable;

	lwmDemuxState demuxState;

	lwmSAllocator *alloc;
	lwmUInt32 userFlags;

	bool isAudioSynchronized;

	lwmUInt32 streamSyncPeriods[lwmSTREAMTYPE_Count];

	lwmSWorkNotifier *videoDigestWorkNotifier;
	lwmIVideoReconstructor *videoReconstructor;
	lwmovie::lwmVidStream *m1vDecoder;
	lwmovie::lwmCMP2Decoder *mp2Decoder;

	lwmMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags);
	void DesyncAudio();
};

lwmMovieState::lwmMovieState(lwmSAllocator *pAlloc, lwmUInt32 pUserFlags)
	: demuxState(lwmDEMUX_MovieHeader)
	, packetDataBytesEscaped(NULL)
	, packetDataBytes(NULL)
	, userFlags(pUserFlags)
	, videoDigestWorkNotifier(NULL)
	, alloc(pAlloc)
	, mp2Decoder(NULL)
	, m1vDecoder(NULL)
	, isAudioSynchronized(false)
{
	this->headerFillable.Init(this->pkgHeaderBytes);
	this->videoFillable.Init(this->videoInfoBytes);
	this->audioFillable.Init(this->audioInfoBytes);
}

void lwmMovieState::DesyncAudio()
{
	this->isAudioSynchronized = false;
	if(this->mp2Decoder)
		this->mp2Decoder->GetAudioBuffer()->ClearAll();
}

static void UnescapePacket(const void *inBuffer, void *outBuffer, lwmUInt32 inSize, lwmUInt32 *pOutSize)
{
	lwmUInt32 outSize = 0;
	const lwmUInt8 *inBytes = static_cast<const lwmUInt8 *>(inBuffer);
	lwmUInt8 *outBytes = static_cast<lwmUInt8*>(outBuffer);

	if(inSize <= 3)
	{
		for(lwmUInt32 i=0;i<inSize;i++)
			outBytes[i] = inBytes[i];
		*pOutSize = inSize;
		return;
	}

	lwmUInt8 *outByteBase = outBytes;

	for(lwmUInt32 i=0;i<3;i++)
		outBytes[i] = inBytes[i];

	outSize = 3;
	lwmUInt32 prevBytes = (inBytes[0] << 16) | (inBytes[1] << 8) | inBytes[2];

	outBytes += 3;
	inBytes += 3;
	for(lwmUInt32 i=3;i<inSize;i++)
	{
		lwmUInt32 inByte = *inBytes++;
		prevBytes = (prevBytes << 8) | inByte;
		if(prevBytes != 0x000001fe)
			*outBytes++ = inByte;
	}

	*pOutSize = static_cast<lwmUInt32>(outBytes - outByteBase);
}

static void DigestPacket(lwmMovieState *movieState, lwmUInt32 *outResult)
{
	const void *packetData = movieState->packetDataBytesEscaped;
	lwmUInt32 packetSize = movieState->packetSize;
	if(movieState->packetInfo.packetTypeAndFlags & lwmPacketHeader::EFlag_Escaped)
	{
		UnescapePacket(packetData, movieState->packetDataBytes, movieState->packetSize, &packetSize);
		packetData = movieState->packetDataBytes;
	}

	switch(movieState->packetInfo.packetTypeAndFlags & ~(lwmPacketHeader::EFlag_All))
	{
	case lwmEPT_Video_StreamParameters:
		// TODO: Check vid decode type
		movieState->m1vDecoder->WaitForDigestFinish();
		movieState->videoReconstructor->WaitForFinish();
		// TODO: Detect errors if DigestStreamParameters fails
		movieState->m1vDecoder->DigestStreamParameters(packetData, packetSize);
		*outResult = lwmDIGEST_Worked;
		break;
	case lwmEPT_Video_InlinePacket:
		// TODO: Check vid decode type
		if(!movieState->m1vDecoder->DigestDataPacket(packetData, packetSize, outResult))
			*outResult = lwmDIGEST_Error;
		else
			*outResult = lwmDIGEST_Worked;
		break;
	case lwmEPT_Video_Synchronization:
		{
			// TODO: Check vid decode type
			lwmVideoSynchronizationPoint syncPoint;
			if(packetSize == lwmPlanHandler<lwmVideoSynchronizationPoint>::SIZE && 
				lwmPlanHandler<lwmVideoSynchronizationPoint>::Read(syncPoint, packetData))
			{
				if(movieState->m1vDecoder)
				{
					movieState->m1vDecoder->WaitForDigestFinish();
					movieState->m1vDecoder->EmitFrame();
				}
				movieState->videoReconstructor->WaitForFinish();
				movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] = syncPoint.videoPeriod;
				*outResult = lwmDIGEST_VideoSync;
			}
			else
			{
				*outResult = lwmDIGEST_Error;
			}
		}
		break;
	case lwmEPT_Audio_StreamParameters:
		*outResult = lwmDIGEST_Error;
		break;
	case lwmEPT_Audio_Frame:
		// TODO: Check stream type
		{
			bool overrun = false;
			bool mustDesync = false;
			if(!movieState->mp2Decoder->DigestDataPacket(packetData, packetSize, overrun))
			{
				*outResult = lwmDIGEST_Error;
				mustDesync = true;
			}
			else
				*outResult = lwmDIGEST_Worked;
			// Only need to desync if audio wasn't synchronized.  Overruns are permitted at the start to absorb algorithmic delay.
			if(overrun && movieState->isAudioSynchronized)
				mustDesync = true;
			if(mustDesync)
				movieState->DesyncAudio();
		}
		break;
	case lwmEPT_Audio_Synchronization:
		// TODO: Check stream type, enforce timing
		{
			// TODO: Check decode type
			lwmCAudioBuffer *audioBuffer = movieState->mp2Decoder->GetAudioBuffer();
			lwmAudioSynchronizationPoint syncPoint;
			if(packetSize == lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE && 
				lwmPlanHandler<lwmAudioSynchronizationPoint>::Read(syncPoint, packetData))
			{
				if(movieState->isAudioSynchronized)
				{
					lwmUInt32 oldPeriod = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio];
					if(syncPoint.audioPeriod < oldPeriod || syncPoint.audioPeriod - oldPeriod != audioBuffer->GetNumUncommittedSamples())
					{
						// Desynchronized, but new samples are still OK
						movieState->isAudioSynchronized = false;
						audioBuffer->ClearCommittedSamples();
					}
				}

				audioBuffer->CommitSamples();
				movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] = syncPoint.audioPeriod;
				*outResult = lwmDIGEST_Worked;
			}
			else
			{
				movieState->DesyncAudio();
				*outResult = lwmDIGEST_Error;
			}
		}
		*outResult = lwmDIGEST_Worked;
		break;
	default:
		*outResult = lwmDIGEST_Error;
		break;
	};
}

static bool InitDecoding(lwmSAllocator *alloc, lwmMovieState *movieState)
{
	switch(movieState->movieInfo.videoStreamType)
	{
	case lwmVST_None:
		// TODO: Test
		break;
	case lwmVST_M1V_Variant:
		if(movieState->videoInfo.videoWidth > 4095)
			goto cleanup;
		if(movieState->videoInfo.videoHeight > 4095)
			goto cleanup;
		movieState->m1vDecoder = static_cast<lwmovie::lwmVidStream*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmVidStream)));
		if(!movieState->m1vDecoder)
			goto cleanup;
		new (movieState->m1vDecoder) lwmovie::lwmVidStream(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight, movieState, movieState->videoDigestWorkNotifier, ((movieState->userFlags) & lwmUSERFLAG_ThreadedDeslicer));
		break;
	default:
		goto cleanup;
	};

	switch(movieState->movieInfo.audioStreamType)
	{
	case lwmAST_None:
		// TODO: Test
		break;
	case lwmAST_MP2:
		movieState->mp2Decoder = static_cast<lwmovie::lwmCMP2Decoder*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmCMP2Decoder)));
		if(!movieState->mp2Decoder)
			goto cleanup;
		new (movieState->mp2Decoder) lwmovie::lwmCMP2Decoder(alloc);
		if(!movieState->mp2Decoder->Init(&movieState->audioInfo))
			goto cleanup;
		break;
	default:
		goto cleanup;
	};

	return true;
cleanup:
	// TODO!!!
	return false;
}

LWMOVIE_API_LINK lwmMovieState *lwmCreateMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags)
{
	lwmMovieState *movieState = static_cast<lwmMovieState *>(alloc->allocFunc(alloc, sizeof(lwmMovieState)));

	if(!movieState)
		return NULL;
	new (movieState) lwmMovieState(alloc, userFlags);
	return movieState;
}

LWMOVIE_API_LINK void lwmMovieState_FeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested)
{
	lwmUInt32 bytesAvailable = numBytes;
	const void *bytes = inBytes;

repeatFeed:;

	switch(movieState->demuxState)
	{
	case lwmDEMUX_MovieHeader:
		{
			if(movieState->headerFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmMovieHeader>::Read(movieState->movieInfo, movieState->pkgHeaderBytes)
					&& movieState->movieInfo.largestPacketSize > 0)
				{
					if(movieState->movieInfo.videoStreamType != lwmVST_None)
					{
						movieState->demuxState = lwmDEMUX_VideoHeader;
						goto repeatFeed;
					}
					else if(movieState->movieInfo.audioStreamType != lwmAST_None)
					{
						movieState->demuxState = lwmDEMUX_AudioHeader;
						goto repeatFeed;
					}
					else
						movieState->demuxState = lwmDEMUX_FatalError;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_VideoHeader:
		{
			if(movieState->videoFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmVideoStreamInfo>::Read(movieState->videoInfo, movieState->videoInfoBytes))
				{
					if(movieState->movieInfo.audioStreamType != lwmAST_None)
					{
						movieState->demuxState = lwmDEMUX_AudioHeader;
						goto repeatFeed;
					}
					else
						movieState->demuxState = lwmDEMUX_InitDecoding;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_AudioHeader:
		{
			if(movieState->audioFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmAudioStreamInfo>::Read(movieState->audioInfo, movieState->audioInfoBytes))
				{
					movieState->demuxState = lwmDEMUX_InitDecoding;
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_InitDecoding:
		{
			movieState->packetDataBytesEscaped = static_cast<lwmUInt8*>(movieState->alloc->allocFunc(movieState->alloc, movieState->movieInfo.largestPacketSize));
			movieState->packetDataBytes = static_cast<lwmUInt8*>(movieState->alloc->allocFunc(movieState->alloc, movieState->movieInfo.largestPacketSize));
			movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

			if(movieState->packetDataBytesEscaped == NULL || movieState->packetDataBytes == NULL)
			{
				movieState->demuxState = lwmDEMUX_FatalError;
				*outResult = lwmDIGEST_Nothing;
			}
			else
			{
				movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

				if(InitDecoding(movieState->alloc, movieState))
				{
					movieState->demuxState = lwmDEMUX_PacketHeader;
					*outResult = lwmDIGEST_Initialize;
				}
				else
				{
					movieState->demuxState = lwmDEMUX_FatalError;
					*outResult = lwmDIGEST_Nothing;
				}
			}

			*outBytesDigested = numBytes - bytesAvailable;
		}
		break;
	case lwmDEMUX_PacketHeader:
		{
			if(movieState->packetHeaderFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmPacketHeader>::Read(movieState->packetInfo, movieState->packetHeaderBytes))
				{
					if(movieState->packetInfo.packetTypeAndFlags & lwmPacketHeader::EFlag_Compact)
					{
						movieState->packetHeaderCompactFillable.Init(movieState->packetHeaderCompactBytes);
						movieState->demuxState = lwmDEMUX_PacketHeaderCompact;
					}
					else
					{
						movieState->packetHeaderFullFillable.Init(movieState->packetHeaderFullBytes);
						movieState->demuxState = lwmDEMUX_PacketHeaderFull;
					}
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;	// TODO: Recovery
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
		}
		break;
	case lwmDEMUX_PacketStart:
		{
			if(movieState->packetSize > movieState->movieInfo.largestPacketSize || movieState->packetSize == 0)
				movieState->demuxState = lwmDEMUX_FatalError;	// TODO: Recovery
			else
			{
				movieState->packetDataFillable.Init(movieState->packetDataBytesEscaped, movieState->packetSize);
				movieState->demuxState = lwmDEMUX_PacketData;
				goto repeatFeed;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketHeaderCompact:
		{
			if(movieState->packetHeaderCompactFillable.FeedData(&bytes, &bytesAvailable))
			{
				lwmPacketHeaderCompact compactHeader;
				if(lwmPlanHandler<lwmPacketHeaderCompact>::Read(compactHeader, movieState->packetHeaderCompactBytes) &&
					(compactHeader.packetSize & 0x01) == 1 &&
					(compactHeader.packetSize & 0xfffe) != 0)
				{
					movieState->packetSize = compactHeader.packetSize >> 1;
					movieState->demuxState = lwmDEMUX_PacketStart;
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketHeaderFull:
		{
			if(movieState->packetHeaderFullFillable.FeedData(&bytes, &bytesAvailable))
			{
				lwmPacketHeaderFull fullHeader;
				if(lwmPlanHandler<lwmPacketHeaderFull>::Read(fullHeader, movieState->packetHeaderFullBytes))
				{
					movieState->packetSize = fullHeader.packetSize;
					movieState->demuxState = lwmDEMUX_PacketStart;
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketData:
		{
			if(movieState->packetDataFillable.FeedData(&bytes, &bytesAvailable))
			{
				DigestPacket(movieState, outResult);
				*outBytesDigested = numBytes - bytesAvailable;

				movieState->demuxState = lwmDEMUX_PacketHeader;
				movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

				return;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
	case lwmDEMUX_FatalError:
		*outResult = lwmDIGEST_Error;
		*outBytesDigested = 0;
		break;
	};
}


LWMOVIE_API_LINK int lwmMovieState_GetStreamParameterU32(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt32 streamParameterU32, lwmUInt32 *outValue)
{
	switch(streamType)
	{
	case lwmSTREAMTYPE_Video:
		{
			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_Width:
				*outValue = movieState->videoInfo.videoWidth;
				return 1;
			case lwmSTREAMPARAM_U32_Height:
				*outValue = movieState->videoInfo.videoHeight;
				return 1;
			case lwmSTREAMPARAM_U32_PPSNumerator:
				*outValue = movieState->videoInfo.periodsPerSecondNum;
				return 1;
			case lwmSTREAMPARAM_U32_PPSDenominator:
				*outValue = movieState->videoInfo.periodsPerSecondDenom;
				return 1;
			case lwmSTREAMPARAM_U32_ReconType:
				{
					switch(movieState->movieInfo.videoStreamType)
					{
					case lwmVST_M1V_Variant:
						*outValue = lwmRC_MPEG1Video;
						return 1;
					default:
						return 0;
					};
				}
				return 1;
			case lwmSTREAMPARAM_U32_SyncPeriod:
				*outValue = movieState->streamSyncPeriods[lwmSTREAMTYPE_Video];
				return 1;
			case lwmSTREAMPARAM_U32_LongestFrameReadAhead:
				*outValue = movieState->movieInfo.longestFrameReadahead;
				return 1;
			};
		}
	case lwmSTREAMTYPE_Audio:
		{
			if(movieState->movieInfo.audioStreamType == lwmAST_None)
				return 0;

			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_SampleRate:
				*outValue = movieState->audioInfo.sampleRate;
				return 1;
			case lwmSTREAMPARAM_U32_SpeakerLayout:
				*outValue = movieState->audioInfo.speakerLayout;
				return 1;
			default:
				return 0;
			};
		}
	};
	return 0;
}


LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmSVideoFrameProvider *frameProvider)
{
	switch(reconstructorType)
	{
	case lwmRC_MPEG1Video:
		{
			lwmovie::lwmCM1VSoftwareReconstructor *recon = static_cast<lwmovie::lwmCM1VSoftwareReconstructor*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmCM1VSoftwareReconstructor)));
			if(!recon)
				return NULL;
			new (recon) lwmovie::lwmCM1VSoftwareReconstructor();
			if(!recon->Initialize(alloc, frameProvider, movieState))
			{
				lwmIVideoReconstructor_Destroy(recon);
				recon = NULL;
			}
			return recon;
		}
		break;
	};
	return NULL;
}

LWMOVIE_API_LINK void lwmIVideoReconstructor_Destroy(lwmIVideoReconstructor *videoRecon)
{
	videoRecon->Destroy();
}

LWMOVIE_API_LINK void lwmMovieState_SetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon)
{
	movieState->videoReconstructor = recon;
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->SetReconstructor(recon);
}

LWMOVIE_API_LINK void lwmMovieState_SetVideoDigestWorkNotifier(lwmMovieState *movieState, lwmSWorkNotifier *videoDigestWorkNotifier)
{
	movieState->videoDigestWorkNotifier = videoDigestWorkNotifier;
}

LWMOVIE_API_LINK void lwmMovieState_Destroy(lwmMovieState *movieState)
{
	lwmSAllocator *alloc = movieState->alloc;
	if(movieState->packetDataBytesEscaped)
		alloc->freeFunc(alloc, movieState->packetDataBytesEscaped);
	if(movieState->packetDataBytes)
		alloc->freeFunc(alloc, movieState->packetDataBytes);
	if(movieState->m1vDecoder)
	{
		movieState->m1vDecoder->~lwmVidStream();
		alloc->freeFunc(alloc, movieState->m1vDecoder);
	}
	if(movieState->mp2Decoder)
	{
		movieState->mp2Decoder->~lwmCMP2Decoder();
		alloc->freeFunc(alloc, movieState->mp2Decoder);
	}
	alloc->freeFunc(alloc, movieState);
}


LWMOVIE_API_LINK void lwmFlushProfileTags(lwmMovieState *movieState, lwmCProfileTagSet *tagSet)
{
	if(movieState->videoReconstructor)
		movieState->videoReconstructor->FlushProfileTags(tagSet);
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->FlushProfileTags(tagSet);
}

LWMOVIE_API_LINK void lwmVideoRecon_SetWorkNotifier(lwmIVideoReconstructor *recon, lwmSWorkNotifier *workNotifier)
{
	recon->SetWorkNotifier(workNotifier);
}

LWMOVIE_API_LINK void lwmMovieState_VideoDigestParticipate(lwmMovieState *movieState)
{
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->Participate();
}

LWMOVIE_API_LINK int lwmMovieState_IsAudioPlaybackSynchronized(lwmMovieState *movieState)
{
	return movieState->isAudioSynchronized ? 1 : 0;
}

LWMOVIE_API_LINK int lwmMovieState_SynchronizeAudioPlayback(lwmMovieState *movieState)
{
	// TODO: Check streams
	if(movieState->isAudioSynchronized)
		return 1;	// Already synchronized
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] == 0 || movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] == 0)
		return 0;	// No streams
	// audioTime = videoPeriod * sampleRate / (ppsNum/ppsDenom)
	lwmUInt64 timeCalc = static_cast<lwmUInt64>(movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] - 1) * movieState->videoInfo.periodsPerSecondDenom;
	if(timeCalc > static_cast<lwmUInt32>(0xffffffff))
		return 0;	// Too big
	timeCalc *= movieState->audioInfo.sampleRate;
	timeCalc /= movieState->videoInfo.periodsPerSecondNum;

	if(timeCalc > static_cast<lwmUInt32>(0xffffffff))
		return 0;	// Too big

	lwmUInt32 audioTime = static_cast<lwmUInt32>(timeCalc);

	lwmCAudioBuffer *audioBuffer = movieState->mp2Decoder->GetAudioBuffer();
	// If decoded samples are below the 0 point, trim them.  This is normal due to algorithmic start delay.
	if(audioBuffer->GetNumCommittedSamples() > movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio])
		audioBuffer->SkipSamples(audioBuffer->GetNumCommittedSamples() - movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio]);
	lwmUInt32 audioBufferStartPoint = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] - audioBuffer->GetNumCommittedSamples();

	if(audioBufferStartPoint > audioTime)
		return 0;	// Audio starts in the future
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] <= audioTime)
		return 0;	// Audio ends in the past

	// Audio starts immediately or in the past, trim any preceding samples and complete
	audioBuffer->SkipSamples(audioTime - audioBufferStartPoint);
	movieState->isAudioSynchronized = true;
	return 1;
}

LWMOVIE_API_LINK lwmUInt32 lwmMovieState_ReadAudioSamples(lwmMovieState *movieState, void *samples, lwmUInt32 numSamples)
{
	// TODO: Check stream type
	if(!movieState->isAudioSynchronized)
		return 0;
	return movieState->mp2Decoder->GetAudioBuffer()->ReadCommittedSamples(samples, numSamples);
}

LWMOVIE_API_LINK void lwmMovieState_NotifyAudioPlaybackUnderrun(lwmMovieState *movieState)
{
	// TODO: Check stream type
	return movieState->DesyncAudio();
}

LWMOVIE_API_LINK void lwmVideoRecon_Participate(lwmIVideoReconstructor *recon)
{
	recon->Participate();
}

LWMOVIE_API_LINK lwmUInt32 lwmVideoRecon_GetWorkFrameIndex(const lwmIVideoReconstructor *recon)
{
	return recon->GetWorkFrameIndex();
}
