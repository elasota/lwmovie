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


#endif
