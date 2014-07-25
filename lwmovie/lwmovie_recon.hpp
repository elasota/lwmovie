#ifndef __LWMOVIE_RECON_HPP__
#define __LWMOVIE_RECON_HPP__

#include "lwmovie_types.hpp"

struct lwmSWorkNotifier;
class lwmCProfileTagSet;

struct lwmIVideoReconstructor
{
	virtual ~lwmIVideoReconstructor();
	virtual void Participate() = 0;
	virtual void GetChannel(lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride) = 0;
	virtual void WaitForFinish() = 0;
	virtual void SetWorkNotifier(const lwmSWorkNotifier *workNotifier) = 0;
	virtual void FlushProfileTags(lwmCProfileTagSet *tagSet) = 0;
};

#endif
