#include <stdlib.h>
#include <string.h>
#include <new>
#include "lwmovie_atomicint.hpp"
#include "lwmovie_recon_m1vsw.hpp"
#include "lwmovie_types.hpp"
#include "lwmovie_simd_defs.hpp"
#include "lwmovie_profile.hpp"
#include "lwmovie_demux.hpp"

void j_rev_dct_sse2( lwmSInt16 data[64] );
void j_rev_dct_sse2_sparseDC( lwmSInt16 data[64], lwmSInt16 value );
void j_rev_dct_sse2_sparseAC( lwmSInt16 data[64], lwmFastUInt8 coeffPos, lwmSInt16 value );

lwmovie::lwmCM1VSoftwareReconstructor::lwmCM1VSoftwareReconstructor()
:	m_mblocks(NULL)
,	m_blocks(NULL)
,	m_dctBlocks(NULL)
,	m_yuvBuffer(NULL)
{
}

lwmovie::lwmCM1VSoftwareReconstructor::~lwmCM1VSoftwareReconstructor()
{
	if(m_mblocks)
		m_alloc.freeFunc(m_alloc.opaque, m_mblocks);
	if(m_blocks)
		m_alloc.freeFunc(m_alloc.opaque, m_blocks);
	if(m_dctBlocks)
		m_alloc.freeFunc(m_alloc.opaque, m_dctBlocks);
	if(m_yuvBuffer)
		m_alloc.freeFunc(m_alloc.opaque, m_yuvBuffer);
	if(m_rowCommitCounts)
		m_alloc.freeFunc(m_alloc.opaque, m_rowCommitCounts);
	if(m_workRowUsers)
		m_alloc.freeFunc(m_alloc.opaque, m_workRowUsers);
	if(m_workRowProfileTags)
		m_alloc.freeFunc(m_alloc.opaque, m_workRowProfileTags);
}

bool lwmovie::lwmCM1VSoftwareReconstructor::Initialize(const lwmSAllocator *alloc, lwmMovieState *movieState)
{
	lwmUInt32 width, height;

	lwmGetVideoParameters(movieState, &width, &height, NULL, NULL, NULL);

	lwmUInt32 mbWidth = (width + 15) / 16;
	lwmUInt32 mbHeight = (height + 15) / 16;
	lwmUInt32 numMB = mbWidth * mbHeight;
	m_alloc = *alloc;
	
	m_yStride = PadToSIMD(mbWidth * 16);
	m_uvStride = PadToSIMD(mbWidth * 8);

	m_yOffset = 0;
	m_uOffset = mbHeight * 16 * m_yStride;
	m_vOffset = m_uOffset + (mbHeight * 8 * m_uvStride);
	m_yuvFrameSize = (m_vOffset + (mbHeight * 8 * m_uvStride));

	m_mblocks = static_cast<lwmReconMBlock*>(m_alloc.allocFunc(m_alloc.opaque, sizeof(lwmReconMBlock) * numMB));
	m_blocks = static_cast<lwmBlockInfo*>(m_alloc.allocFunc(m_alloc.opaque, sizeof(lwmBlockInfo) * numMB * 6));
	m_dctBlocks = static_cast<lwmDCTBLOCK*>(m_alloc.allocFunc(m_alloc.opaque, sizeof(lwmDCTBLOCK) * numMB * 6));
	m_yuvBuffer = static_cast<lwmUInt8*>(m_alloc.allocFunc(m_alloc.opaque, m_yuvFrameSize * 3));
	m_rowCommitCounts = static_cast<lwmAtomicInt*>(m_alloc.allocFunc(m_alloc.opaque, mbHeight * sizeof(lwmAtomicInt)));
	m_workRowUsers = static_cast<lwmAtomicInt*>(m_alloc.allocFunc(m_alloc.opaque, mbHeight * sizeof(lwmAtomicInt)));

	m_workRowProfileTags = static_cast<lwmCProfileTagSet*>(m_alloc.allocFunc(m_alloc.opaque, mbHeight * sizeof(lwmCProfileTagSet)));
	for(lwmUInt32 i=0;i<mbHeight;i++)
		new (m_workRowProfileTags + i) lwmCProfileTagSet();


	m_mbWidth = mbWidth;
	m_mbHeight = mbHeight;

	m_movieState = movieState;

	if(!m_mblocks || !m_blocks || !m_dctBlocks || !m_yuvBuffer || !m_rowCommitCounts)
		return false;

	m_stWorkNotifier.opaque = this;
	m_stWorkNotifier.join = STWNJoinFunc;
	m_stWorkNotifier.notifyAvailable = STWNNotifyAvailableFunc;

	m_workNotifier = &m_stWorkNotifier;

	return true;
}

lwmovie::lwmDCTBLOCK *lwmovie::lwmCM1VSoftwareReconstructor::StartReconBlock(lwmSInt32 address)
{
	return m_dctBlocks + address;
}

void lwmovie::lwmCM1VSoftwareReconstructor::SetMBlockInfo(lwmSInt32 mbAddress, bool skipped, bool mb_motion_forw, bool mb_motion_back,
			lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, bool full_pel_forw,
			lwmSInt32 recon_right_back, lwmSInt32 recon_down_back, bool full_pel_back)
{
	if(full_pel_forw)
	{
		recon_right_for <<= 1;
		recon_down_for <<= 1;
	}
	if(full_pel_back)
	{
		recon_right_back <<= 1;
		recon_down_back <<= 1;
	}

	m_mblocks[mbAddress].SetReconInfo(skipped, mb_motion_forw, mb_motion_back, recon_right_for, recon_down_for, recon_right_back, recon_down_back);
}

void lwmovie::lwmCM1VSoftwareReconstructor::SetBlockInfo(lwmSInt32 sbAddress, bool zero_block_flag)
{
	m_blocks[sbAddress].SetReconInfo(zero_block_flag);
}

void lwmovie::lwmCM1VSoftwareReconstructor::CommitZero(lwmSInt32 address)
{
	m_blocks[address].needs_idct = false;
}

void lwmovie::lwmCM1VSoftwareReconstructor::CommitSparse(lwmSInt32 address, lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff)
{
	lwmBlockInfo *blockInfo = m_blocks + address;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = true;
	blockInfo->sparse_idct_coef = lastCoeff;
	blockInfo->sparse_idct_index = lastCoeffPos;
}

void lwmovie::lwmCM1VSoftwareReconstructor::CommitFull(lwmSInt32 address)
{
	lwmBlockInfo *blockInfo = m_blocks + address;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = false;
}

void lwmovie::lwmCM1VSoftwareReconstructor::WaitForFinish()
{
	m_workNotifier->join(m_workNotifier->opaque);
}

void lwmovie::lwmCM1VSoftwareReconstructor::Participate()
{
	lwmUInt32 numMBlocks = m_mbWidth * m_mbHeight;

	const lwmReconMBlock *mblock = m_mblocks;
	const lwmBlockInfo *blocks = m_blocks;
	lwmUInt32 mbWidth = m_mbWidth;
	lwmUInt32 mbHeight = m_mbHeight;

	lwmUInt8 *cy = m_currentTarget.base + m_yOffset;
	lwmUInt8 *cu = m_currentTarget.base + m_uOffset;
	lwmUInt8 *cv = m_currentTarget.base + m_vOffset;
	lwmUInt8 *py = m_pastTarget.base + m_yOffset;
	lwmUInt8 *pu = m_pastTarget.base + m_uOffset;
	lwmUInt8 *pv = m_pastTarget.base + m_vOffset;
	lwmUInt8 *fy = m_futureTarget.base + m_yOffset;
	lwmUInt8 *fu = m_futureTarget.base + m_uOffset;
	lwmUInt8 *fv = m_futureTarget.base + m_vOffset;

	lwmUInt32 rowStrideY = m_yStride;
	lwmUInt32 rowStrideUV = m_uvStride;
	lwmUInt32 mbRowStrideY = 16 * m_yStride;
	lwmUInt32 mbRowStrideUV = 8 * m_uvStride;

	lwmUInt32 row = 0;

	// Find a row with 0 commit count
	for(lwmUInt32 sr=0;sr<mbHeight;sr++)
	{
		if(m_rowCommitCounts[sr] != 0)
		{
			// If this is the first thread to work on this row, we're done
			if(lwmAtomicIncrement(m_workRowUsers + sr) == 1)
			{
				row = sr;
				break;
			}
		}
	}

	lwmUInt32 mbAddress = row*mbWidth;

	ReconstructRow(m_mblocks + mbAddress, m_blocks + mbAddress*6, m_dctBlocks + mbAddress*6,
		cy + row*mbRowStrideY, cu + row*mbRowStrideUV, cv + row*mbRowStrideUV,
		fy + row*mbRowStrideY, fu + row*mbRowStrideUV, fv + row*mbRowStrideUV,
		py + row*mbRowStrideY, pu + row*mbRowStrideUV, pv + row*mbRowStrideUV,
		m_workRowProfileTags + row
		);
}

void lwmovie::lwmCM1VSoftwareReconstructor::SetWorkNotifier(const lwmSWorkNotifier *workNotifier)
{
	m_workNotifier = workNotifier;
}

void lwmovie::lwmCM1VSoftwareReconstructor::ReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
	lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, bool halfRes, lwmLargeUInt stride, lwmCProfileTagSet *profileTags)
{
	lwmSInt32 mbRightFor = mblock->recon_right_for;
	lwmSInt32 mbDownFor = mblock->recon_down_for;
	lwmSInt32 mbRightBack = mblock->recon_right_back;
	lwmSInt32 mbDownBack = mblock->recon_down_back;

	if(halfRes)
	{
		mbRightFor >>= 1;
		mbDownFor >>= 1;
		mbRightBack >>= 1;
		mbDownBack >>= 1;
	}

	// Halfpel approximation
	mbRightFor /= 2;
	mbDownFor /= 2;
	mbRightBack /= 2;
	mbDownBack /= 2;

	f += mbRightFor + mbDownFor * static_cast<lwmSInt32>(stride);
	p += mbRightBack + mbDownBack * static_cast<lwmSInt32>(stride);

	const lwmSInt16 *dctCoeff = dctBlock->data;
	for(int row=0;row<8;row++)
	{
		lwmUInt8 *cpx = c;
		const lwmUInt8 *fpx = f;
		const lwmUInt8 *ppx = p;
		
		for(int col=0;col<8;col++)
		{
			lwmSInt16 coeff = 0;

			if(mblock->mb_motion_forw)
				coeff = *fpx++;
			if(mblock->mb_motion_back)
				coeff += *ppx++;
			if(mblock->mb_motion_forw && mblock->mb_motion_back)
				coeff /= 2;

			if(!mblock->skipped)
				coeff += (*dctCoeff++);

			if(coeff < 0) coeff = 0;
			else if(coeff > 255) coeff = 255;
			(*cpx++) = static_cast<lwmUInt8>(coeff);
		}

		c += stride;
		f += stride;
		p += stride;
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::ReconstructRow(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlocks,
	lwmUInt8 *cy, lwmUInt8 *cu, lwmUInt8 *cv,
	lwmUInt8 *fy, lwmUInt8 *fu, lwmUInt8 *fv,
	lwmUInt8 *py, lwmUInt8 *pu, lwmUInt8 *pv,
	lwmCProfileTagSet *profileTags
	)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ReconRow);
#endif

	lwmUInt32 mbWidth = m_mbWidth;

	lwmUInt32 yStride = m_yStride;
	lwmUInt32 uvStride = m_uvStride;

	lwmLargeUInt blockOffsets[4];
	blockOffsets[0] = 0;
	blockOffsets[1] = 8;
	blockOffsets[2] = yStride * 8;
	blockOffsets[3] = 8 + yStride * 8;

	for(lwmUInt32 mbCol=0;mbCol<mbWidth;mbCol++)
	{
		bool allZero = mblock->skipped ||
			(block[0].zero_block_flag && block[1].zero_block_flag && block[2].zero_block_flag &&
			 block[3].zero_block_flag && block[4].zero_block_flag && block[5].zero_block_flag);

		ReconstructLumaBlocks(mblock, block, dctBlocks, cy, fy, py, yStride, profileTags);
		ReconstructChromaBlock(mblock, block + 4, dctBlocks + 4, cu, fu, pu, uvStride, profileTags);
		ReconstructChromaBlock(mblock, block + 5, dctBlocks + 5, cv, fv, pv, uvStride, profileTags);

		cy += 16;
		cu += 8;
		cv += 8;
		fy += 16;
		fu += 8;
		fv += 8;
		py += 16;
		pu += 8;
		pv += 8;
		dctBlocks += 6;
		mblock++;
		block += 6;
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::PutDCTBlock(const lwmDCTBLOCK *dctBlock, lwmUInt8 *channel, lwmUInt32 stride)
{
	const lwmSInt16 *s16row = dctBlock->data;
	for(lwmUInt32 row=0;row<8;row++)
	{
		for(lwmUInt32 col=0;col<8;col++)
		{
			lwmSInt16 value = s16row[col];
			if(value < 0)
				value = 0;
			else if(value > 255)
				value = 255;
			channel[col] = static_cast<lwmUInt8>(value);
		}
		s16row += 8;
		channel += stride;
	}
}


void lwmovie::lwmCM1VSoftwareReconstructor::GetChannel(lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride)
{
	switch(channelNum)
	{
	case 0:
		*outPChannel = m_currentTarget.base + m_yOffset;
		*outStride = m_yStride;
		break;
	case 1:
		*outPChannel = m_currentTarget.base + m_uOffset;
		*outStride = m_uvStride;
		break;
	case 2:
		*outPChannel = m_currentTarget.base + m_vOffset;
		*outStride = m_uvStride;
		break;
	default:
		*outPChannel = NULL;
		*outStride = 0;
		break;
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past)
{
	for(lwmUInt32 i=0;i<m_mbHeight;i++)
		m_rowCommitCounts[i] = m_workRowUsers[i] = 0;

	m_currentTarget.base = m_yuvBuffer + current * m_yuvFrameSize;
	m_futureTarget.base = m_yuvBuffer + future * m_yuvFrameSize;
	m_pastTarget.base = m_yuvBuffer + past * m_yuvFrameSize;
}

void lwmovie::lwmCM1VSoftwareReconstructor::MarkRowFinished(lwmSInt32 firstMBAddress)
{
	lwmUInt32 row = static_cast<lwmUInt32>(firstMBAddress) / m_mbWidth;

	// Check commit count, should be zero.  If non-zero, the video is bad and is committing the same row multiple times
	if(lwmAtomicIncrement(m_rowCommitCounts + row) == 1)
		m_workNotifier->notifyAvailable(m_workNotifier->opaque);
}

void lwmovie::lwmCM1VSoftwareReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
	for(lwmUInt32 i=0;i<m_mbHeight;i++)
		m_workRowProfileTags[i].FlushTo(tagSet);
}
