#ifndef __LWMOVIE_RECON_M1VSW_HPP__
#define __LWMOVIE_RECON_M1VSW_HPP__

#include "lwmovie_recon_m1v.hpp"

struct lwmSAllocator;

namespace lwmovie
{
	class lwmCM1VSoftwareReconstructor : public lwmIM1VReconstructor
	{
	public:
		struct STarget
		{
			lwmUInt8 *base;
		};

		lwmCM1VSoftwareReconstructor();
		~lwmCM1VSoftwareReconstructor();

		bool Initialize(const lwmSAllocator *alloc, lwmMovieState *movieState);

		virtual lwmDCTBLOCK *StartReconBlock(lwmSInt32 address);

		virtual void SetMBlockInfo(lwmSInt32 mbAddress, bool skipped, bool mb_motion_forw, bool mb_motion_back, lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, bool full_pel_forw, lwmSInt32 recon_right_back, lwmSInt32 recon_down_back, bool full_pel_back);
		virtual void SetBlockInfo(lwmSInt32 sbAddress, bool zero_block_flag);
		virtual void CommitZero(lwmSInt32 sbAddress);
		virtual void CommitSparse(lwmSInt32 sbAddress, lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff);
		virtual void CommitFull(lwmSInt32 sbAddress);
		virtual void MarkRowFinished(lwmSInt32 firstMBAddress);
		virtual void WaitForFinish();

		virtual void Participate();
		virtual void SetWorkNotifier(const lwmSWorkNotifier *workNotifier);
		virtual void GetChannel(lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride);
		virtual void StartNewFrame(lwmUInt32 currentTarget, lwmUInt32 futureTarget, lwmUInt32 pastTarget);

		virtual void FlushProfileTags(lwmCProfileTagSet *tagSet);

		lwmReconMBlock *m_mblocks;
		lwmBlockInfo *m_blocks;
		lwmDCTBLOCK *m_dctBlocks;
		lwmAtomicInt *m_rowCommitCounts;
		lwmAtomicInt *m_workRowUsers;
		lwmCProfileTagSet *m_workRowProfileTags;

		lwmUInt32 m_yuvFrameSize;
		lwmUInt32 m_yStride;
		lwmUInt32 m_uvStride;
		lwmUInt32 m_yOffset;
		lwmUInt32 m_uOffset;
		lwmUInt32 m_vOffset;

		lwmUInt8 *m_yuvBuffer;
		STarget m_currentTarget;
		STarget m_pastTarget;
		STarget m_futureTarget;

		lwmSAllocator m_alloc;
		
		lwmUInt32 m_mbWidth;
		lwmUInt32 m_mbHeight;
		
		lwmSWorkNotifier m_stWorkNotifier;
		const lwmSWorkNotifier *m_workNotifier;

		lwmMovieState *m_movieState;

	private:
		static void PutDCTBlock(const lwmDCTBLOCK *dctBlock, lwmUInt8 *channel, lwmUInt32 stride);
		void ReconstructRow(const lwmReconMBlock *mblocks, const lwmBlockInfo *blocks, lwmDCTBLOCK *dctBlocks, lwmUInt8 *cy, lwmUInt8 *cu, lwmUInt8 *cv,
			lwmUInt8 *fy, lwmUInt8 *fu, lwmUInt8 *fv, lwmUInt8 *py, lwmUInt8 *pu, lwmUInt8 *pv, lwmCProfileTagSet *profileTags);
		void ReconstructBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, bool halfRes, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);
		static void ReconstructLumaBlocks(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);
		static void ReconstructChromaBlock(const lwmReconMBlock *mblock, const lwmBlockInfo *block, lwmDCTBLOCK *dctBlock,
			lwmUInt8 *c, const lwmUInt8 *f, const lwmUInt8 *p, lwmLargeUInt stride, lwmCProfileTagSet *profileTags);

		static void STWNJoinFunc(void *opaque);
		static void STWNNotifyAvailableFunc(void *opaque);
	};
}

#endif
