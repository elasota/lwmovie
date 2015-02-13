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
#ifndef __LWMOVIE_FP_HPP__
#define __LWMOVIE_FP_HPP__

#include "lwmovie_demux.hpp"

namespace lwmovie
{
	struct lwmIVideoFrameProvider : public lwmSVideoFrameProvider
	{
	public:
		lwmIVideoFrameProvider();
		virtual ~lwmIVideoFrameProvider();

		virtual int CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat) = 0;
		virtual void LockWorkFrame(lwmUInt32 workFrameIndex, lwmUInt32 lockType) = 0;
		virtual void UnlockWorkFrame(lwmUInt32 workFrameIndex) = 0;
		virtual void *GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex) = 0;
		virtual lwmUInt32 GetWorkFramePlaneStride(lwmUInt32 planeIndex) = 0;
		virtual void Destroy() = 0;

	private:
		static int StaticCreateWorkFrames(lwmSVideoFrameProvider *frameProvider, lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);
		static void StaticLockWorkFrame(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 lockType);
		static void StaticUnlockWorkFrame(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex);
		static void *StaticGetWorkFramePlane(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);
		static lwmUInt32 StaticGetWorkFramePlaneStride(lwmSVideoFrameProvider *frameProvider, lwmUInt32 planeIndex);
		static void StaticDestroy(lwmSVideoFrameProvider *frameProvider);
	};
}

#endif
