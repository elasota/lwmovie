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
#ifndef __LWMOVIE_RECON_M1VSW_HPP__
#define __LWMOVIE_RECON_M1VSW_HPP__

#include "lwmovie_recon_m1v.hpp"

struct lwmSAllocator;
struct lwmSVideoFrameProvider;

namespace lwmovie
{
	namespace m1v
	{
		class CSoftwareReconstructor;

		class CBaseBlockCursor : public IM1VBlockCursor
		{
		public:
			CBaseBlockCursor();

			virtual void CloseMB();
			virtual void SetMBlockInfo(bool skipped, bool mb_motion_forw, bool mb_motion_back,
				lwmSInt32 recon_right_for, lwmSInt32 recon_down_for,
				lwmSInt32 recon_right_back, lwmSInt32 recon_down_back);

			virtual void SetBlockInfo(lwmSInt32 blockIndex, bool zero_block_flag);
			virtual idct::DCTBLOCK *StartReconBlock(lwmSInt32 subBlockIndex);

			virtual void CommitZero();
			virtual void CommitSparse(lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff);
			virtual void CommitFull();

		protected:
			lwmReconMBlock *m_currentMBlock;
			lwmBlockInfo *m_currentBlockBase;
			idct::DCTBLOCK *m_currentDCTBlockBase;

			lwmBlockInfo *m_openedReconBlock;
		};


		class CGridBlockCursor : public CBaseBlockCursor
		{
		public:
			CGridBlockCursor(lwmReconMBlock *mblocks, lwmBlockInfo *blocks, idct::DCTBLOCK *dctBlocks);

			virtual void OpenMB(lwmSInt32 mbAddress);

		private:
			lwmReconMBlock *m_mblocks;
			lwmBlockInfo *m_blocks;
			idct::DCTBLOCK *m_dctBlocks;
		};

		class CSelfContainedBlockCursor : public CBaseBlockCursor
		{
		public:
			explicit CSelfContainedBlockCursor(CSoftwareReconstructor *recon);

			virtual void OpenMB(lwmSInt32 mbAddress);
			virtual void CloseMB();

		private:
			CSoftwareReconstructor *m_recon;
			lwmSInt32 m_targetMBAddress;

			lwmReconMBlock m_mblock;
			lwmBlockInfo m_blocks[6];

			idct::DCTBLOCK m_dctBlocks[6];
		};

		class CSoftwareReconstructor : public IM1VReconstructor
		{
		public:
			explicit CSoftwareReconstructor(bool isMPEG2);
			~CSoftwareReconstructor();

			bool Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState, bool useRowJobs);

			virtual IM1VBlockCursor *CreateBlockCursor();

			virtual void MarkRowFinished(lwmSInt32 firstMBAddress);
			virtual void WaitForFinish();
			virtual void PresentFrame(lwmUInt32 workFrame);

			virtual void Participate();
			virtual void SetWorkNotifier(lwmSWorkNotifier *workNotifier);
			virtual lwmUInt32 GetWorkFrameIndex() const;
			virtual void StartNewFrame(lwmUInt32 currentTarget, lwmUInt32 futureTarget, lwmUInt32 pastTarget, bool currentIsB);
			virtual void CloseFrame();

			virtual void FlushProfileTags(lwmCProfileTagSet *tagSet);
			virtual lwmSVideoFrameProvider *GetFrameProvider() const;

			virtual void Destroy();

			void STReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlock, lwmSInt32 mbAddress);

		private:
			struct STWorkNotifier : public lwmSWorkNotifier
			{
				CSoftwareReconstructor *recon;
			};

			struct SStrideTriplet
			{
				lwmUInt32 y, u, v;

				inline void MakeMBStrides(SStrideTriplet &dest) const
				{
					dest.y = this->y * 16;
					dest.u = this->u * 8;
					dest.v = this->v * 8;
				}
			};

			struct STarget
			{
				lwmUInt8 *yPlane;
				lwmUInt8 *uPlane;
				lwmUInt8 *vPlane;
				SStrideTriplet strides;
				lwmUInt32 frameIndex;
				bool isOpen;

				void LoadFromFrameProvider(lwmSVideoFrameProvider *frameProvider, lwmUInt32 frameIndex);
				void Close(lwmSVideoFrameProvider *frameProvider);
			};

			void ReconstructRow(lwmUInt32 row, const lwmReconMBlock *mblocks, const lwmBlockInfo *blocks, idct::DCTBLOCK *dctBlocks,
				lwmUInt8 *cy, lwmUInt8 *cu, lwmUInt8 *cv,
				lwmUInt8 *fy, lwmUInt8 *fu, lwmUInt8 *fv,
				lwmUInt8 *py, lwmUInt8 *pu, lwmUInt8 *pv,
				SStrideTriplet cStride, SStrideTriplet fStride, SStrideTriplet pStride,
				lwmCProfileTagSet *profileTags);
			static void ReconstructLumaBlocks(const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlock,
				lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt cstride, lwmLargeUInt fstride, lwmLargeUInt pstride, lwmCProfileTagSet *profileTags);
			static void ReconstructChromaBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, idct::DCTBLOCK *dctBlock,
				lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt cstride, lwmLargeUInt fstride, lwmLargeUInt pstride, lwmCProfileTagSet *profileTags);

			static void STWNJoinFunc(lwmSWorkNotifier *workNotifier);
			static void STWNNotifyAvailableFunc(lwmSWorkNotifier *workNotifier);

			bool m_useRowJobs;
			bool m_isMPEG2;

			lwmReconMBlock *m_mblocks;
			lwmBlockInfo *m_blocks;
			idct::DCTBLOCK *m_dctBlocks;
			lwmAtomicInt *m_rowCommitCounts;
			lwmAtomicInt *m_workRowUsers;
#ifdef LWMOVIE_PROFILE
			lwmCProfileTagSet *m_workRowProfileTags;
#endif

			STarget m_currentTarget;
			STarget m_pastTarget;
			STarget m_futureTarget;

			lwmUInt32 m_outputTarget;

			lwmSAllocator *m_alloc;
			lwmSVideoFrameProvider *m_frameProvider;

			lwmUInt32 m_mbWidth;
			lwmUInt32 m_mbHeight;

			STWorkNotifier m_stWorkNotifier;
			lwmSWorkNotifier *m_workNotifier;

			lwmMovieState *m_movieState;
		};
	}
}

#endif
