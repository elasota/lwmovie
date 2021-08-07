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
#include "lwmovie_idct.hpp"

#ifdef LWMOVIE_SSE2
#include "lwmovie_recon_m1vsw_sse2.inl"
#endif

#ifdef LWMOVIE_NOSIMD
#include "lwmovie_recon_m1vsw_c.inl"
#endif


lwmovie::m1v::CSoftwareReconstructor::CSoftwareReconstructor(bool isMPEG2)
:	m_mblocks(NULL)
,	m_blocks(NULL)
,	m_dctBlocks(NULL)
,	m_workRowUsers(NULL)
,	m_rowCommitCounts(NULL)
,	m_useRowJobs(false)
,	m_isMPEG2(isMPEG2)
{
}

lwmovie::m1v::CSoftwareReconstructor::~CSoftwareReconstructor()
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

bool lwmovie::m1v::CSoftwareReconstructor::Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState, bool useRowJobs)
{
	lwmUInt32 width, height, numWFrames, numRWFrames;

	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &width);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &height);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumReadWriteWorkFrames, &numRWFrames);
	lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames, &numWFrames);

	lwmUInt32 mbWidth = (width + 15) / 16;
	lwmUInt32 mbHeight = (height + 15) / 16;
	lwmUInt32 numMB = mbWidth * mbHeight;
	m_alloc = alloc;
	m_frameProvider = frameProvider;
	m_useRowJobs = useRowJobs;

	if(numWFrames > 1 || numRWFrames != 2)
		return false;

	if(!frameProvider->createWorkFramesFunc(frameProvider, numRWFrames, numWFrames, mbWidth * 16, mbHeight * 16, lwmFRAMEFORMAT_8Bit_420P_Planar))
		return false;

	if(m_useRowJobs)
	{
		m_mblocks = m_alloc->NAlloc<lwmReconMBlock>(numMB);
		m_blocks = m_alloc->NAlloc<lwmBlockInfo>(numMB * 6);
		m_dctBlocks = m_alloc->NAlloc<idct::DCTBLOCK>(numMB * 6);
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

void lwmovie::m1v::CSoftwareReconstructor::WaitForFinish()
{
	m_workNotifier->joinFunc(m_workNotifier);
}

void lwmovie::m1v::CSoftwareReconstructor::Participate()
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

	SStrideTriplet rowStrideC = m_currentTarget.strides;
	SStrideTriplet rowStrideF = m_futureTarget.strides;
	SStrideTriplet rowStrideP = m_pastTarget.strides;

	SStrideTriplet mbRowStrideC;
	SStrideTriplet mbRowStrideF;
	SStrideTriplet mbRowStrideP;

	rowStrideC.MakeMBStrides(mbRowStrideC);
	rowStrideF.MakeMBStrides(mbRowStrideF);
	rowStrideP.MakeMBStrides(mbRowStrideP);

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
		cy + row*mbRowStrideC.y, cu + row*mbRowStrideC.u, cv + row*mbRowStrideC.v,
		fy + row*mbRowStrideF.y, fu + row*mbRowStrideF.u, fv + row*mbRowStrideF.v,
		py + row*mbRowStrideP.y, pu + row*mbRowStrideP.u, pv + row*mbRowStrideP.v,
		rowStrideC, rowStrideF, rowStrideP,
#ifdef LWMOVIE_PROFILE
		m_workRowProfileTags + row
#else
		NULL
#endif
		);
}

void lwmovie::m1v::CSoftwareReconstructor::SetWorkNotifier(lwmSWorkNotifier *workNotifier)
{
	m_workNotifier = workNotifier;
}

void lwmovie::m1v::CSoftwareReconstructor::STReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *blocks, idct::DCTBLOCK *dctBlocks, lwmSInt32 mbAddress)
{
	lwmUInt32 mbWidth = m_mbWidth;

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

	SStrideTriplet cMBStrides, fMBStrides, pMBStrides;
	SStrideTriplet cStrides = m_currentTarget.strides;
	SStrideTriplet fStrides = m_futureTarget.strides;
	SStrideTriplet pStrides = m_pastTarget.strides;
	cStrides.MakeMBStrides(cMBStrides);
	fStrides.MakeMBStrides(fMBStrides);
	pStrides.MakeMBStrides(pMBStrides);
	lwmUInt32 lumaColOffset = col * 16;
	lwmUInt32 chromaColOffset = col * 8;

	lwmUInt8 *cy = m_currentTarget.yPlane;
	lwmUInt8 *cu = m_currentTarget.uPlane;
	lwmUInt8 *cv = m_currentTarget.vPlane;
	lwmUInt8 *py = m_pastTarget.yPlane;
	lwmUInt8 *pu = m_pastTarget.uPlane;
	lwmUInt8 *pv = m_pastTarget.vPlane;
	lwmUInt8 *fy = m_futureTarget.yPlane;
	lwmUInt8 *fu = m_futureTarget.uPlane;
	lwmUInt8 *fv = m_futureTarget.vPlane;

	ReconstructLumaBlocks(mblock, blocks, dctBlocks,
		cy + cMBStrides.y * row + lumaColOffset, fy + fMBStrides.y * row + lumaColOffset, py + pMBStrides.y * row + lumaColOffset,
		cStrides.y, fStrides.y, pStrides.y,
		NULL);
	ReconstructChromaBlock(mblock, blocks + 4, dctBlocks + 4,
		cu + cMBStrides.u * row + chromaColOffset, fu + fMBStrides.u * row + chromaColOffset, pu + pMBStrides.u * row + chromaColOffset,
		cStrides.u, fStrides.u, pStrides.u,
		NULL);
	ReconstructChromaBlock(mblock, blocks + 5, dctBlocks + 5,
		cv + cMBStrides.v * row + chromaColOffset, fv + fMBStrides.v * row + chromaColOffset, pv + pMBStrides.v * row + chromaColOffset,
		cStrides.v, fStrides.v, pStrides.v,
		NULL);
}

void lwmovie::m1v::CSoftwareReconstructor::ReconstructRow(lwmUInt32 row, const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlocks,
	lwmUInt8 *cy, lwmUInt8 *cu, lwmUInt8 *cv,
	lwmUInt8 *fy, lwmUInt8 *fu, lwmUInt8 *fv,
	lwmUInt8 *py, lwmUInt8 *pu, lwmUInt8 *pv,
	SStrideTriplet cStride, SStrideTriplet fStride, SStrideTriplet pStride,
	lwmCProfileTagSet *profileTags
	)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ReconRow);
#endif

	lwmUInt32 mbWidth = m_mbWidth;

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

		ReconstructLumaBlocks(mblock, block, dctBlocks, cy, fy, py, cStride.y, fStride.y, pStride.y, profileTags);
		ReconstructChromaBlock(mblock, block + 4, dctBlocks + 4, cu, fu, pu, cStride.u, fStride.u, pStride.u, profileTags);
		ReconstructChromaBlock(mblock, block + 5, dctBlocks + 5, cv, fv, pv, cStride.v, fStride.v, pStride.v, profileTags);

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

void lwmovie::m1v::CSoftwareReconstructor::ReconstructLumaBlocks(const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlock, lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt cstride, lwmLargeUInt fstride, lwmLargeUInt pstride, lwmCProfileTagSet *profileTags)
{
	lwmUInt8 bytes[16+256+256];

	lwmLargeUInt alignOffs = (bytes + 15 - static_cast<const lwmUInt8 *>(0)) & 0xf;
	lwmUInt8 *alignedBytes = bytes + 15 - alignOffs;

	lwmUInt8 *motion = alignedBytes + 0;
	lwmUInt8 *mergeMotion = alignedBytes + 256;

	lwmSInt32 reconDownFor = mblock->recon_down_for;
	lwmSInt32 reconRightFor = mblock->recon_right_for;
	lwmSInt32 reconDownBack = mblock->recon_down_back;
	lwmSInt32 reconRightBack = mblock->recon_right_back;

	if((!mblock->mb_motion_forw) && (!mblock->mb_motion_back))
	{
		// No motion
		
		if(mblock->skipped)
		{
			// Skipped MB, motion compensation only
			reconutil::ZeroLumaBlockPair(c, cstride);
			reconutil::ZeroLumaBlockPair(c + cstride * 8, cstride);
		}
		else
		{
			// Apply DCT
			if(!block[0].zero_block_flag)
			{
				block[0].IDCT(dctBlock);
				if(!block[1].zero_block_flag)
				{
					// 0+1
					block[1].IDCT(dctBlock + 1);
					reconutil::SetLumaDCTPaired(c, dctBlock, cstride);
				}
				else
				{
					// 0 only
					reconutil::SetLumaDCTLow(c, dctBlock, cstride);
				}
			}
			else
			{
				if(!block[1].zero_block_flag)
				{
					// 1 only
					block[1].IDCT(dctBlock + 1);
					reconutil::SetLumaDCTHigh(c, dctBlock, cstride);
				}
				else
				{
					// No DCT
					reconutil::ZeroLumaBlockPair(c, cstride);
				}
			}

			lwmUInt8 *c2 = c + cstride * 8;

			if(!block[2].zero_block_flag)
			{
				block[2].IDCT(dctBlock + 2);
				if(!block[3].zero_block_flag)
				{
					// 2+3
					block[3].IDCT(dctBlock + 3);
					reconutil::SetLumaDCTPaired(c2, dctBlock + 2, cstride);
				}
				else
				{
					// 2 only
					reconutil::SetLumaDCTLow(c2, dctBlock + 2, cstride);
				}
			}
			else
			{
				if(!block[3].zero_block_flag)
				{
					// 3 only
					block[3].IDCT(dctBlock + 3);
					reconutil::SetLumaDCTHigh(c2, dctBlock + 2, cstride);
				}
				else
				{
					// No DCT
					reconutil::ZeroLumaBlockPair(c2, cstride);
				}
			}
		}

		// End of no-motion
	}
	else
	{
		if(mblock->mb_motion_forw)
		{
			reconutil::ExtractMotionLuma(motion, f, reconRightFor, reconDownFor, fstride, profileTags);
			if(mblock->mb_motion_back)
			{
				reconutil::ExtractMotionLuma(mergeMotion, p, reconRightBack, reconDownBack, pstride, profileTags);
				reconutil::MergeLumaMotion(motion, mergeMotion);
			}
		}
		else //if(mblock->mb_motion_back)
		{
			reconutil::ExtractMotionLuma(motion, p, reconRightBack, reconDownBack, pstride, profileTags);
		}

		if(mblock->skipped)
		{
			// Skipped MB, motion compensation only
			reconutil::CopyLumaBlockPair(c, motion, cstride);
			reconutil::CopyLumaBlockPair(c + cstride * 8, motion + 128, cstride);
		}
		else
		{
			// Apply DCT
			if(!block[0].zero_block_flag)
			{
				block[0].IDCT(dctBlock);
				if(!block[1].zero_block_flag)
				{
					// 0+1
					block[1].IDCT(dctBlock +1);
					reconutil::ApplyLumaDCTPaired(c, motion, dctBlock, cstride);
				}
				else
				{
					// 0 only
					reconutil::ApplyLumaDCTLow(c, motion, dctBlock, cstride);
				}
			}
			else
			{
				if(!block[1].zero_block_flag)
				{
					// 1 only
					block[1].IDCT(dctBlock + 1);
					reconutil::ApplyLumaDCTHigh(c, motion, dctBlock, cstride);
				}
				else
				{
					// No DCT
					reconutil::CopyLumaBlockPair(c, motion, cstride);
				}
			}

			lwmUInt8 *c2 = c + cstride * 8;

			if(!block[2].zero_block_flag)
			{
				block[2].IDCT(dctBlock + 2);
				if(!block[3].zero_block_flag)
				{
					// 2+3
					block[3].IDCT(dctBlock + 3);
					reconutil::ApplyLumaDCTPaired(c2, motion + 128, dctBlock + 2, cstride);
				}
				else
				{
					// 2 only
					reconutil::ApplyLumaDCTLow(c2, motion + 128, dctBlock + 2, cstride);
				}
			}
			else
			{
				if(!block[3].zero_block_flag)
				{
					// 3 only
					block[3].IDCT(dctBlock + 3);
					reconutil::ApplyLumaDCTHigh(c2, motion + 128, dctBlock + 2, cstride);
				}
				else
				{
					// No DCT
					reconutil::CopyLumaBlockPair(c2, motion + 128, cstride);
				}
			}
		}

		// End of motion + DCT
	}
}

void lwmovie::m1v::CSoftwareReconstructor::ReconstructChromaBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlock, lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt cstride, lwmLargeUInt fstride, lwmLargeUInt pstride, lwmCProfileTagSet *profileTags)
{
	lwmUInt8 bytes[16+64+64];

	lwmLargeUInt alignOffs = (bytes + 15 - static_cast<const lwmUInt8 *>(0)) & 0xf;
	lwmUInt8 *alignedBytes = bytes + 15 - alignOffs;

	lwmUInt8 *motion = alignedBytes + 0;
	lwmUInt8 *mergeMotion = alignedBytes + 64;

	lwmSInt32 reconDownFor = mblock->recon_down_for;
	lwmSInt32 reconRightFor = mblock->recon_right_for;
	lwmSInt32 reconDownBack = mblock->recon_down_back;
	lwmSInt32 reconRightBack = mblock->recon_right_back;

	// Note: Divide, not shift.  Chroma MVs round towards zero.
	reconDownFor = reconDownFor / 2;
	reconRightFor = reconRightFor / 2;
	reconDownBack = reconDownBack / 2;
	reconRightBack = reconRightBack / 2;

	if((!mblock->mb_motion_forw) && (!mblock->mb_motion_back))
	{
		// No motion
		
		if(mblock->skipped)
		{
			// Skipped MB, motion compensation only
			reconutil::ZeroChromaBlock(c, cstride);
		}
		else
		{
			// Apply DCT
			if(!block->zero_block_flag)
			{
				block->IDCT(dctBlock);
				reconutil::SetChromaDCT(c, dctBlock, cstride);
			}
			else
				reconutil::ZeroChromaBlock(c, cstride);
		}

		// End of no-motion
	}
	else
	{
		if(mblock->mb_motion_forw)
		{
			reconutil::ExtractMotionChroma(motion, f, reconRightFor, reconDownFor, fstride, profileTags);
			if(mblock->mb_motion_back)
			{
				reconutil::ExtractMotionChroma(mergeMotion, p, reconRightBack, reconDownBack, pstride, profileTags);
				reconutil::MergeChromaMotion(motion, mergeMotion);
			}
		}
		else //if(mblock->mb_motion_back)
		{
			reconutil::ExtractMotionChroma(motion, p, reconRightBack, reconDownBack, pstride, profileTags);
		}

		if(mblock->skipped)
		{
			// Skipped MB, motion compensation only
			reconutil::CopyChromaBlock(c, motion, cstride);
		}
		else
		{
			// Apply DCT
			if(!block->zero_block_flag)
			{
				block->IDCT(dctBlock);
				reconutil::ApplyChromaDCT(c, motion, dctBlock, cstride);
			}
			else
				reconutil::CopyChromaBlock(c, motion, cstride);
		}

		// End of motion + DCT
	}
}

void lwmovie::m1v::CSoftwareReconstructor::StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past, bool currentIsB)
{
	this->CloseFrame();

	if(current == lwmRECONSLOT_Dropped_IP || current == lwmRECONSLOT_Dropped_B)
		return;

	if(m_useRowJobs)
		for(lwmUInt32 i=0;i<m_mbHeight;i++)
			m_rowCommitCounts[i] = m_workRowUsers[i] = 0;

	m_frameProvider->lockWorkFrameFunc(m_frameProvider, current - 1, currentIsB ? lwmVIDEOLOCK_Write_Only : lwmVIDEOLOCK_Write_ReadLater);
	m_currentTarget.LoadFromFrameProvider(m_frameProvider, current - 1);
	if(future && future != current)
	{
		m_frameProvider->lockWorkFrameFunc(m_frameProvider, future - 1, lwmVIDEOLOCK_Read);
		m_futureTarget.LoadFromFrameProvider(m_frameProvider, future - 1);
	}
	if (past && past != current)
	{
		m_frameProvider->lockWorkFrameFunc(m_frameProvider, past - 1, lwmVIDEOLOCK_Read);
		m_pastTarget.LoadFromFrameProvider(m_frameProvider, past - 1);
	}
}

void lwmovie::m1v::CSoftwareReconstructor::CloseFrame()
{
	m_currentTarget.Close(m_frameProvider);
	m_futureTarget.Close(m_frameProvider);
	m_pastTarget.Close(m_frameProvider);
}

void lwmovie::m1v::CSoftwareReconstructor::PresentFrame(lwmUInt32 slot)
{
	CloseFrame();
	this->m_outputTarget = slot - 1;
}

lwmovie::m1v::IM1VBlockCursor *lwmovie::m1v::CSoftwareReconstructor::CreateBlockCursor()
{
	if(!m_useRowJobs)
	{
		CSelfContainedBlockCursor *blockCursor = m_alloc->NAlloc<CSelfContainedBlockCursor>(1);
		if(blockCursor)
		{
			new (blockCursor)CSelfContainedBlockCursor(this);
			return blockCursor;
		}
		else
			return NULL;
	}

	CGridBlockCursor *blockCursor = m_alloc->NAlloc<CGridBlockCursor>(1);
	if(blockCursor)
	{
		new (blockCursor)CGridBlockCursor(m_mblocks, m_blocks, m_dctBlocks);
		return blockCursor;
	}
	else
		return NULL;
}

void lwmovie::m1v::CSoftwareReconstructor::MarkRowFinished(lwmSInt32 firstMBAddress)
{
	if(m_useRowJobs)
	{
		lwmUInt32 row = static_cast<lwmUInt32>(firstMBAddress) / m_mbWidth;

		// Check commit count, should be zero.  If non-zero, the video is bad and is committing the same row multiple times
		if(lwmAtomicIncrement(m_rowCommitCounts + row) == 1)
			m_workNotifier->notifyAvailableFunc(m_workNotifier);
	}
}

void lwmovie::m1v::CSoftwareReconstructor::FlushProfileTags(lwmCProfileTagSet *tagSet)
{
#ifdef LWMOVIE_PROFILE
	for(lwmUInt32 i=0;i<m_mbHeight;i++)
		m_workRowProfileTags[i].FlushTo(tagSet);
#endif
}

lwmSVideoFrameProvider *lwmovie::m1v::CSoftwareReconstructor::GetFrameProvider() const
{
	return this->m_frameProvider;
}

void lwmovie::m1v::CSoftwareReconstructor::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	m1v::CSoftwareReconstructor *self = this;
	self->~CSoftwareReconstructor();
	alloc->Free(self);
}

lwmUInt32 lwmovie::m1v::CSoftwareReconstructor::GetWorkFrameIndex() const
{
	return m_outputTarget;
}

//////////////////////////////////////////////////////////////////
void lwmovie::m1v::CSoftwareReconstructor::STarget::LoadFromFrameProvider(lwmSVideoFrameProvider *frameProvider, lwmUInt32 frameIndex)
{
	this->yPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 0, &this->strides.y));
	this->uPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 1, &this->strides.u));
	this->vPlane = static_cast<lwmUInt8*>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 2, &this->strides.v));
	this->frameIndex = frameIndex;
	this->isOpen = true;
}

void lwmovie::m1v::CSoftwareReconstructor::STarget::Close(lwmSVideoFrameProvider *frameProvider)
{
	if(this->isOpen)
	{
		this->isOpen = false;
		frameProvider->unlockWorkFrameFunc(frameProvider, this->frameIndex);
	}
}

void lwmovie::m1v::CSoftwareReconstructor::STWNJoinFunc(lwmSWorkNotifier *workNotifier)
{
}

void lwmovie::m1v::CSoftwareReconstructor::STWNNotifyAvailableFunc(lwmSWorkNotifier *workNotifier)
{
	STWorkNotifier *typeWN = static_cast<STWorkNotifier*>(workNotifier);
	static_cast<STWorkNotifier*>(workNotifier)->recon->Participate();
}

//////////////////////////////////////////////////////////////////
// Grid block cursor
lwmovie::m1v::CGridBlockCursor::CGridBlockCursor(lwmReconMBlock *mblocks, lwmBlockInfo *blocks, idct::DCTBLOCK *dctBlocks)
	: m_mblocks(mblocks)
	, m_blocks(blocks)
	, m_dctBlocks(dctBlocks)
{
}

void lwmovie::m1v::CGridBlockCursor::OpenMB(lwmSInt32 mbAddress)
{
	m_currentMBlock = m_mblocks + mbAddress;
	m_currentBlockBase = m_blocks + mbAddress*6;
	m_currentDCTBlockBase = m_dctBlocks + mbAddress*6;
}

//////////////////////////////////////////////////////////////////
// Self-contained block cursor
lwmovie::m1v::CSelfContainedBlockCursor::CSelfContainedBlockCursor(lwmovie::m1v::CSoftwareReconstructor *recon)
	: m_targetMBAddress(0)
	, m_recon(recon)
{
}

void lwmovie::m1v::CSelfContainedBlockCursor::OpenMB(lwmSInt32 mbAddress)
{
	m_targetMBAddress = mbAddress;
	m_currentDCTBlockBase = m_dctBlocks;
	m_currentBlockBase = m_blocks;
	m_currentMBlock = &m_mblock;
}

void lwmovie::m1v::CSelfContainedBlockCursor::CloseMB()
{
	m_recon->STReconstructBlock(&m_mblock, m_blocks, m_dctBlocks, m_targetMBAddress);
}


//////////////////////////////////////////////////////////////////
// Base block cursor
lwmovie::m1v::CBaseBlockCursor::CBaseBlockCursor()
	: m_currentMBlock(NULL)
	, m_currentBlockBase(NULL)
	, m_currentDCTBlockBase(NULL)
	, m_openedReconBlock(NULL)
{
}

void lwmovie::m1v::CBaseBlockCursor::CloseMB()
{
}

void lwmovie::m1v::CBaseBlockCursor::SetMBlockInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back,
			lwmSInt32 recon_right_for, lwmSInt32 recon_down_for,
			lwmSInt32 recon_right_back, lwmSInt32 recon_down_back)
{
	m_currentMBlock->SetReconInfo(skipped, mb_motion_forw, mb_motion_back, recon_right_for, recon_down_for, recon_right_back, recon_down_back);
}

void lwmovie::m1v::CBaseBlockCursor::SetBlockInfo(lwmSInt32 blockIndex, bool zero_block_flag)
{
	m_currentBlockBase[blockIndex].SetReconInfo(zero_block_flag);
}

lwmovie::idct::DCTBLOCK *lwmovie::m1v::CBaseBlockCursor::StartReconBlock(lwmSInt32 subBlockIndex)
{
	m_openedReconBlock = m_currentBlockBase + subBlockIndex;
	return m_currentDCTBlockBase + subBlockIndex;
}

void lwmovie::m1v::CBaseBlockCursor::CommitZero()
{
	m_openedReconBlock->needs_idct = false;
}

void lwmovie::m1v::CBaseBlockCursor::CommitSparse(lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff)
{
	lwmBlockInfo *blockInfo = m_openedReconBlock;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = true;
	blockInfo->sparse_idct_coef = lastCoeff;
	blockInfo->sparse_idct_index = lastCoeffPos;
}

void lwmovie::m1v::CBaseBlockCursor::CommitFull()
{
	lwmBlockInfo *blockInfo = m_openedReconBlock;
	blockInfo->needs_idct = true;
	blockInfo->sparse_idct = false;
}

/////////////////////////////////////////////////////////////
lwmovie::m1v::IM1VBlockCursor::~IM1VBlockCursor()
{
}
