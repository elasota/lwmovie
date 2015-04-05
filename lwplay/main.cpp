#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2-2.0.3/include/SDL.h"
#include "../lwmovie/lwmovie.h"

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

	class CAudioQueue
	{
	public:
		explicit CAudioQueue(lwmUInt32 numBytes);
		~CAudioQueue();
		lwmUInt32 NumBytesQueued() const;
		lwmUInt32 NumBytesAvailable() const;
		bool UnderrunOccurred() const;
		void ClearUnderrun();
		lwmUInt32 Capacity() const;
		void *GetQueuePoint();
		void CommitQueuedData(lwmUInt32 numBytes);

		static void SDLCALL StaticPullFromSDL(void *userdata, Uint8 * stream, int len);

	private:
		void PullFromSDL(lwmUInt8 *targetBytes, lwmUInt32 len);

		bool m_underrunOccurred;
		lwmUInt32 m_numBytesQueued;
		lwmUInt32 m_capacity;
		lwmUInt8 *m_sampleBytes;
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

lwplay::CAudioQueue::CAudioQueue(lwmUInt32 numBytes)
	: m_numBytesQueued(0)
	, m_capacity(numBytes)
	, m_underrunOccurred(false)
{
	m_sampleBytes = new lwmUInt8[numBytes];
}

lwplay::CAudioQueue::~CAudioQueue()
{
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

void lwplay::CAudioQueue::ClearUnderrun()
{
	m_underrunOccurred = false;
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
	FILE *f = fopen(argv[1], "rb");

	lwmMovieState *movieState = lwmCreateMovieState(&myAllocator, 0);
	lwmSVideoFrameProvider *frameProvider = NULL;
	lwmIVideoReconstructor *videoRecon = NULL;
	lwmVideoRGBConverter *rgbConverter = NULL;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	bool textureIsYUV = false;
	lwplay::CAudioQueue *audioQueue = NULL;
	lwmUInt32 vidWidth, vidHeight, fpsNum, fpsDenom;

	bool playingAudio = false;
	SDL_AudioSpec wantAudio, haveAudio;
	SDL_AudioDeviceID audioDeviceID;

	lwmUInt32 numFramesDecoded = 0;
	lwmUInt32 playbackStartTime = 0;
	lwmUInt32 audioSampleRate = 0;
	lwmUInt8 sampleSizeBytes = 0;

	while(true)
	{
		if(numBytesAvailable == 0 && !eof)
		{
			numBytesAvailable = fread(dataBuffer, 1, sizeof(dataBuffer), f);
			if(numBytesAvailable == 0)
				eof = true;
			else
				unreadDataStart = dataBuffer;
		}

		lwmUInt32 numBytesDigested;
		lwmUInt32 digestResult;
		lwmMovieState_FeedData(movieState, unreadDataStart, static_cast<lwmUInt32>(numBytesAvailable), &digestResult, &numBytesDigested);
		numBytesAvailable -= numBytesDigested;
		unreadDataStart += numBytesDigested;

		switch(digestResult)
		{
		case lwmDIGEST_Initialize:
			{
				lwmUInt32 reconType;
				lwmUInt32 frameFormat, channelLayout;
				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Width, &vidWidth);
				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_Height, &vidHeight);

				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_PPSNumerator, &fpsNum);
				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_PPSDenominator, &fpsDenom);

				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_ReconType, &reconType);
				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_VideoFrameFormat, &frameFormat);
				lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Video, 0, lwmSTREAMPARAM_U32_VideoChannelLayout, &channelLayout);

				frameProvider = lwmCreateSystemMemoryFrameProvider(&myAllocator, movieState);
				videoRecon = lwmCreateSoftwareVideoReconstructor(movieState, &myAllocator, reconType, 0, frameProvider);
				lwmMovieState_SetVideoReconstructor(movieState, videoRecon);

				lwmUInt32 audioSpeakerLayout;
				int neededChannels = 0;
				lwmUInt8 audioStreamCount = lwmMovieState_GetAudioStreamCount(movieState);

				if(audioStreamCount)
				{
					// Activate audio stream 0
					if(lwmMovieState_SetAudioStreamEnabled(movieState, 0, 1))
					{
						lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Audio, 0, lwmSTREAMPARAM_U32_SampleRate, &audioSampleRate);
						lwmMovieState_GetStreamParameterU32(movieState, lwmSTREAMTYPE_Audio, 0, lwmSTREAMPARAM_U32_SpeakerLayout, &audioSpeakerLayout);

						if(audioSpeakerLayout == lwmSPEAKERLAYOUT_Mono)
						{
							sampleSizeBytes = 2;
							neededChannels = 1;
						}
						else if(audioSpeakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
						{
							neededChannels = 2;
							sampleSizeBytes = 4;
						}

						SDL_zero(wantAudio);
						wantAudio.freq = static_cast<int>(audioSampleRate);
						wantAudio.channels = static_cast<int>(neededChannels);
						wantAudio.format = AUDIO_S16SYS;
						wantAudio.samples = 512;
						wantAudio.userdata = audioQueue = new lwplay::CAudioQueue(sampleSizeBytes * 8192);
						wantAudio.callback = lwplay::CAudioQueue::StaticPullFromSDL;
						audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wantAudio, &haveAudio, 0);
						SDL_PauseAudioDevice(audioDeviceID, 0);

						playingAudio = true;
					}
				}

				// TODO: Check for failures
				window = SDL_CreateWindow("lwplay", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<int>(vidWidth), static_cast<int>(vidHeight), 0);
				renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

				if(channelLayout == lwmVIDEOCHANNELLAYOUT_YCbCr_BT601 && frameFormat == lwmFRAMEFORMAT_8Bit_420P_Planar)
				{
					textureIsYUV = true;
					texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(vidWidth), static_cast<int>(vidHeight));
				}
				else
				{
					rgbConverter = lwmVideoRGBConverter_Create(&myAllocator, videoRecon, static_cast<lwmEFrameFormat>(frameFormat), static_cast<lwmEVideoChannelLayout>(channelLayout), vidWidth, vidHeight, lwmVIDEOCHANNELLAYOUT_RGBA);

					int sdlPixelFormat;
					if(SDL_PIXELTYPE(SDL_PIXELFORMAT_RGBA8888) == SDL_PIXELTYPE_PACKED32 && SDL_BYTEORDER == SDL_LIL_ENDIAN)
						sdlPixelFormat = SDL_PIXELFORMAT_ABGR8888;
					else
						sdlPixelFormat = SDL_PIXELFORMAT_RGBA8888;

					texture = SDL_CreateTexture(renderer, sdlPixelFormat, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(vidWidth), static_cast<int>(vidHeight));
					textureIsYUV = false;
				}

				//PTP_WORK videoReconWork = CreateThreadpoolWork(MyVideoReconWorkCallback, videoRecon, NULL);
				//videoReconNotifier.SetWork(videoReconWork);
				//lwmVideoRecon_SetWorkNotifier(videoRecon, &videoReconNotifier);

			}
			break;
		case lwmDIGEST_VideoSync:
			{
				numFramesDecoded++;
				Uint32 timeSinceStart = 0;
				Uint32 currentTime = SDL_GetTicks();
				if(playbackStartTime == 0)
					playbackStartTime = currentTime;
				else
				{
					lwmUInt32 thisFrameTime = playbackStartTime + static_cast<lwmUInt32>(static_cast<lwmUInt64>(numFramesDecoded) * 1000 * fpsDenom / fpsNum);
					if(currentTime < thisFrameTime)
					{
						SDL_Delay(thisFrameTime - currentTime);
					}
					else
					{
						lwmUInt32 slip = currentTime - thisFrameTime;
						if(slip > 1000)
							playbackStartTime += slip;
					}
				}

				SDL_Event e;
				while(SDL_PollEvent(&e))
				{
					if(e.type == SDL_QUIT)
						goto exitMovieLoop;
				}

				const lwmUInt8 *yBuffer;
				const lwmUInt8 *uBuffer;
				const lwmUInt8 *vBuffer;
				lwmUInt32 yStride, uStride, vStride;

				lwmUInt32 frameIndex = lwmVideoRecon_GetWorkFrameIndex(videoRecon);
				frameProvider->lockWorkFrameFunc(frameProvider, frameIndex, lwmVIDEOLOCK_Read);

				yBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 0));
				uBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 1));
				vBuffer = static_cast<const lwmUInt8 *>(frameProvider->getWorkFramePlaneFunc(frameProvider, frameIndex, 2));

				yStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 0);
				uStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 1);
				vStride = frameProvider->getWorkFramePlaneStrideFunc(frameProvider, 2);

				SDL_Rect rect;
				rect.x = rect.y = 0;
				rect.w = static_cast<int>(vidWidth);
				rect.h = static_cast<int>(vidHeight);

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

				if(playingAudio)
				{
					bool canQueueSamples = false;
					SDL_LockAudioDevice(audioDeviceID);
					if(lwmMovieState_IsAudioPlaybackSynchronized(movieState))
					{
						if(audioQueue->UnderrunOccurred())
						{
							// Underrun occurred
							lwmMovieState_NotifyAudioPlaybackUnderrun(movieState);
							audioQueue->ClearUnderrun();
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
						if(audioQueue->NumBytesQueued() == 0)
						{
							if(lwmMovieState_SynchronizeAudioPlayback(movieState) != 0)
							{
								canQueueSamples = true;
								audioQueue->ClearUnderrun();
							}
						}
					}

					if(canQueueSamples)
					{
						void *dest = audioQueue->GetQueuePoint();
						lwmUInt32 numSamplesRead = lwmMovieState_ReadAudioSamples(movieState, 0, dest, audioQueue->NumBytesAvailable() / sampleSizeBytes);
						audioQueue->CommitQueuedData(numSamplesRead * sampleSizeBytes);
					}
					SDL_UnlockAudioDevice(audioDeviceID);
				}
			}
			break;
		case lwmDIGEST_Error:
			goto exitMovieLoop;
		default:
			break;
		};

		if(digestResult == lwmDIGEST_Nothing && eof)
		{
			// End of movie
			break;
		}
	}

exitMovieLoop:
	lwmMovieState_Destroy(movieState);
	if(rgbConverter)
		lwmVideoRGBConverter_Destroy(rgbConverter);
	lwmIVideoReconstructor_Destroy(videoRecon);
	lwmSVideoFrameProvider_Destroy(frameProvider);

	if(playingAudio)
		SDL_CloseAudioDevice(audioDeviceID);
	if(audioQueue)
		delete audioQueue;
	SDL_Quit();

	return 0;
}
