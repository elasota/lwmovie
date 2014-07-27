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
#include <new>
#include "lwmovie_fp.hpp"

int lwmovie::lwmIVideoFrameProvider::StaticCreateWorkFrames(lwmSVideoFrameProvider *frameProvider, lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat)
{
	return static_cast<lwmIVideoFrameProvider *>(frameProvider)->CreateWorkFrames(numRWFrames, numWriteOnlyFrames, workFrameWidth, workFrameHeight, frameFormat);
}

void lwmovie::lwmIVideoFrameProvider::StaticLockWorkFrame(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 lockType)
{
	static_cast<lwmIVideoFrameProvider *>(frameProvider)->LockWorkFrame(workFrameIndex, lockType);
}

void lwmovie::lwmIVideoFrameProvider::StaticUnlockWorkFrame(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex)
{
	static_cast<lwmIVideoFrameProvider *>(frameProvider)->UnlockWorkFrame(workFrameIndex);
}

void *lwmovie::lwmIVideoFrameProvider::StaticGetWorkFramePlane(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex)
{
	return static_cast<lwmIVideoFrameProvider *>(frameProvider)->GetWorkFramePlane(workFrameIndex, planeIndex);
}

lwmUInt32 lwmovie::lwmIVideoFrameProvider::StaticGetWorkFramePlaneStride(lwmSVideoFrameProvider *frameProvider, lwmUInt32 planeIndex)
{
	return static_cast<lwmIVideoFrameProvider *>(frameProvider)->GetWorkFramePlaneStride(planeIndex);
}

lwmovie::lwmIVideoFrameProvider::lwmIVideoFrameProvider()
{
	this->createWorkFramesFunc = StaticCreateWorkFrames;
	this->lockWorkFrameFunc = StaticLockWorkFrame;
	this->unlockWorkFrameFunc = StaticUnlockWorkFrame;
	this->getWorkFramePlaneFunc = StaticGetWorkFramePlane;
	this->getWorkFramePlaneStrideFunc = StaticGetWorkFramePlaneStride;
}

lwmovie::lwmIVideoFrameProvider::~lwmIVideoFrameProvider()
{
}
