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
#ifndef __LWMOVIE_CPP_SHIMS_H__
#define __LWMOVIE_CPP_SHIMS_H__

#include "lwmovie.h"

struct lwmIVideoFrameProvider : public lwmSVideoFrameProvider
{
	virtual int CreateWorkFrames(lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat) = 0;
	virtual void LockWorkFrame(lwmUInt32 workFrameIndex, lwmEVideoLockType lockType) = 0;
	virtual void UnlockWorkFrame(lwmUInt32 workFrameIndex) = 0;
	virtual void *GetWorkFramePlane(lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch) = 0;
	virtual lwmUInt32 GetWorkFramePlaneWidth(lwmUInt32 planeIndex) const = 0;
	virtual lwmUInt32 GetWorkFramePlaneHeight(lwmUInt32 planeIndex) const = 0;
	virtual void Destroy() = 0;

private:
	static inline int StaticCreateWorkFrames(struct lwmSVideoFrameProvider *self, lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat)
	{
		return static_cast<lwmIVideoFrameProvider*>(self)->CreateWorkFrames(numRWFrames, numWriteOnlyFrames, workFrameWidth, workFrameHeight, frameFormat);
	}

	static inline void StaticLockWorkFrame(struct lwmSVideoFrameProvider *self, lwmUInt32 workFrameIndex, lwmUInt32 lockType)
	{
		return static_cast<lwmIVideoFrameProvider*>(self)->LockWorkFrame(workFrameIndex, static_cast<lwmEVideoLockType>(lockType));
	}

	static inline void StaticUnlockWorkFrame(struct lwmSVideoFrameProvider *self, lwmUInt32 workFrameIndex)
	{
		static_cast<lwmIVideoFrameProvider*>(self)->UnlockWorkFrame(workFrameIndex);
	}

	static inline void *StaticGetWorkFramePlane(struct lwmSVideoFrameProvider *self, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch)
	{
		return static_cast<lwmIVideoFrameProvider*>(self)->GetWorkFramePlane(workFrameIndex, planeIndex, outPitch);
	}

	static inline lwmUInt32 StaticGetWorkFramePlaneWidth(const struct lwmSVideoFrameProvider *self, lwmUInt32 planeIndex)
	{
		return static_cast<const lwmIVideoFrameProvider*>(self)->GetWorkFramePlaneWidth(planeIndex);
	}

	static inline lwmUInt32 StaticGetWorkFramePlaneHeight(const struct lwmSVideoFrameProvider *self, lwmUInt32 planeIndex)
	{
		return static_cast<const lwmIVideoFrameProvider*>(self)->GetWorkFramePlaneHeight(planeIndex);
	}

	static inline void StaticDestroy(struct lwmSVideoFrameProvider *self)
	{
		static_cast<lwmIVideoFrameProvider*>(self)->Destroy();
	}

public:
	lwmIVideoFrameProvider()
	{
		createWorkFramesFunc = StaticCreateWorkFrames;
		lockWorkFrameFunc = StaticLockWorkFrame;
		unlockWorkFrameFunc = StaticUnlockWorkFrame;
		getWorkFramePlaneFunc = StaticGetWorkFramePlane;
		getWorkFramePlaneWidthFunc = StaticGetWorkFramePlaneWidth;
		getWorkFramePlaneHeightFunc = StaticGetWorkFramePlaneHeight;
		destroyFunc = StaticDestroy;
	}
};


struct lwmIWorkNotifier : public lwmSWorkNotifier
{
public:
	virtual void NotifyAvailable() = 0;
	virtual void Join() = 0;
private:
	static void StaticNotifyAvailable(struct lwmSWorkNotifier *workNotifier)
	{
		static_cast<lwmIWorkNotifier*>(workNotifier)->NotifyAvailable();
	}

	static void StaticJoin(struct lwmSWorkNotifier *workNotifier)
	{
		static_cast<lwmIWorkNotifier*>(workNotifier)->Join();
	}

public:
	lwmIWorkNotifier()
	{
		this->notifyAvailableFunc = StaticNotifyAvailable;
		this->joinFunc = StaticJoin;
	}
};

struct lwmIAllocator : public lwmSAllocator
{
	virtual void *Realloc(void *ptr, lwmLargeUInt sz) = 0;

private:
	static void *StaticRealloc(struct lwmSAllocator *alloc, void *ptr, lwmLargeUInt sz)
	{
		return static_cast<lwmIAllocator*>(alloc)->Realloc(ptr, sz);
	}

public:
	lwmIAllocator()
	{
		this->reallocFunc = StaticRealloc;
	}
};

#endif
