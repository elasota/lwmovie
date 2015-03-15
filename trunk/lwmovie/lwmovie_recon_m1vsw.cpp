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
#include <stdlib.h>
#include <string.h>
#include <new>
#include "../common/lwmovie_atomicint_funcs.hpp"
#include "../common/lwmovie_coretypes.h"
#include "lwmovie.h"
#include "lwmovie_recon_m1vsw.hpp"
#include "lwmovie_simd_defs.hpp"
#include "lwmovie_profile.hpp"

void j_rev_dct_sse2( lwmSInt16 data[64] );
void j_rev_dct_sse2_sparseDC( lwmSInt16 data[64], lwmSInt16 value );
void j_rev_dct_sse2_sparseAC( lwmSInt16 data[64], lwmFastUInt8 coeffPos, lwmSInt16 value );

lwmovie::lwmCM1VSoftwareReconstructor::lwmCM1VSoftwareReconstructor()
:	m_mblocks(NULL)
,	m_blocks(NULL)
,	m_dctBlocks(NULL)
,	m_workRowUsers(NULL)
,	m_rowCommitCounts(NULL)
,	m_useRowJobs(false)
{
}

lwmovie::lwmCM1VSoftwareReconstructor::~lwmCM1VSoftwareReconstructor()
{
	this->WaitForFinish();
	if(m_mblocks)
		m_alloc->Free(m_mblocks);
	if(m_blocks)
		m_alloc->Free(m_blocks);
	if(m_dctBlocks)
		m_alloc->Free(m_dctBlocks);
	if(m_rowCommitCounts)
		m_alloc->Free(m_rowCommitCounts);
	if(m_workRowUsers)
		m_alloc->Free(m_workRowUsers);
#ifdef LWMOVIE_PROFILE
	if(m_workRowProfileTags)
		m_alloc->Free(m_workRowProfileTags);
#endif
}

bool lwmovie::lwmCM1VSoftwareReconstructor::Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState, bool useRowJobs)
{
	lwmUInt32 width, height;

	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &width);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &height);

	lwmUInt32 mbWidth = (width + 15) / 16;
	lwmUInt32 mbHeight = (height + 15) / 16;
	lwmUInt32 numMB = mbWidth * mbHeight;
	m_alloc = alloc;
	m_frameProvider = frameProvider;
	m_useRowJobs = useRowJobs;

	if(!frameProvider->createWorkFramesFunc(frameProvider, 2, 1, mbWidth * 16, mbHeight * 16, lwmFRAMEFORMAT_YUV420P_Planar))
		return false;
	m_yStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 0);
	m_uStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 1);
	m_vStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 2);

	if(m_useRowJobs)
	{
		m_mblocks = m_alloc->NAlloc<lwmReconMBlock>(numMB);
		m_blocks = m_alloc->NAlloc<lwmBlockInfo>(numMB * 6);
		m_dctBlocks = m_alloc->NAlloc<lwmDCTBLOCK>(numMB * 6);
		m_rowCommitCounts = m_alloc->NAlloc<lwmAtomicInt>(mbHeight);
		m_workRowUsers = m_alloc->NAlloc<lwmAtomicInt>(mbHeight);
	}

#ifdef LWMOVIE_PROFILE
	m_workRowProfileTags = static_cast<lwmCProfileTagSet*>(m_alloc->allocFunc(m_alloc, mbHeight * sizeof(lwmCProfileTagSet)));
	for(lwmUInt32 i=0;i<mbHeight;i++)
		new (m_workRowProfileTags + i) lwmCProfileTagSet();
#endif

	m_mbWidth = mbWidth;
	m_mbHeight = mbHeight;

	m_movieState = movieState;

	if(m_useRowJobs)
	{
		if(!m_mblocks || !m_blocks || !m_dctBlocks || !m_rowCommitCounts || !m_workRowUsers)
			return false;
	}

	m_stWorkNotifier.joinFunc = STWNJoinFunc;
	m_stWorkNotifier.notifyAvailableFunc = STWNNotifyAvailableFunc;
	m_stWorkNotifier.recon = this;

	m_workNotifier = &m_stWorkNotifier;

	m_currentTarget.isOpen = m_pastTarget.isOpen = m_futureTarget.isOpen = false;

	return true;
}

void lwmovie::lwmCM1VSoftwareReconstructor::WaitForFinish()
{
	m_workNotifier->joinFunc(m_workNotifier);
}

void lwmovie::lwmCM1VSoftwareReconstructor::Participate()
{
	lwmUInt32 numMBlocks = m_mbWidth * m_mbHeight;

	const lwmReconMBlock *mblock = m_mblocks;
	const lwmBlockInfo *blocks = m_blocks;
	lwmUInt32 mbWidth = m_mbWidth;
	lwmUInt32 mbHeight = m_mbHeight;

	lwmUInt8 *cy = m_currentTarget.yPlane;
	lwmUInt8 *cu = m_currentTarget.uPlane;
	lwmUInt8 *cv = m_currentTarget.vPlane;
	lwmUInt8 *py = m_pastTarget.yPlane;
	lwmUInt8 *pu = m_pastTarget.uPlane;
	lwmUInt8 *pv = m_pastTarget.vPlane;
	lwmUInt8 *fy = m_futureTarget.yPlane;
	lwmUInt8 *fu = m_futureTarget.uPlane;
	lwmUInt8 *fv = m_futureTarget.vPlane;

	lwmUInt32 rowStrideY = m_yStride;
	lwmUInt32 rowStrideU = m_uStride;
	lwmUInt32 rowStrideV = m_vStride;
	lwmUInt32 mbRowStrideY = 16 * m_yStride;
	lwmUInt32 mbRowStrideU = 8 * m_uStride;
	lwmUInt32 mbRowStrideV = 8 * m_vStride;

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

	ReconstructRow(row, m_mblocks + mbAddress, m_blocks + mbAddress*6, m_dctBlocks + mbAddress*6,
		cy + row*mbRowStrideY, cu + row*mbRowStrideU, cv + row*mbRowStrideV,
		fy + row*mbRowStrideY, fu + row*mbRowStrideU, fv + row*mbRowStrideV,
		py + row*mbRowStrideY, pu + row*mbRowStrideU, pv + row*mbRowStrideV,
#ifdef LWMOVIE_PROFILE
		m_workRowProfileTags + row
#else
		NULL
#endif
		);
}

void lwmovie::lwmCM1VSoftwareReconstructor::SetWorkNotifier(lwmSWorkNotifier *workNotifier)
{
	m_workNotifier = workNotifier;
}

void lwmovie::lwmCM1VSoftwareReconstructor::STReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *blocks, lwmDCTBLOCK *dctBlocks, lwmSInt32 mbAddress)
{
	lwmUInt32 mbWidth = m_mbWidth;

	lwmUInt32 yStride = m_yStride;
	lwmUInt32 uStride = m_uStride;
	lwmUInt32 vStride = m_vStride;

	lwmLargeUInt blockOffsets[4];
	blockOffsets[0] = 0;
	blockOffsets[1] = 8;
	blockOffsets[2] = yStride * 8;
	blockOffsets[3] = 8 + yStride * 8;

	lwmSInt32 row = mbAddress / m_mbWidth;
	lwmSInt32 col = mbAddress % m_mbWidth;

	lwmSInt32 mvMinDown = -row * 32;
	lwmSInt32 mvMaxDown = (m_mbHeight - row - 1) * 32;
	lwmSInt32 mvMinRight = -col * 32;
	lwmSInt32 mvMaxRight = (m_mbWidth - col - 1) * 32;

	// Check MVs
	if(mblock->mb_motion_forw)
	{
		if(mblock->recon_down_for < mvMinDown || mblock->recon_down_for > mvMaxDown || mblock->recon_right_for < mvMinRight || mblock->recon_right_for > mvMaxRight)
			return;
	}
	if(mblock->mb_motion_back)
	{
		if(mblock->recon_down_back < mvMinDown || mblock->recon_down_back > mvMaxDown || mblock->recon_right_back < mvMinRight || mblock->recon_right_back > mvMaxRight)
			return;
	}

	lwmUInt32 yOffset = static_cast<lwmUInt32>(row * 16 * m_yStride + col * 16);
	lwmUInt32 uOffset = static_cast<lwmUInt32>(row * 8 * m_uStride + col * 8);
	lwmUInt32 vOffset = static_cast<lwmUInt32>(row * 8 * m_vStride + col * 8);

	lwmUInt8 *cy = m_currentTarget.yPlane;
	lwmUInt8 *cu = m_currentTarget.uPlane;
	lwmUInt8 *cv = m_currentTarget.vPlane;
	lwmUInt8 *py = m_pastTarget.yPlane;
	lwmUInt8 *pu = m_pastTarget.uPlane;
	lwmUInt8 *pv = m_pastTarget.vPlane;
	lwmUInt8 *fy = m_futureTarget.yPlane;
	lwmUInt8 *fu = m_futureTarget.uPlane;
	lwmUInt8 *fv = m_futureTarget.vPlane;

	ReconstructLumaBlocks(mblock, blocks, dctBlocks, cy + yOffset, fy + yOffset, py + yOffset, yStride, NULL);
	ReconstructChromaBlock(mblock, blocks + 4, dctBlocks + 4, cu + uOffset, fu + uOffset, pu + uOffset, uStride, NULL);
	ReconstructChromaBlock(mblock, blocks + 5, dctBlocks + 5, cv + vOffset, fv + vOffset, pv + vOffset, vStride, NULL);
}

void lwmovie::lwmCM1VSoftwareReconstructor::ReconstructRow(lwmUInt32 row, const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlocks,
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
	lwmUInt32 uStride = m_uStride;
	lwmUInt32 vStride = m_vStride;

	lwmLargeUInt blockOffsets[4];
	blockOffsets[0] = 0;
	blockOffsets[1] = 8;
	blockOffsets[2] = yStride * 8;
	blockOffsets[3] = 8 + yStride * 8;

	lwmSInt32 mvMinDown = -static_cast<lwmSInt32>(row) * 32;
	lwmSInt32 mvMaxDown = (m_mbHeight - row - 1) * 32;
	lwmSInt32 mvMinRight = 0;
	lwmSInt32 mvMaxRight = (m_mbWidth - 1) * 32;

	for(lwmUInt32 mbCol=0;mbCol<mbWidth;mbCol++)
	{
		// Check MVs
		if(mblock->mb_motion_forw)
		{
			if(mblock->recon_down_for < mvMinDown || mblock->recon_down_for > mvMaxDown || mblock->recon_right_for < mvMinRight || mblock->recon_right_for > mvMaxRight)
				continue;
		}
		if(mblock->mb_motion_back)
		{
			if(mblock->recon_down_back < mvMinDown || mblock->recon_down_back > mvMaxDown || mblock->recon_right_back < mvMinRight || mblock->recon_right_back > mvMaxRight)
				continue;
		}

		ReconstructLumaBlocks(mblock, block, dctBlocks, cy, fy, py, yStride, profileTags);
		ReconstructChromaBlock(mblock, block + 4, dctBlocks + 4, cu, fu, pu, uStride, profileTags);
		ReconstructChromaBlock(mblock, block + 5, dctBlocks + 5, cv, fv, pv, vStride, profileTags);

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
		mvMinRight -= 32;
		mvMaxRight -= 32;
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past, bool currentIsB)
{
	this->CloseFrame();

	if(m_useRowJobs)
		for(lwmUInt32 i=0;i<m_mbHeight;i++)
			m_rowCommitCounts[i] = m_workRowUsers[i] = 0;

	m_frameProvider->lockWorkFrameFunc(m_frameProvider, current - 1, currentIsB ? lwmVIDEOLOCK_Write_Only : lwmVIDEOLOCK_Write_ReadLater);
	m_currentTarget.LoadFromFrameProvider(m_frameProvider, current - 1);
	if(future)
	{
		m_frameProvider->lockWorkFrameFunc(m_frameProvider, future - 1, lwmVIDEOLOCK_Read);
		m_futureTarget.LoadFromFrameProvider(m_frameProvider, future - 1);
	}
	if(past)
	{
		m_frameProvider->lockWorkFrameFunc(m_frameProvider, past - 1, lwmVIDEOLOCK_Read);
		m_pastTarget.LoadFromFrameProvider(m_frameProvider, past - 1);
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::CloseFrame()
{
	m_currentTarget.Close(m_frameProvider);
	m_futureTarget.Close(m_frameProvider);
	m_pastTarget.Close(m_frameProvider);
}

void lwmovie::lwmCM1VSoftwareReconstructor::PresentFrame(lwmUInt32 slot)
{
	CloseFrame();
	this->m_outputTarget = slot - 1;
}

lwmovie::lwmIM1VBlockCursor *lwmovie::lwmCM1VSoftwareReconstructor::CreateBlockCursor()
{
	if(!m_useRowJobs)
	{
		lwmCM1VSelfContainedBlockCursor *blockCursor = m_alloc->NAlloc<lwmCM1VSelfContainedBlockCursor>(1);
		if(blockCursor)
		{
			new (blockCursor) lwmCM1VSelfContainedBlockCursor(this);
			return blockCursor;
		}
		else
			return NULL;
	}

	lwmCM1VGridBlockCursor *blockCursor = m_alloc->NAlloc<lwmCM1VGridBlockCursor>(1);
	if(blockCursor)
	{
		new (blockCursor) lwmCM1VGridBlockCursor(m_mblocks, m_blocks, m_dctBlocks);
		return blockCursor;
	}
	else
		return NULL;
}

void lwmovie::lwmCM1VSoftwareReconstructor::MarkRowFinished(lwmSInt32 firstMBAddress)
{
	if(m_useRowJobs)
	{
		lwmUInt32 row = static_cast<lwmUInt32>(firstMBAddress) / m_mbWidth;

		// Check commit count, should be zero.  If non-zero, the video is bad and is committing the same row multiple times
		if(lwmAtomicIncrement(m_rowCommitCounts + row) == 1)
			m_workNotifier->notifyAvailableFunc(m_workNotifier);
	}
}

void lwmovie::lwmCM1VSoftwareReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
#ifdef LWMOVIE_PROFILE
	for(lwmUInt32 i=0;i<m_mbHeight;i++)
		m_workRowProfileTags[i].FlushTo(tagSet);
#endif
}


void lwmovie::lwmCM1VSoftwareReconstructor::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	lwmCM1VSoftwareReconstructor *self = this;
	self->~lwmCM1VSoftwareReconstructor();
	alloc->Free(self);
}

lwmUInt32 lwmovie::lwmCM1VSoftwareReconstructor::GetWorkFrameIndex() const
{
	return m_outputTarget;
}

//////////////////////////////////////////////////////////////////
void lwmovie::lwmCM1VSoftwareReconstructor::STarget::LoadFromFrameProvider(lwmSVideoFrameProvider *frameProvider, lwmUInt32 frameIndex)
{
	this->yPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 0));
	this->uPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 1));
	this->vPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 2));
	this->frameIndex = frameIndex;
	this->isOpen = true;
}

void lwmovie::lwmCM1VSoftwareReconstructor::STarget::Close(lwmSVideoFrameProvider *frameProvider)
{
	if(this->isOpen)
	{
		this->isOpen = false;
		frameProvider->unlockWorkFrameFunc(frameProvider, this->frameIndex);
	}
}

//////////////////////////////////////////////////////////////////
// Grid block cursor
lwmovie::lwmCM1VGridBlockCursor::lwmCM1VGridBlockCursor(lwmReconMBlock *mblocks, lwmBlockInfo *blocks, lwmDCTBLOCK *dctBlocks)
	: m_mblocks(mblocks)
	, m_blocks(blocks)
	, m_dctBlocks(dctBlocks)
{
}

void lwmovie::lwmCM1VGridBlockCursor::OpenMB(lwmSInt32 mbAddress)
{
	m_currentMBlock = m_mblocks + mbAddress;
	m_currentBlockBase = m_blocks + mbAddress*6;
	m_currentDCTBlockBase = m_dctBlocks + mbAddress*6;
}

//////////////////////////////////////////////////////////////////
// Self-contained block cursor
lwmovie::lwmCM1VSelfContainedBlockCursor::lwmCM1VSelfContainedBlockCursor(lwmCM1VSoftwareReconstructor *recon)
	: m_targetMBAddress(0)
	, m_recon(recon)
{
}

void lwmovie::lwmCM1VSelfContainedBlockCursor::OpenMB(lwmSInt32 mbAddress)
{
	m_targetMBAddress = mbAddress;
	m_currentDCTBlockBase = m_dctBlocks;
	m_currentBlockBase = m_blocks;
	m_currentMBlock = &m_mblock;
}

void lwmovie::lwmCM1VSelfContainedBlockCursor::CloseMB()
{
	m_recon->STReconstructBlock(&m_mblock, m_blocks, m_dctBlocks, m_targetMBAddress);
}


//////////////////////////////////////////////////////////////////
// Base block cursor
lwmovie::lwmCM1VBaseBlockCursor::lwmCM1VBaseBlockCursor()
	: m_currentMBlock(NULL)
	, m_currentBlockBase(NULL)
	, m_currentDCTBlockBase(NULL)
	, m_openedReconBlock(NULL)
{
}

void lwmovie::lwmCM1VBaseBlockCursor::CloseMB()
{
}

void lwmovie::lwmCM1VBaseBlockCursor::SetMBlockInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back,
			lwmSInt32 recon_right_for, lwmSInt32 recon_down_for,
			lwmSInt32 recon_right_back, lwmSInt32 recon_down_back)
{
	m_currentMBlock->SetReconInfo(skipped, mb_motion_forw, mb_motion_back, recon_right_for, recon_down_for, recon_right_back, recon_down_back);
}

void lwmovie::lwmCM1VBaseBlockCursor::SetBlockInfo(lwmSInt32 blockIndex, bool zero_block_flag)
{
	m_currentBlockBase[blockIndex].SetReconInfo(zero_block_flag);
}

lwmovie::lwmDCTBLOCK *lwmovie::lwmCM1VBaseBlockCursor::StartReconBlock(lwmSInt32 subBlockIndex)
{
	m_openedReconBlock = m_currentBlockBase + subBlockIndex;
	return m_currentDCTBlockBase + subBlockIndex;
}

void lwmovie::lwmCM1VBaseBlockCursor::CommitZero()
{
	m_openedReconBlock->needs_idct = false;
}

void lwmovie::lwmCM1VBaseBlockCursor::CommitSparse(lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff)
{
	lwmBlockInfo *blockInfo = m_openedReconBlock;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = true;
	blockInfo->sparse_idct_coef = lastCoeff;
	blockInfo->sparse_idct_index = lastCoeffPos;
}


void lwmovie::lwmCM1VBaseBlockCursor::CommitFull()
{
	lwmBlockInfo *blockInfo = m_openedReconBlock;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = false;
}


/////////////////////////////////////////////////////////////
lwmovie::lwmIM1VBlockCursor::~lwmIM1VBlockCursor()
{
}
