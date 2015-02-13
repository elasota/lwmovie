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
	class lwmCM1VSoftwareReconstructor : public lwmIM1VReconstructor
	{
	public:
		lwmCM1VSoftwareReconstructor();
		~lwmCM1VSoftwareReconstructor();

		bool Initialize(lwmSAllocator *alloc, lwmSVideoFrameProvider *frameProvider, lwmMovieState *movieState);

		virtual lwmDCTBLOCK *StartReconBlock(lwmSInt32 address);

		virtual void SetMBlockInfo(lwmSInt32 mbAddress, bool skipped, bool mb_motion_forw, bool mb_motion_back, lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, bool full_pel_forw, lwmSInt32 recon_right_back, lwmSInt32 recon_down_back, bool full_pel_back);
		virtual void SetBlockInfo(lwmSInt32 sbAddress, bool zero_block_flag);
		virtual void CommitZero(lwmSInt32 sbAddress);
		virtual void CommitSparse(lwmSInt32 sbAddress, lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff);
		virtual void CommitFull(lwmSInt32 sbAddress);
		virtual void MarkRowFinished(lwmSInt32 firstMBAddress);
		virtual void WaitForFinish();
		virtual void PresentFrame(lwmUInt32 workFrame);

		virtual void Participate();
		virtual void SetWorkNotifier(lwmSWorkNotifier *workNotifier);
		virtual lwmUInt32 GetWorkFrameIndex() const;
		virtual void StartNewFrame(lwmUInt32 currentTarget, lwmUInt32 futureTarget, lwmUInt32 pastTarget, bool currentIsB);
		virtual void CloseFrame();

		virtual void FlushProfileTags(lwmCProfileTagSet *tagSet);

		virtual void Destroy();

	private:
		struct STWorkNotifier : public lwmSWorkNotifier
		{
			lwmCM1VSoftwareReconstructor *recon;
		};

		struct STarget
		{
			lwmUInt8 *yPlane;
			lwmUInt8 *uPlane;
			lwmUInt8 *vPlane;
			lwmUInt32 frameIndex;
			bool isOpen;

			void LoadFromFrameProvider(lwmSVideoFrameProvider *frameProvider, lwmUInt32 frameIndex);
			void Close(lwmSVideoFrameProvider *frameProvider);
		};

		static void PutDCTBlock(const lwmDCTBLOCK *dctBlock, lwmUInt8 *channel, lwmUInt32 stride);
		void ReconstructRow(const lwmReconMBlock *mblocks, const lwmBlockInfo *blocks, lwmDCTBLOCK *dctBlocks, lwmUInt8 *cy, lwmUInt8 *cu, lwmUInt8 *cv,
			lwmUInt8 *fy, lwmUInt8 *fu, lwmUInt8 *fv, lwmUInt8 *py, lwmUInt8 *pu, lwmUInt8 *pv, lwmCProfileTagSet *profileTags);
		void ReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, bool halfRes, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);
		static void ReconstructLumaBlocks(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);
		static void ReconstructChromaBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);

		static void STWNJoinFunc(lwmSWorkNotifier *workNotifier);
		static void STWNNotifyAvailableFunc(lwmSWorkNotifier *workNotifier);
		
		lwmReconMBlock *m_mblocks;
		lwmBlockInfo *m_blocks;
		lwmDCTBLOCK *m_dctBlocks;
		lwmAtomicInt *m_rowCommitCounts;
		lwmAtomicInt *m_workRowUsers;
#ifdef LWMOVIE_PROFILE
		lwmCProfileTagSet *m_workRowProfileTags;
#endif

		STarget m_currentTarget;
		STarget m_pastTarget;
		STarget m_futureTarget;

		lwmUInt32 m_outputTarget;

		lwmUInt32 m_yStride;
		lwmUInt32 m_uStride;
		lwmUInt32 m_vStride;

		lwmSAllocator *m_alloc;
		lwmSVideoFrameProvider *m_frameProvider;
		
		lwmUInt32 m_mbWidth;
		lwmUInt32 m_mbHeight;

		STWorkNotifier m_stWorkNotifier;
		lwmSWorkNotifier *m_workNotifier;

		lwmMovieState *m_movieState;
	};
}

#endif
