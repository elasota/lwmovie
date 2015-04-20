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
LWMOVIE_API_CLASS lwmCProfileTagSet;

enum lwmEVideoLockType
{
	lwmVIDEOLOCK_None,
	lwmVIDEOLOCK_Write_Only,		// Target will be written to and never read from
	lwmVIDEOLOCK_Write_ReadLater,	// Target will be written to and possibly read from later
	lwmVIDEOLOCK_Read,				// Target is being read from
};

enum lwmEDigestResult
{
	lwmDIGEST_Nothing,
	lwmDIGEST_Worked,
	lwmDIGEST_Error,				// Fatal error
	lwmDIGEST_VideoSync,			// Video sync with frame
	lwmDIGEST_VideoSync_Dropped,	// Video sync with timing, but no video frame was produced.
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
	lwmFRAMEFORMAT_8Bit_420P_Planar,
	lwmFRAMEFORMAT_8Bit_3Channel_Interleaved,
	lwmFRAMEFORMAT_8Bit_4Channel_Interleaved,

	lwmFRAMEFORMAT_Count,
};

enum lwmEVideoChannelLayout
{
	lwmVIDEOCHANNELLAYOUT_Unknown,
	lwmVIDEOCHANNELLAYOUT_YCbCr_BT601,	// 16-235 luma, 16-240 chroma
	lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG,	// 0-255 luma, 0-255 chroma
	lwmVIDEOCHANNELLAYOUT_RGB,
	lwmVIDEOCHANNELLAYOUT_BGR,
	lwmVIDEOCHANNELLAYOUT_ARGB,
	lwmVIDEOCHANNELLAYOUT_ABGR,
	lwmVIDEOCHANNELLAYOUT_RGBA,
	lwmVIDEOCHANNELLAYOUT_BGRA,

	lwmVIDEOCHANNELLAYOUT_Count,
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
	lwmSTREAMPARAM_U32_VideoFrameFormat,
	lwmSTREAMPARAM_U32_VideoChannelLayout,
	lwmSTREAMPARAM_U32_LongestFrameReadAhead,
	lwmSTREAMPARAM_U32_NumReadWriteWorkFrames,
	lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames,

	lwmSTREAMPARAM_U32_SampleRate,
	lwmSTREAMPARAM_U32_SpeakerLayout,
	
	lwmSTREAMPARAM_U32_DropAggressiveness,
};

enum lwmEDropAggressiveness
{
	lwmDROPAGGRESSIVENESS_None,			// Don't drop anything
	lwmDROPAGGRESSIVENESS_B,			// Drop B-frames
	lwmDROPAGGRESSIVENESS_BP,			// Drop B-frames and P-frames
	lwmDROPAGGRESSIVENESS_BPI,			// Drop B-frames, P-frames, and I-frames
	lwmDROPAGGRESSIVENESS_All,			// Drop everything
};

enum lwmEUserFlag
{
	lwmUSERFLAG_None				= 0,

	lwmUSERFLAG_ThreadedDeslicer		= (1 << 0),
	lwmUSERFLAG_ThreadedReconstructor	= (1 << 1),
};

enum lwmEPixelConvertFlags
{
	lwmPIXELCONVERTFLAG_Interpolate	=	(1 << 0),
	lwmPIXELCONVERTFLAG_NoCache	=		(1 << 1),
};

struct lwmSVideoFrameProvider
{
	// Creates working frames.  If compiling with SIMD, the work frames must be SIMD-aligned.
	// numRWFrames: Number of frames that require read and write access.
	// numWriteOnlyFrames: Number of frames that require write-only access.
	// frameFormat: Frame format.
	int (*createWorkFramesFunc)(struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 numRWFrames, lwmUInt32 numWriteOnlyFrames, lwmUInt32 workFrameWidth, lwmUInt32 workFrameHeight, lwmUInt32 frameFormat);

	// Locks a working frame for memory access.
	// workFrameIndex: Work frame index.
	// lockType: One of lwmEVideoLockType
	void (*lockWorkFrameFunc)(struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 lockType);

	// Unlocks a working frame.
	void (*unlockWorkFrameFunc)(struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex);

	// Retrieves the pixel data for a work frame plane.
	void *(*getWorkFramePlaneFunc)(struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex, lwmUInt32 *outPitch);

	// Retrieves the width of a work frame plane
	lwmUInt32 (*getWorkFramePlaneWidthFunc)(const struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 planeIndex);

	// Retrieves the height of a work frame plane
	lwmUInt32 (*getWorkFramePlaneHeightFunc)(const struct lwmSVideoFrameProvider *frameProvider, lwmUInt32 planeIndex);

	// Destroys the frame provider
	void (*destroyFunc)(struct lwmSVideoFrameProvider *frameProvider);
};

LWMOVIE_API_LINK void lwmInitialize();
LWMOVIE_API_LINK struct lwmMovieState *lwmCreateMovieState(struct lwmSAllocator *alloc, lwmUInt32 userFlags);
LWMOVIE_API_LINK void lwmMovieState_FeedData(struct lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested);
LWMOVIE_API_LINK lwmUInt8 lwmMovieState_GetAudioStreamCount(const struct lwmMovieState *movieState);
LWMOVIE_API_LINK int lwmMovieState_SetAudioStreamEnabled(struct lwmMovieState *movieState, lwmUInt8 streamIndex, int enable);
LWMOVIE_API_LINK int lwmMovieState_GetCommonParameterU32(const struct lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt32 commonParameterU32, lwmUInt32 *output);
LWMOVIE_API_LINK int lwmMovieState_GetStreamParameterU32(const struct lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, lwmUInt32 streamParameterU32, lwmUInt32 *output);
LWMOVIE_API_LINK int lwmMovieState_GetStreamMetaID(const struct lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, char outMetaID[8]);
LWMOVIE_API_LINK void lwmMovieState_SetStreamParameterU32(struct lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, lwmUInt32 streamParameterU32, lwmUInt32 value);
LWMOVIE_API_LINK void lwmMovieState_SetVideoReconstructor(struct lwmMovieState *movieState, struct lwmIVideoReconstructor *recon);
LWMOVIE_API_LINK void lwmMovieState_VideoDigestParticipate(struct lwmMovieState *movieState);
LWMOVIE_API_LINK void lwmMovieState_SetVideoDigestWorkNotifier(struct lwmMovieState *movieState, struct lwmSWorkNotifier *videoDigestWorkNotifier);
LWMOVIE_API_LINK void lwmMovieState_Destroy(struct lwmMovieState *movieState);

LWMOVIE_API_LINK int lwmMovieState_IsAudioPlaybackSynchronized(struct lwmMovieState *movieState);
LWMOVIE_API_LINK int lwmMovieState_SynchronizeAudioPlayback(struct lwmMovieState *movieState);
LWMOVIE_API_LINK lwmUInt32 lwmMovieState_GetNumAudioSamplesAvailable(struct lwmMovieState *movieState, lwmUInt8 streamIndex);
LWMOVIE_API_LINK lwmUInt32 lwmMovieState_ReadAudioSamples(struct lwmMovieState *movieState, lwmUInt8 streamIndex, void *samples, lwmUInt32 numSamples);
LWMOVIE_API_LINK void lwmMovieState_NotifyAudioPlaybackUnderrun(struct lwmMovieState *movieState);

LWMOVIE_API_LINK void lwmVideoRecon_Participate(struct lwmIVideoReconstructor *videoRecon);
LWMOVIE_API_LINK void lwmVideoRecon_SetWorkNotifier(struct lwmIVideoReconstructor *recon, struct lwmSWorkNotifier *workNotifier);
LWMOVIE_API_LINK lwmUInt32 lwmVideoRecon_GetWorkFrameIndex(const struct lwmIVideoReconstructor *recon);
LWMOVIE_API_LINK void lwmVideoRecon_Destroy(struct lwmIVideoReconstructor *videoRecon);

LWMOVIE_API_LINK struct lwmSVideoFrameProvider *lwmCreateSystemMemoryFrameProvider(struct lwmSAllocator *alloc, const struct lwmMovieState *movieState);
LWMOVIE_API_LINK void lwmSVideoFrameProvider_Destroy(struct lwmSVideoFrameProvider *frameProvider);

LWMOVIE_API_LINK int lwmVideoRecon_ExportRGB(struct lwmSAllocator *alloc, struct lwmIVideoReconstructor *recon, void *outPixels, lwmLargeUInt outStride, lwmUInt32 outWidth, lwmUInt32 outHeight, enum lwmEVideoChannelLayout channelLayout, int pixelConvertFlags);

LWMOVIE_API_LINK struct lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(struct lwmMovieState *movieState, struct lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmUInt32 flags, struct lwmSVideoFrameProvider *frameProvider);
LWMOVIE_API_LINK void lwmIVideoReconstructor_Destroy(struct lwmIVideoReconstructor *videoRecon);

LWMOVIE_API_LINK void lwmFlushProfileTags(struct lwmMovieState *movieState, LWMOVIE_API_CLASS lwmCProfileTagSet *tagSet);

LWMOVIE_API_LINK struct lwmVideoRGBConverter *lwmVideoRGBConverter_CreateSliced(struct lwmSAllocator *alloc, struct lwmIVideoReconstructor *recon, enum lwmEFrameFormat inFrameFormat, enum lwmEVideoChannelLayout inChannelLayout, lwmUInt32 outWidth, lwmUInt32 outHeight, enum lwmEVideoChannelLayout outChannelLayout, lwmLargeUInt numSlices);
LWMOVIE_API_LINK struct lwmVideoRGBConverter *lwmVideoRGBConverter_Create(struct lwmSAllocator *alloc, struct lwmIVideoReconstructor *recon, enum lwmEFrameFormat inFrameFormat, enum lwmEVideoChannelLayout inChannelLayout, lwmUInt32 outWidth, lwmUInt32 outHeight, enum lwmEVideoChannelLayout outChannelLayout);
LWMOVIE_API_LINK void lwmVideoRGBConverter_SetWorkNotifier(struct lwmVideoRGBConverter *converter, struct lwmSWorkNotifier *workNotifier);
LWMOVIE_API_LINK void lwmVideoRGBConverter_Convert(struct lwmVideoRGBConverter *converter, void *outPixels, lwmLargeUInt outStride, int conversionFlags);
LWMOVIE_API_LINK void lwmVideoRGBConverter_Destroy(struct lwmVideoRGBConverter *converter);
LWMOVIE_API_LINK void lwmVideoRGBConverter_ConvertParticipate(struct lwmVideoRGBConverter *converter);

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

LWMOVIE_API_LINK struct lwmSVideoFrameProvider *lwmCreateD3D11FrameProvider(struct lwmSAllocator *alloc, struct ID3D11Device *device, struct ID3D11DeviceContext *context, int isUsingHardwareReconstructor);
LWMOVIE_API_LINK struct ID3D11ShaderResourceView *lwmD3D11FrameProvider_GetWorkFramePlaneSRV(struct lwmSVideoFrameProvider *vfp, lwmUInt32 workFrameIndex, lwmUInt32 planeIndex);

LWMOVIE_API_LINK struct lwmIVideoReconstructor *lwmCreateD3D11VideoReconstructor(struct lwmMovieState *movieState, struct lwmSAllocator *alloc, lwmUInt32 reconstructorType, struct ID3D11Device *device, struct ID3D11DeviceContext *context, struct lwmSVideoFrameProvider *frameProvider);
LWMOVIE_API_LINK void lwmD3D11Reconstructor_ExecuteIDCT(struct lwmIVideoReconstructor *reconstructor);
LWMOVIE_API_LINK void lwmD3D11Reconstructor_ExecuteForFrame(struct lwmIVideoReconstructor *reconstructor);

#endif
