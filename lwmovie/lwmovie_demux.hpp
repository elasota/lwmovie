#ifndef __LWMOVIE_DEMUX_HPP__
#define __LWMOVIE_DEMUX_HPP__

#include "lwmovie_types.hpp"
#include "lwmovie_external_types.h"

struct lwmMovieState;
struct lwmIVideoReconstructor;

enum
{
	lwmDIGEST_Nothing,
	lwmDIGEST_Worked,
	lwmDIGEST_Error,
	lwmDIGEST_VideoSync,
	lwmDIGEST_AudioSync,
	lwmDIGEST_Initialize,
};

enum
{
	lwmRC_Unknown,
	lwmRC_MPEG1Video,
};

extern "C" void lwmFeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested);
extern "C" void lwmGetVideoParameters(const lwmMovieState *movieState, lwmUInt32 *outWidth, lwmUInt32 *outHeight, lwmUInt32 *outPPSNum, lwmUInt32 *outPPSDenom, lwmUInt32 *outReconstructorType);
extern "C" void lwmCreateSoftwareVideoReconstructor(const lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, lwmUInt32 reconstructorType, const lwmSWorkNotifier *workNotifier, lwmIVideoReconstructor **outReconstructor);
extern "C" void lwmReconstructorParticipate(lwmIVideoReconstructor *reconstructor);
extern "C" void lwmSetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon);
extern "C" void lwmReconParticipate(lwmIVideoReconstructor *recon);
extern "C" void lwmReconGetChannel(lwmIVideoReconstructor *recon, lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride);
extern "C" lwmMovieState *lwmInitialize(const lwmSAllocator *alloc);

#endif
