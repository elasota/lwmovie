#ifndef __LWMOVIE_RECON_HPP__
#define __LWMOVIE_RECON_HPP__

#include "lwmovie_types.hpp"

struct lwmIVideoReconstructor
{
	virtual ~lwmIVideoReconstructor();
	virtual void Participate() = 0;
	virtual void GetChannel(lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride) = 0;
	virtual void WaitForFinish() = 0;
};

#endif
