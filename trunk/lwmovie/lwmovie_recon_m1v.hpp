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

		virtual void StartNewFrame(lwmUInt32 current, lwmUInt32 future, lwmUInt32 past, bool currentIsB) = 0;
		virtual void PresentFrame(lwmUInt32 outFrame) = 0;

		virtual void Participate() = 0;
	};
}

#endif