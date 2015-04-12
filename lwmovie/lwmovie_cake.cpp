#include <new>
#include <stdlib.h>
#include "lwmovie_cake.h"

class lwmCake
{
public:
	lwmCake(lwmSAllocator *alloc, lwmCakeFileReader *fileReader, lwmCakeTimeReader *timeReader);
	~lwmCake();
	lwmECakeResult lwmCakeReadMovieInfo(lwmCakeMovieInfo *outMovieInfo);
	bool Init(const lwmCakeCreateOptions *createOptions);
	bool BeginDecoding(const lwmCakeDecodeOptions *decodeOptions);
	lwmECakeResult Decode(lwmCakeDecodeOutput *decodeOutput);
	lwmECakeResult ReadMovieInfo(lwmCakeMovieInfo *outMovieInfo);
	lwmIVideoReconstructor *GetVideoReconstructor() const;
	lwmSVideoFrameProvider *GetVideoFrameProvider() const;
	void SetStreamAudioDevice(lwmUInt8 streamIndex, lwmCakeAudioDevice *audioDevice);
	void ReadAudioSamples(const lwmCakeAudioSource *audioSource, void *samples, lwmUInt32 numSamples);
	void Destroy();

private:
	void FillBuffer();
	void HandleVideoSync(lwmCakeDecodeOutput *decodeOutput, bool isDropped);

	static const lwmLargeUInt BUFFER_SIZE = 4096;

	lwmCakeAudioStreamInfo *m_audioStreamInfos;
	lwmCakeAudioSource *m_audioSources;

	lwmSAllocator *m_alloc;
	lwmCakeFileReader *m_fileReader;
	lwmCakeTimeReader *m_timeReader;
	lwmUInt64 m_timeBase;
	lwmMovieState *m_movieState;
	lwmSVideoFrameProvider *m_frameProvider;
	lwmIVideoReconstructor *m_reconstructor;
	lwmCakeWorkNotifierFactory *m_notifierFactory;

	lwmSWorkNotifier *m_digestNotifier;
	lwmSWorkNotifier *m_reconNotifier;

	lwmUInt32 m_currentVideoTimestamp;
	lwmUInt32 m_playbackStartTime;

	lwmUInt32 m_fpsNum;
	lwmUInt32 m_fpsDenom;
	lwmUInt32 m_frameDisplayTime;

	lwmUInt32 m_timeResolution;

	bool m_threadedRecon;
	bool m_threadedDigest;
	bool m_fatalError;
	bool m_firstFrame;
	bool m_waitingForDisplay;
	bool m_waitingFrameIsDropped;

	lwmUInt8 m_byteBuffer[BUFFER_SIZE];
	lwmUInt32 m_bufferStart;
	lwmUInt32 m_bufferUsed;
	lwmUInt32 m_userFlags;
	bool m_eof;
};

lwmCake::lwmCake(lwmSAllocator *alloc, lwmCakeFileReader *fileReader, lwmCakeTimeReader *timeReader)
	: m_alloc(alloc)
	, m_fileReader(fileReader)
	, m_timeReader(timeReader)
	, m_movieState(NULL)
	, m_timeBase(timeReader->getTimeMillisecondsFunc(timeReader))
	, m_bufferStart(0)
	, m_bufferUsed(0)
	, m_eof(false)
	, m_frameProvider(NULL)
	, m_reconstructor(NULL)
	, m_notifierFactory(NULL)
	, m_digestNotifier(NULL)
	, m_reconNotifier(NULL)
	, m_audioStreamInfos(NULL)
	, m_audioSources(NULL)
	, m_threadedRecon(false)
	, m_threadedDigest(false)
	, m_fatalError(false)
	, m_firstFrame(true)
	, m_waitingForDisplay(false)
	, m_waitingFrameIsDropped(false)
	, m_currentVideoTimestamp(0)
	, m_timeResolution(0)
	, m_userFlags(0)
{
}

lwmCake::~lwmCake()
{
	if(m_movieState)
		lwmMovieState_Destroy(m_movieState);
	if(m_reconstructor)
		lwmIVideoReconstructor_Destroy(m_reconstructor);
	if(m_frameProvider)
		lwmSVideoFrameProvider_Destroy(m_frameProvider);

	if(m_digestNotifier)
		m_notifierFactory->destroyWorkNotifierFunc(m_notifierFactory, m_digestNotifier);
	if(m_reconNotifier)
		m_notifierFactory->destroyWorkNotifierFunc(m_notifierFactory, m_reconNotifier);

	if(m_audioStreamInfos)
		m_alloc->Free(m_audioStreamInfos);
}

bool lwmCake::Init(const lwmCakeCreateOptions *createOptions)
{
	m_userFlags = 0;
	if(createOptions->bUseThreadedDeslicer)
	{
		m_threadedDigest = true;
		m_userFlags |= lwmUSERFLAG_ThreadedDeslicer;
	}
	if(createOptions->bUseThreadedReconstructor)
	{
		m_threadedRecon = true;
		m_userFlags |= lwmUSERFLAG_ThreadedReconstructor;
	}

	m_movieState = lwmCreateMovieState(m_alloc, m_userFlags);
	if(!m_movieState)
		return false;

	m_timeResolution = m_timeReader->getResolutionMillisecondsFunc(m_timeReader);
	m_notifierFactory = createOptions->customNotifierFactory;

	if(m_notifierFactory && m_threadedDigest)
	{
		struct Shim
		{
			static void Participate(void *obj) { lwmMovieState_VideoDigestParticipate(static_cast<lwmMovieState*>(obj)); }
		};

		m_digestNotifier = m_notifierFactory->createWorkNotifierFunc(m_notifierFactory, m_movieState, Shim::Participate);
		if(!m_digestNotifier)
			return false;
		lwmMovieState_SetVideoDigestWorkNotifier(m_movieState, m_digestNotifier);
	}

	return true;
}

bool lwmCake::BeginDecoding(const lwmCakeDecodeOptions *decodeOptions)
{
	lwmSVideoFrameProvider *fp = decodeOptions->customFrameProvider;
	if(fp == NULL)
		fp = lwmCreateSystemMemoryFrameProvider(m_alloc, m_movieState);
	if(fp == NULL)
		return false;
	m_frameProvider = fp;

	lwmIVideoReconstructor *recon = decodeOptions->customReconstructor;
	if(recon == NULL)
	{
		lwmUInt32 reconType;
		lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_ReconType, &reconType);

		recon = lwmCreateSoftwareVideoReconstructor(m_movieState, m_alloc, reconType, m_userFlags, m_frameProvider);
		if(recon == NULL)
			return false;
	}
	m_reconstructor = recon;

	lwmMovieState_SetVideoReconstructor(m_movieState, m_reconstructor);

	if(m_notifierFactory)
	{
		if(m_threadedRecon)
		{
			struct Shim
			{
				static void Participate(void *obj) { lwmVideoRecon_Participate(static_cast<lwmIVideoReconstructor*>(obj)); }
			};

			m_reconNotifier = m_notifierFactory->createWorkNotifierFunc(m_notifierFactory, m_reconstructor, Shim::Participate);

			if(!m_reconNotifier)
				return false;
			lwmVideoRecon_SetWorkNotifier(m_reconstructor, m_reconNotifier);
		}
	}

	return true;
}


lwmECakeResult lwmCake::ReadMovieInfo(lwmCakeMovieInfo *outMovieInfo)
{
	bool waitingForInit = true;
	while(waitingForInit)
	{
		FillBuffer();

		lwmUInt32 numBytesDigested, digestResult;
		lwmMovieState_FeedData(m_movieState, m_byteBuffer + m_bufferStart, m_bufferUsed, &digestResult, &numBytesDigested);
		m_bufferStart += numBytesDigested;
		m_bufferUsed -= numBytesDigested;

		switch(digestResult)
		{
		case lwmDIGEST_Nothing:
			if(m_eof)
				return lwmCAKE_RESULT_Error;
			else
				return lwmCAKE_RESULT_Waiting;
			break;
		case lwmDIGEST_Worked:
			break;
		case lwmDIGEST_Error:
			return lwmCAKE_RESULT_Error;
		case lwmDIGEST_Initialize:
			waitingForInit = false;
			break;
		}
	}

	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &outMovieInfo->videoWidth);
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &outMovieInfo->videoHeight);
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_PPSNumerator, &m_fpsNum);
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_PPSDenominator, &m_fpsDenom);

	outMovieInfo->fpsNum = m_fpsNum;
	outMovieInfo->fpsDenom = m_fpsDenom;

	lwmUInt32 rawValue;

	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_VideoFrameFormat, &rawValue);
	outMovieInfo->videoFrameFormat = static_cast<lwmEFrameFormat>(rawValue);
	lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_VideoChannelLayout, &rawValue);
	outMovieInfo->videoChannelLayout = static_cast<lwmEVideoChannelLayout>(rawValue);

	lwmUInt8 streamCount = lwmMovieState_GetAudioStreamCount(m_movieState);
	if(streamCount > 0)
	{
		m_audioStreamInfos = m_alloc->NAlloc<lwmCakeAudioStreamInfo>(streamCount);
		if(!m_audioStreamInfos)
			return lwmCAKE_RESULT_Error;

		m_audioSources = m_alloc->NAlloc<lwmCakeAudioSource>(streamCount);
		if(!m_audioSources)
			return lwmCAKE_RESULT_Error;

		for(lwmUInt8 i=0;i<streamCount;i++)
		{
			lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Audio, i, lwmSTREAMPARAM_U32_SampleRate, &m_audioStreamInfos[i].sampleRate);
			lwmUInt32 value;
			lwmMovieState_GetStreamParameterU32(m_movieState, lwmSTREAMTYPE_Audio, i, lwmSTREAMPARAM_U32_SpeakerLayout, &value);
			m_audioStreamInfos[i].speakerLayout = static_cast<lwmESpeakerLayout>(value);
			m_audioStreamInfos[i].attachedAudioDevice = NULL;
			m_audioSources[i].audioStreamInfo = m_audioStreamInfos + i;
		}
	}

	outMovieInfo->numAudioStreams = streamCount;
	outMovieInfo->audioStreamInfos = m_audioStreamInfos;

	return lwmCAKE_RESULT_Initialized;
}

void lwmCake::FillBuffer()
{
	if(m_eof)
		return;
	if(m_bufferUsed == 0)
		m_bufferStart = 0;
	lwmUInt32 availSpace = BUFFER_SIZE - (m_bufferStart + m_bufferUsed);
	if(availSpace)
	{
		lwmUInt32 numRead = static_cast<lwmUInt32>(m_fileReader->readBytesFunc(m_fileReader, m_byteBuffer + m_bufferStart, availSpace));
		m_bufferUsed += numRead;
		m_eof = (m_fileReader->isEOFFunc(m_fileReader) != 0);
	}
}

lwmECakeResult lwmCake::Decode(lwmCakeDecodeOutput *decodeOutput)
{
	decodeOutput->displayDelay = 0;

	if(m_fatalError)
		return lwmCAKE_RESULT_Error;

	if(m_waitingForDisplay == true)
	{
		lwmUInt32 currentTime = static_cast<lwmUInt32>(m_timeReader->getTimeMillisecondsFunc(m_timeReader) - m_timeBase);
		if(currentTime < m_frameDisplayTime)
		{
			lwmUInt32 diff = m_frameDisplayTime - currentTime;
			if(diff > m_timeResolution)
			{
				decodeOutput->displayDelay = diff;
				return lwmCAKE_RESULT_Waiting;
			}
		}
		m_waitingForDisplay = false;
		this->HandleVideoSync(decodeOutput, m_waitingFrameIsDropped);
		return m_waitingFrameIsDropped ? lwmCAKE_RESULT_Waiting : lwmCAKE_RESULT_NewVideoFrame;
	}

	while(true)
	{
		if(m_bufferUsed != BUFFER_SIZE && !m_eof)
			FillBuffer();

		// TODO: Fail if consumed data overruns the frame size limit

		lwmUInt32 numBytesDigested;
		lwmUInt32 digestResult;
		lwmMovieState_FeedData(m_movieState, m_byteBuffer + m_bufferStart, m_bufferUsed, &digestResult, &numBytesDigested);
		m_bufferStart += numBytesDigested;
		m_bufferUsed -= numBytesDigested;
		
		switch(digestResult)
		{
		case lwmDIGEST_Initialize:
			// BeginDecoding wasn't called
			m_fatalError = true;
			return lwmCAKE_RESULT_Error;
		case lwmDIGEST_VideoSync_Dropped:
		case lwmDIGEST_VideoSync:
			{
				// TODO: Read the real timestamp, handle time going backwards
				m_currentVideoTimestamp++;

				lwmECakeResult cakeResult = lwmCAKE_RESULT_Waiting;
				lwmUInt32 currentTime = static_cast<lwmUInt32>(m_timeReader->getTimeMillisecondsFunc(m_timeReader) - m_timeBase);
				if(m_firstFrame)
				{
					m_playbackStartTime = currentTime;
					m_firstFrame = false;
				}
				else
				{
					lwmUInt32 thisFrameTime = m_playbackStartTime + static_cast<lwmUInt32>(static_cast<lwmUInt64>(m_currentVideoTimestamp) * 1000 * m_fpsDenom / m_fpsNum);
					if(currentTime < thisFrameTime)
					{
						m_waitingForDisplay = true;
						m_waitingFrameIsDropped = (digestResult == lwmDIGEST_VideoSync_Dropped);
						m_frameDisplayTime = thisFrameTime;
						decodeOutput->displayDelay = (thisFrameTime - currentTime);
						return lwmCAKE_RESULT_Waiting;
					}
					else
					{
						lwmUInt32 slip = currentTime - thisFrameTime;
						if(slip > 1000)
							m_playbackStartTime += slip;

						if(digestResult == lwmDIGEST_VideoSync_Dropped)
						{
							HandleVideoSync(decodeOutput, true);
							return lwmCAKE_RESULT_Waiting;
						}
						else
						{
							HandleVideoSync(decodeOutput, false);
							return lwmCAKE_RESULT_NewVideoFrame;
						}
					}
				}
			}
			break;
		};
	}
}

void lwmCake::HandleVideoSync(lwmCakeDecodeOutput *decodeOutput, bool isDropped)
{
	if(!isDropped)
	{
		decodeOutput->workFrameIndex = lwmVideoRecon_GetWorkFrameIndex(this->m_reconstructor);
	}

	lwmUInt8 streamCount = lwmMovieState_GetAudioStreamCount(m_movieState);

	for(lwmUInt8 firstSource=0;firstSource<streamCount;firstSource++)
	{
		// No more streams?
		lwmCakeAudioDevice *device = m_audioSources[firstSource].audioStreamInfo->attachedAudioDevice;
		if(device == NULL)
			break;

		lwmUInt32 requiredSampleRate = m_audioSources[firstSource].audioStreamInfo->sampleRate;

		lwmUInt32 minSamplesAvailable = lwmMovieState_GetNumAudioSamplesAvailable(m_movieState, firstSource);
		lwmUInt8 lastSource = firstSource;
		for(lwmUInt8 scanSource=firstSource+1;scanSource!=streamCount;scanSource++)
		{
			if(m_audioSources[scanSource].audioStreamInfo->attachedAudioDevice != device)
				break;
			lastSource = scanSource;
			
			lwmUInt32 numSamples = lwmMovieState_GetNumAudioSamplesAvailable(m_movieState, scanSource);
			lwmUInt32 sampleRate = m_audioSources[scanSource].audioStreamInfo->sampleRate;
			if(numSamples < minSamplesAvailable)
				minSamplesAvailable = 0;
			if(sampleRate != requiredSampleRate)
			{
				minSamplesAvailable = 0;	// Incompatible audio streams
				break;
			}
		}

		bool canQueueSamples = false;
		if(lwmMovieState_IsAudioPlaybackSynchronized(m_movieState))
		{
			if(device->underrunOccurredFunc(device))
			{
				// Underrun occurred
				lwmMovieState_NotifyAudioPlaybackUnderrun(m_movieState);
				device->resetFunc(device);
			}
			else
			{
				// Otherwise, queue more audio samples
				canQueueSamples = true;
			}
		}
		else
		{
			// Wait for any leftover playback to finish
			device->resetFunc(device);
			if(lwmMovieState_SynchronizeAudioPlayback(m_movieState) != 0)
				canQueueSamples = true;
		}

		if(minSamplesAvailable != 0 && canQueueSamples)
		{
			if(canQueueSamples)
			{
				lwmLargeUInt numSources = lastSource - firstSource + 1;
				device->queueSamplesFunc(device, this, m_audioSources + firstSource, numSources, minSamplesAvailable);
			}
		}
	}
}

lwmIVideoReconstructor *lwmCake::GetVideoReconstructor() const
{
	return m_reconstructor;
}

lwmSVideoFrameProvider *lwmCake::GetVideoFrameProvider() const
{
	return this->m_frameProvider;
}

void lwmCake::SetStreamAudioDevice(lwmUInt8 streamIndex, lwmCakeAudioDevice *audioDevice)
{
	class AudioSourceSorter
	{
	public:
		static int Sort(const void *pa, const void *pb)
		{
			// This just needs to be stable.  Groups audio devices and speaker layouts.
			const lwmCakeAudioStreamInfo *a = static_cast<const lwmCakeAudioSource*>(pa)->audioStreamInfo;
			const lwmCakeAudioStreamInfo *b = static_cast<const lwmCakeAudioSource*>(pb)->audioStreamInfo;

			if(a->attachedAudioDevice != NULL && b->attachedAudioDevice == NULL) return -1;
			if(a->attachedAudioDevice == NULL && b->attachedAudioDevice != NULL) return 1;
			if(a->attachedAudioDevice < b->attachedAudioDevice) return -1;
			if(a->attachedAudioDevice > b->attachedAudioDevice) return 1;
			if(a->speakerLayout < b->speakerLayout) return -1;
			if(a->speakerLayout > b->speakerLayout) return 1;
			if(a < b) return -1;
			if(a > b) return 1;
			return 0;
		}
	};

	lwmCakeAudioStreamInfo *stream = m_audioStreamInfos + streamIndex;
	if(stream->attachedAudioDevice != audioDevice)
		lwmMovieState_SetAudioStreamEnabled(m_movieState, streamIndex, 0);
	stream->attachedAudioDevice = audioDevice;
	if(stream->attachedAudioDevice != NULL)
		lwmMovieState_SetAudioStreamEnabled(m_movieState, streamIndex, 1);
	
	lwmUInt8 streamCount = lwmMovieState_GetAudioStreamCount(m_movieState);
	for(lwmUInt8 i=0;i<streamCount;i++)
		this->m_audioSources[i].audioStreamInfo = this->m_audioStreamInfos + i;
	qsort(m_audioSources, streamCount, sizeof(m_audioSources[0]), AudioSourceSorter::Sort);
}

void lwmCake::ReadAudioSamples(const lwmCakeAudioSource *audioSource, void *samples, lwmUInt32 numSamples)
{
	// Ignore return value, Cake tells the audio device the sample limit
	lwmMovieState_ReadAudioSamples(m_movieState, static_cast<lwmUInt8>(audioSource->audioStreamInfo - m_audioStreamInfos), samples, numSamples);
}

void lwmCake::Destroy()
{
	lwmSAllocator *alloc = m_alloc;
	this->~lwmCake();
	alloc->Free(this);
}


//////////////////////////////////////////////////////////////////////////////////
// C API
LWMOVIE_API_LINK lwmCake *lwmCake_Create(lwmSAllocator *alloc, lwmCakeFileReader *fileReader, lwmCakeTimeReader *timeReader, const lwmCakeCreateOptions *createOptions)
{
	lwmCake *cake = alloc->NAlloc<lwmCake>(1);
	if(cake == NULL)
		return NULL;
	new (cake) lwmCake(alloc, fileReader, timeReader);

	if(!cake->Init(createOptions))
	{
		cake->~lwmCake();
		alloc->Free(cake);
		return NULL;
	}

	return cake;
}

LWMOVIE_API_LINK lwmECakeResult lwmCake_ReadMovieInfo(lwmCake *cake, lwmCakeMovieInfo *outMovieInfo)
{
	return cake->ReadMovieInfo(outMovieInfo);
}

LWMOVIE_API_LINK int lwmCake_BeginDecoding(LWMOVIE_API_CLASS lwmCake *cake, const lwmCakeDecodeOptions *decodeOptions)
{
	return cake->BeginDecoding(decodeOptions) ? 1 : 0;
}

LWMOVIE_API_LINK lwmECakeResult lwmCake_Decode(LWMOVIE_API_CLASS lwmCake *cake, lwmCakeDecodeOutput *decodeOutput)
{
	return cake->Decode(decodeOutput);
}

LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCake_GetVideoReconstructor(const LWMOVIE_API_CLASS lwmCake *cake)
{
	return cake->GetVideoReconstructor();
}

LWMOVIE_API_LINK lwmSVideoFrameProvider *lwmCake_GetVideoFrameProvider(const LWMOVIE_API_CLASS lwmCake *cake)
{
	return cake->GetVideoFrameProvider();
}

LWMOVIE_API_LINK void lwmCake_SetStreamAudioDevice(lwmCake *cake, lwmUInt8 streamIndex, lwmCakeAudioDevice *audioDevice)
{
	cake->SetStreamAudioDevice(streamIndex, audioDevice);
}

LWMOVIE_API_LINK void lwmCake_ReadAudioSamples(LWMOVIE_API_CLASS lwmCake *cake, const struct lwmCakeAudioSource *audioSource, void *samples, lwmUInt32 numSamples)
{
	cake->ReadAudioSamples(audioSource, samples, numSamples);
}

LWMOVIE_API_LINK void lwmCake_Destroy(LWMOVIE_API_CLASS lwmCake *cake)
{
	cake->Destroy();
}
