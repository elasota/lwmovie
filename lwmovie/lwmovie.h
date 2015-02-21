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
#ifndef __LWMOVIE_H__
#define __LWMOVIE_H__

#include "../common/lwmovie_coretypes.h"
#include "lwmovie_api.h"
#include "lwmovie_external_types.h"

struct lwmMovieState;
struct lwmIVideoReconstructor;
class lwmCProfileTagSet;

enum
{
	lwmVIDEOLOCK_Write_Only,		// Target will be written to and never read from
	lwmVIDEOLOCK_Write_ReadLater,	// Target will be written to and possibly read from later
	lwmVIDEOLOCK_Read,				// Target is being read from
};

struct lwmSVideoFrameProvider
{
	int (*createWorkFramesFunc)(lwmSVideoFrameProvider *frameProvider, lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);
	void (*lockWorkFrameFunc)(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 lockType);
	void (*unlockWorkFrameFunc)(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex);
	void *(*getWorkFramePlaneFunc)(lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);
	lwmUInt32 (*getWorkFramePlaneStrideFunc)(lwmSVideoFrameProvider *frameProvider, lwmUInt32 planeIndex);
	void (*destroyFunc)(lwmSVideoFrameProvider *frameProvider);
};

enum lwmEDigestResult
{
	lwmDIGEST_Nothing,
	lwmDIGEST_Worked,
	lwmDIGEST_Error,
	lwmDIGEST_VideoSync,
	lwmDIGEST_AudioSync,
	lwmDIGEST_Initialize,
};

enum lwmEReconstructorType
{
	lwmRC_Unknown,
	lwmRC_MPEG1Video,
};

enum lwmEFrameFormat
{
	lwmFRAMEFORMAT_Unknown,
	lwmFRAMEFORMAT_YUV420P_Planar,
};

enum lwmESpeakerLayout
{
	lwmSPEAKERLAYOUT_Unknown,
	lwmSPEAKERLAYOUT_Mono,
	lwmSPEAKERLAYOUT_Stereo_LR,

	lwmSPEAKERLAYOUT_Count,
};

enum lwmEStreamType
{
	lwmSTREAMTYPE_Video,
	lwmSTREAMTYPE_Audio,

	lwmSTREAMTYPE_Count,
};

enum lwmEStreamParameter
{
	lwmSTREAMPARAM_U32_Width,
	lwmSTREAMPARAM_U32_Height,
	lwmSTREAMPARAM_U32_SyncPeriod,
	lwmSTREAMPARAM_U32_PPSNumerator,
	lwmSTREAMPARAM_U32_PPSDenominator,
	lwmSTREAMPARAM_U32_ReconType,
	lwmSTREAMPARAM_U32_LongestFrameReadAhead,

	lwmSTREAMPARAM_U32_SampleRate,
	lwmSTREAMPARAM_U32_SpeakerLayout,
};

enum lwmEUserFlag
{
	lwmUSERFLAG_None				= 0,

	lwmUSERFLAG_ThreadedDeslicer	= (1 << 0)
};

LWMOVIE_API_LINK void lwmInitialize();
LWMOVIE_API_LINK lwmMovieState *lwmCreateMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags);
LWMOVIE_API_LINK void lwmMovieState_FeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested);
LWMOVIE_API_LINK int lwmMovieState_GetStreamParameterU32(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt32 streamParameterU32, lwmUInt32 *output);
LWMOVIE_API_LINK void lwmMovieState_SetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon);
LWMOVIE_API_LINK void lwmMovieState_VideoDigestParticipate(lwmMovieState *movieState);
LWMOVIE_API_LINK void lwmMovieState_SetVideoDigestWorkNotifier(lwmMovieState *movieState, lwmSWorkNotifier *videoDigestWorkNotifier);
LWMOVIE_API_LINK void lwmMovieState_Destroy(lwmMovieState *movieState);


LWMOVIE_API_LINK int lwmMovieState_IsAudioPlaybackSynchronized(lwmMovieState *movieState);
LWMOVIE_API_LINK int lwmMovieState_SynchronizeAudioPlayback(lwmMovieState *movieState);
LWMOVIE_API_LINK lwmUInt32 lwmMovieState_ReadAudioSamples(lwmMovieState *movieState, void *samples, lwmUInt32 numSamples);
LWMOVIE_API_LINK void lwmMovieState_NotifyAudioPlaybackUnderrun(lwmMovieState *movieState);

LWMOVIE_API_LINK void lwmVideoRecon_Participate(lwmIVideoReconstructor *videoRecon);
LWMOVIE_API_LINK void lwmVideoRecon_SetWorkNotifier(lwmIVideoReconstructor *recon, lwmSWorkNotifier *workNotifier);
LWMOVIE_API_LINK lwmUInt32 lwmVideoRecon_GetWorkFrameIndex(const lwmIVideoReconstructor *recon);
LWMOVIE_API_LINK void lwmVideoRecon_Destroy(lwmIVideoReconstructor *videoRecon);

LWMOVIE_API_LINK lwmSVideoFrameProvider *lwmCreateSystemMemoryFrameProvider(lwmSAllocator *alloc, const lwmMovieState *movieState);
LWMOVIE_API_LINK void lwmSVideoFrameProvider_Destroy(lwmSVideoFrameProvider *frameProvider);

LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmSVideoFrameProvider *frameProvider);
LWMOVIE_API_LINK void lwmIVideoReconstructor_Destroy(lwmIVideoReconstructor *videoRecon);

LWMOVIE_API_LINK void lwmFlushProfileTags(lwmMovieState *movieState, lwmCProfileTagSet *tagSet);

#endif
