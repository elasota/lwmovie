#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2-2.0.3/include/SDL.h"
#include "threadpool.hpp"
#include "../lwmovie/lwmovie.h"
#include "../lwmovie/lwmovie_cake.h"
#include "../lwmovie/lwmovie_cake_cppshims.hpp"

namespace lwplay
{
	class CAllocator : public lwmSAllocator
	{
	public:
		CAllocator();
	private:
		static void *StaticAlloc(lwmSAllocator *alloc, lwmLargeUInt sz);
		static void StaticFree(lwmSAllocator *alloc, void *ptr);
	};

	class CAudioQueue : public lwmICakeAudioDevice
	{
	public:
		CAudioQueue(const lwmCakeAudioStreamInfo *streamInfo, lwmUInt32 numSamples);
		~CAudioQueue();
		lwmUInt32 NumBytesQueued() const;
		lwmUInt32 NumBytesAvailable() const;
		bool UnderrunOccurred() const;
		bool InitedOK() const;
		void Reset();
		lwmUInt32 Capacity() const;
		void *GetQueuePoint();
		void CommitQueuedData(lwmUInt32 numBytes);
		lwmLargeUInt QueueSamples(lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples);

		static void SDLCALL StaticPullFromSDL(void *userdata, Uint8 * stream, int len);

	private:
		void PullFromSDL(lwmUInt8 *targetBytes, lwmUInt32 len);

		bool m_underrunOccurred;
		bool m_initedOK;
		bool m_isPaused;
		SDL_AudioDeviceID m_audioDeviceID;
		lwmUInt8 m_sampleSizeBytes;
		lwmUInt32 m_numBytesQueued;
		lwmUInt32 m_capacity;
		lwmUInt8 *m_sampleBytes;
	};
	
	class CFileReader : public lwmCakeFileReader
	{
	private:
		FILE *m_f;

		static int StaticIsEOF(lwmCakeFileReader *self);
		static lwmLargeUInt StaticReadBytes(lwmCakeFileReader *self, void *dest, lwmLargeUInt numBytes);

	public:
		explicit CFileReader(FILE *f);
	};

	class CTimer : public lwmCakeTimeReader
	{
	private:
		static lwmUInt64 StaticGetTime(lwmCakeTimeReader *timeReader);
		static lwmUInt32 StaticGetResolution(lwmCakeTimeReader *timeReader);
	public:
		CTimer();
	};
}

lwplay::CAllocator::CAllocator()
{
	this->allocFunc = StaticAlloc;
	this->freeFunc = StaticFree;
}

void *lwplay::CAllocator::StaticAlloc(lwmSAllocator *alloc, lwmLargeUInt sz)
{
	return _aligned_malloc(sz, 16);
}

void lwplay::CAllocator::StaticFree(lwmSAllocator *alloc, void *ptr)
{
	_aligned_free(ptr);
}

lwplay::CAudioQueue::CAudioQueue(const lwmCakeAudioStreamInfo *streamInfo, lwmUInt32 numSamples)
	: m_numBytesQueued(0)
	, m_underrunOccurred(false)
	, m_initedOK(false)
	, m_isPaused(true)
	, m_audioDeviceID(0)
	, m_sampleSizeBytes(0)
{

	int neededChannels = 0;

	if(streamInfo->speakerLayout == lwmSPEAKERLAYOUT_Mono)
	{
		m_sampleSizeBytes = 2;
		neededChannels = 1;
	}
	else if(streamInfo->speakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
	{
		m_sampleSizeBytes = 4;
		neededChannels = 2;
	}
	else
		return;

	m_capacity = m_sampleSizeBytes * numSamples;
	m_sampleBytes = new lwmUInt8[m_capacity];
	
	SDL_AudioSpec wantAudio, haveAudio;

	SDL_zero(wantAudio);
	wantAudio.freq = static_cast<int>(streamInfo->sampleRate);
	wantAudio.channels = static_cast<int>(neededChannels);
	wantAudio.format = AUDIO_S16SYS;
	wantAudio.samples = 512;
	wantAudio.userdata = this;
	wantAudio.callback = StaticPullFromSDL;
	m_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wantAudio, &haveAudio, 0);
	if(m_audioDeviceID == 0)
		return;

	if(haveAudio.format != wantAudio.format ||
		haveAudio.channels != wantAudio.channels ||
		haveAudio.freq != wantAudio.freq)
	{
		SDL_CloseAudioDevice(m_audioDeviceID);
		m_audioDeviceID = 0;
	}

	m_initedOK = true;
}

lwplay::CAudioQueue::~CAudioQueue()
{
	if(m_audioDeviceID != 0)
		SDL_CloseAudioDevice(m_audioDeviceID);
	delete[] m_sampleBytes;
}

lwmUInt32 lwplay::CAudioQueue::NumBytesQueued() const
{
	return m_numBytesQueued;
}

lwmUInt32 lwplay::CAudioQueue::NumBytesAvailable() const
{
	return m_capacity - m_numBytesQueued;
}

bool lwplay::CAudioQueue::UnderrunOccurred() const
{
	return m_underrunOccurred;
}

bool lwplay::CAudioQueue::InitedOK() const
{
	return m_initedOK;
}

lwmUInt32 lwplay::CAudioQueue::Capacity() const
{
	return m_capacity;
}

void *lwplay::CAudioQueue::GetQueuePoint()
{
	return m_sampleBytes + m_numBytesQueued;
}

void lwplay::CAudioQueue::CommitQueuedData(lwmUInt32 numBytes)
{
	m_numBytesQueued += numBytes;
}

void lwplay::CAudioQueue::Reset()
{
	m_isPaused = true;
	SDL_PauseAudioDevice(m_audioDeviceID, 1);
	SDL_LockAudioDevice(m_audioDeviceID);
	m_numBytesQueued = 0;
	m_underrunOccurred = false;
	SDL_UnlockAudioDevice(m_audioDeviceID);
}


lwmLargeUInt lwplay::CAudioQueue::QueueSamples(lwmCake *cake, const lwmCakeAudioSource *sources, lwmLargeUInt numSources, lwmLargeUInt numSamples)
{
	SDL_LockAudioDevice(m_audioDeviceID);
	void *dest = m_sampleBytes + m_numBytesQueued;
	lwmLargeUInt numBytesAvailable = m_capacity - m_numBytesQueued;
	lwmLargeUInt numSamplesAvailable = numBytesAvailable / m_sampleSizeBytes;
	if(numSamplesAvailable > numSamples)
		numSamplesAvailable = numSamples;
	numBytesAvailable = numSamplesAvailable * m_sampleSizeBytes;

	lwmCake_ReadAudioSamples(cake, sources + 0, dest, numSamplesAvailable);

	m_numBytesQueued += numBytesAvailable;
	SDL_UnlockAudioDevice(m_audioDeviceID);
	
	if(m_isPaused)
	{
		m_isPaused = false;
		SDL_PauseAudioDevice(m_audioDeviceID, 0);
	}

	return numSamplesAvailable;
}

void SDLCALL lwplay::CAudioQueue::StaticPullFromSDL(void *userdata, Uint8 * stream, int len)
{
	static_cast<CAudioQueue*>(userdata)->PullFromSDL(stream, static_cast<lwmUInt32>(len));
}

void lwplay::CAudioQueue::PullFromSDL(lwmUInt8 *dest, lwmUInt32 len)
{
	if(len > m_numBytesQueued)
	{
		m_underrunOccurred = true;
		memcpy(dest, m_sampleBytes, m_numBytesQueued);
		memset(dest + m_numBytesQueued, 0, len - m_numBytesQueued);
		m_numBytesQueued = 0;
	}
	else
	{
		memcpy(dest, m_sampleBytes, len);
		memmove(m_sampleBytes, m_sampleBytes + len, m_numBytesQueued - len);
		m_numBytesQueued -= len;
	}
}

int lwplay::CFileReader::StaticIsEOF(lwmCakeFileReader *self)
{
	return feof(static_cast<lwplay::CFileReader*>(self)->m_f);
}
	
lwmLargeUInt lwplay::CFileReader::StaticReadBytes(lwmCakeFileReader *self, void *dest, lwmLargeUInt numBytes)
{
	return fread(dest, 1, numBytes, static_cast<lwplay::CFileReader*>(self)->m_f);
}

lwplay::CFileReader::CFileReader(FILE *f)
	: m_f(f)
{
	this->isEOFFunc = StaticIsEOF;
	this->readBytesFunc = StaticReadBytes;
}

lwmUInt64 lwplay::CTimer::StaticGetTime(lwmCakeTimeReader *timeReader)
{
	return SDL_GetTicks();
}

lwmUInt32 lwplay::CTimer::StaticGetResolution(lwmCakeTimeReader *timeReader)
{
	return 10;
}

lwplay::CTimer::CTimer()
{
	this->getTimeMillisecondsFunc = StaticGetTime;
	this->getResolutionMillisecondsFunc = StaticGetResolution;
}


int main(int argc, char **argv)
{
	if(argc != 2)
	{
		return -1;
	}

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		fprintf(stderr, "Failed to init SDL");
		return -1;
	}

	lwmInitialize();

	lwplay::CAllocator myAllocator;

	char dataBuffer[4000];
	char *unreadDataStart = NULL;
	lwmLargeUInt numBytesAvailable = 0;
	bool eof = false;
	FILE *f2 = fopen(argv[1], "rb");

	if(!f2)
		return -1;

	lwplay::CFileReader myFileReader(f2);
	lwplay::CTimer myTimer;

	lwmCake *cake;

	lwmVideoRGBConverter *rgbConverter = NULL;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	bool textureIsYUV = false;
	lwplay::CAudioQueue *audioDevice = NULL;
	lwmCakeMovieInfo movieInfo;
	lwplay::CWorkNotifierFactory *workNotifierFactory = new lwplay::CWorkNotifierFactory();

	
	{
		lwmCakeCreateOptions cakeOpts;
		memset(&cakeOpts, 0, sizeof(cakeOpts));
		cakeOpts.bUseThreadedDeslicer = 1;
		cakeOpts.bUseThreadedReconstructor = 1;
		cakeOpts.customNotifierFactory = workNotifierFactory;
		cake = lwmCake_Create(&myAllocator, &myFileReader, &myTimer, &cakeOpts);
	}

	{
		while(true)
		{
			lwmECakeResult cakeResult = lwmCake_ReadMovieInfo(cake, &movieInfo);
			
			if(cakeResult == lwmCAKE_RESULT_Initialized)
				break;
			if(cakeResult == lwmCAKE_RESULT_Waiting)
				continue;
			// Anything else is an error
			return -1;
		}

		window = SDL_CreateWindow("lwplay", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(movieInfo.videoWidth), static_cast<int>(movieInfo.videoHeight), 0);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		if(movieInfo.videoChannelLayout == lwmVIDEOCHANNELLAYOUT_YCbCr_BT601 && movieInfo.videoFrameFormat == lwmFRAMEFORMAT_8Bit_420P_Planar)
		{
			textureIsYUV = true;
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(movieInfo.videoWidth), static_cast<int>(movieInfo.videoHeight));
		}
		else
		{
			// TODO: Cake
			lwmIVideoReconstructor *recon = lwmCake_GetVideoReconstructor(cake);
			rgbConverter = lwmVideoRGBConverter_Create(&myAllocator, recon, movieInfo.videoFrameFormat, movieInfo.videoChannelLayout, movieInfo.videoWidth, movieInfo.videoHeight, lwmVIDEOCHANNELLAYOUT_RGBA);

			int sdlPixelFormat;
			if(SDL_PIXELTYPE(SDL_PIXELFORMAT_RGBA8888) == SDL_PIXELTYPE_PACKED32 && SDL_BYTEORDER == SDL_LIL_ENDIAN)
				sdlPixelFormat = SDL_PIXELFORMAT_ABGR8888;
			else
				sdlPixelFormat = SDL_PIXELFORMAT_RGBA8888;

			texture = SDL_CreateTexture(renderer, sdlPixelFormat, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(movieInfo.videoWidth), static_cast<int>(movieInfo.videoHeight));
			textureIsYUV = false;
		}

		if(movieInfo.numAudioStreams > 0)
		{
			lwmUInt8 sampleSizeBytes = 1;
			audioDevice = new lwplay::CAudioQueue(movieInfo.audioStreamInfos + 0, 8192);
			if(!audioDevice->InitedOK())
			{
				delete audioDevice;
				audioDevice = NULL;
			}
		}
	}

	{
		lwmCakeDecodeOptions decodeOptions;
		memset(&decodeOptions, 0, sizeof(decodeOptions));
		if(!lwmCake_BeginDecoding(cake, &decodeOptions))
			return -1;
	}
	
	if(movieInfo.numAudioStreams > 0 && audioDevice)
		lwmCake_SetStreamAudioDevice(cake, 0, audioDevice);

	while(true)
	{
		lwmCakeDecodeOutput decodeOutput;
		lwmECakeResult result = lwmCake_Decode(cake, &decodeOutput);

		switch(result)
		{
		case lwmCAKE_RESULT_Waiting:
			if(decodeOutput.displayDelay)
				SDL_Delay(decodeOutput.displayDelay);
			break;
		case lwmCAKE_RESULT_NewVideoFrame:
			{
				const lwmUInt8 *yBuffer;
				const lwmUInt8 *uBuffer;
				const lwmUInt8 *vBuffer;
				lwmUInt32 yStride, uStride, vStride;

				lwmSVideoFrameProvider *frameProvider = lwmCake_GetVideoFrameProvider(cake);
				lwmUInt32 frameIndex = decodeOutput.workFrameIndex;

				frameProvider->lockWorkFrameFunc(frameProvider, frameIndex, lwmVIDEOLOCK_Read);

				yBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 0, &yStride));
				uBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 1, &uStride));
				vBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 2, &vStride));

				SDL_Rect rect;
				rect.x = rect.y = 0;
				rect.w = static_cast<int>(movieInfo.videoWidth);
				rect.h = static_cast<int>(movieInfo.videoHeight);

				if(textureIsYUV)
					SDL_UpdateYUVTexture(texture, &rect, yBuffer, static_cast<int>(yStride), uBuffer, static_cast<int>(uStride), vBuffer, static_cast<int>(vStride));
				else
				{
					void *pixels;
					int pitch;
					SDL_LockTexture(texture, &rect, &pixels, &pitch);
					lwmVideoRGBConverter_Convert(rgbConverter, pixels, static_cast<lwmLargeUInt>(pitch), 0);
					SDL_UnlockTexture(texture);
				}
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);

				frameProvider->unlockWorkFrameFunc(frameProvider, frameIndex);
			}
			break;
		case lwmCAKE_RESULT_Finished:
		case lwmCAKE_RESULT_Error:
			goto exitMovieLoop;
			break;
		default:
			break;
		};

		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
				goto exitMovieLoop;
		}
	}

exitMovieLoop:
	lwmCake_Destroy(cake);

	fclose(f2);

	if(texture)
		SDL_DestroyTexture(texture);
	if(renderer)
		SDL_DestroyRenderer(renderer);
	if(audioDevice)
		delete audioDevice;
	SDL_Quit();

	return 0;
}
