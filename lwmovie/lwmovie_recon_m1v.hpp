#ifndef __LWMOVIE_RECON_M1V_HPP__
#define __LWMOVIE_RECON_M1V_HPP__

#include "lwmovie_recon.hpp"
#include "lwmovie_videotypes.hpp"

namespace lwmovie
{
	struct lwmIM1VReconstructor : public lwmIVideoReconstructor
	{
	public:
		virtual lwmDCTBLOCK *StartReconBlock(lwmSInt32 address) = 0;
		virtual void SetMBlockInfo(lwmSInt32 mbAddress, bool skipped, bool mb_motion_forw, bool mb_motion_back,
			lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, bool full_pel_forw,
			lwmSInt32 recon_right_back, lwmSInt32 recon_down_back, bool full_pel_back) = 0;
		virtual void SetBlockInfo(lwmSInt32 sbAddress, bool zero_block_flag) = 0;
		virtual void CommitZero(lwmSInt32 address) = 0;
		virtual void CommitSparse(lwmSInt32 address, lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff) = 0;
		virtual void CommitFull(lwmSInt32 address) = 0;
		virtual void MarkRowFinished(lwmSInt32 firstMBAddress) = 0;
		virtual void WaitForFinish() = 0;

		virtual void StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past) = 0;

		virtual void Participate() = 0;
	};
}

#endif
