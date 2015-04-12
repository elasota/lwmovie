#ifndef __LWMOVIE_CAKEWALKER_H__
#define __LWMOVIE_CAKEWALKER_H__

#include "lwmovie.h"

struct lwmCakeAudioDevice;

// Simpler interface, a.k.a. "Cake"
enum lwmECakeReconType
{
	lwmCAKE_RECONTYPE_Software,
};

enum lwmECakeFrameProviderType
{
	lwmCAKE_FPTYPE_SystemMemory,
};

enum lwmECakeResult
{
	lwmCAKE_RESULT_Waiting,
	lwmCAKE_RESULT_NewVideoFrame,
	lwmCAKE_RESULT_Finished,
	lwmCAKE_RESULT_Initialized,
	lwmCAKE_RESULT_Error,
};

struct lwmCakeTimeReader
{
	lwmUInt64 (*getTimeMillisecondsFunc)(struct lwmCakeTimeReader *timeReader);
	lwmUInt32 (*getResolutionMillisecondsFunc)(struct lwmCakeTimeReader *timeReader);
};

struct lwmCakeFileReader
{
	int (*isEOFFunc)(struct lwmCakeFileReader *fileReader);
	lwmLargeUInt (*readBytesFunc)(struct lwmCakeFileReader *fileReader, void *dest, lwmLargeUInt numBytes);
};

struct lwmCakeAudioStreamInfo
{
	lwmUInt32 sampleRate;
	enum lwmESpeakerLayout speakerLayout;
	struct lwmCakeAudioDevice *attachedAudioDevice;
};

struct lwmCakeAudioSource
{
	const struct lwmCakeAudioStreamInfo *audioStreamInfo;
};

struct lwmCakeAudioDevice
{
	lwmLargeUInt (*queueSamplesFunc)(struct lwmCakeAudioDevice *audioDevice, LWMOVIE_API_CLASS lwmCake *cake, const struct lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples);
	int (*underrunOccurredFunc)(struct lwmCakeAudioDevice *audioDevice);
	void (*resetFunc)(struct lwmCakeAudioDevice *audioDevice);
};

struct lwmCakeMovieInfo
{
	lwmUInt32 videoWidth;
	lwmUInt32 videoHeight;
	lwmUInt32 fpsNum;
	lwmUInt32 fpsDenom;
	enum lwmEVideoChannelLayout videoChannelLayout;
	enum lwmEFrameFormat videoFrameFormat;

	lwmLargeUInt numAudioStreams;
	const struct lwmCakeAudioStreamInfo *audioStreamInfos;
};

struct lwmCakeCreateOptions
{
	int bUseThreadedDeslicer;
	int bUseThreadedReconstructor;
	struct lwmCakeWorkNotifierFactory *customNotifierFactory;
};

typedef void (*lwmCakeParticipateCallback)(void *obj);

struct lwmCakeWorkNotifierFactory
{
	struct lwmSWorkNotifier *(*createWorkNotifierFunc)(struct lwmCakeWorkNotifierFactory *workNotifierFactory, void *obj, lwmCakeParticipateCallback participationCallback);
	void (*destroyWorkNotifierFunc)(struct lwmCakeWorkNotifierFactory *workNotifierFactory, struct lwmSWorkNotifier *notifier);
};

struct lwmCakeDecodeOptions
{
	struct lwmSVideoFrameProvider *customFrameProvider;
	struct lwmIVideoReconstructor *customReconstructor;
};

struct lwmCakeDecodeOutput
{
	lwmUInt32 displayDelay;
	lwmUInt32 workFrameIndex;
};

LWMOVIE_API_CLASS lwmCake;

LWMOVIE_API_LINK LWMOVIE_API_CLASS lwmCake *lwmCake_Create(struct lwmSAllocator *alloc, struct lwmCakeFileReader *fileReader, struct lwmCakeTimeReader *timeReader, const struct lwmCakeCreateOptions *createOptions);
LWMOVIE_API_LINK enum lwmECakeResult lwmCake_ReadMovieInfo(LWMOVIE_API_CLASS lwmCake *cake, struct lwmCakeMovieInfo *outMovieInfo);
LWMOVIE_API_LINK int lwmCake_BeginDecoding(LWMOVIE_API_CLASS lwmCake *cake, const struct lwmCakeDecodeOptions *decodeOptions);
LWMOVIE_API_LINK void lwmCake_SetStreamAudioDevice(LWMOVIE_API_CLASS lwmCake *cake, lwmUInt8 streamIndex, struct lwmCakeAudioDevice *audioDevice);
LWMOVIE_API_LINK enum lwmECakeResult lwmCake_Decode(LWMOVIE_API_CLASS lwmCake *cake, struct lwmCakeDecodeOutput *decodeOutput);
LWMOVIE_API_LINK struct lwmIVideoReconstructor *lwmCake_GetVideoReconstructor(const LWMOVIE_API_CLASS lwmCake *cake);
LWMOVIE_API_LINK struct lwmSVideoFrameProvider *lwmCake_GetVideoFrameProvider(const LWMOVIE_API_CLASS lwmCake *cake);
LWMOVIE_API_LINK void lwmCake_ReadAudioSamples(LWMOVIE_API_CLASS lwmCake *cake, const struct lwmCakeAudioSource *audioSource, void *samples, lwmUInt32 numSamples);
LWMOVIE_API_LINK void lwmCake_Destroy(LWMOVIE_API_CLASS lwmCake *cake);

#endif
