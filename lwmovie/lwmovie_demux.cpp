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
#include <new>
#include "lwmovie.h"
#include "lwmovie_fillable.hpp"
#include "lwmovie_package.hpp"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_recon_m1vsw.hpp"
#include "lwmovie_audiobuffer.hpp"
#include "lwmovie_mp2_decoder.hpp"
#include "lwmovie_celt_decoder.hpp"
#include "lwmovie_adpcm_decoder.hpp"
#include "lwmovie_roq.hpp"
#include "lwmovie_recon_roqsw.hpp"
#include "../mp2dec2/lwmovie_layer2_decodestate.hpp"

namespace lwmovie
{
	class CVidStream;

	namespace layerii
	{
		class CDecoder;
	}
	namespace celt
	{
		class CDecoder;
	}
	namespace adpcm
	{
		class CDecoder;
	}
}

enum lwmDemuxState
{
	lwmDEMUX_MovieHeader,
	lwmDEMUX_AudioCommonHeader,
	lwmDEMUX_AudioStreamHeader,
	lwmDEMUX_VideoHeader,
	lwmDEMUX_InitDecoding,
	lwmDEMUX_PacketHeader,
	lwmDEMUX_PacketHeaderFull,
	lwmDEMUX_PacketHeaderCompact,
	lwmDEMUX_PacketStart,
	lwmDEMUX_PacketData,

	lwmDEMUX_FatalError
};

struct lwmMovieState
{
	lwmMovieHeader movieInfo;
	lwmVideoStreamInfo videoInfo;
	lwmAudioCommonInfo audioCommonInfo;
	lwmAudioStreamInfo *audioStreamInfos;
	lwmPacketHeader packetInfo;

	lwmUInt8 pkgHeaderBytes[lwmPlanHandler<lwmMovieHeader>::SIZE];
	lwmUInt8 videoInfoBytes[lwmPlanHandler<lwmVideoStreamInfo>::SIZE];
	lwmUInt8 audioCommonInfoBytes[lwmPlanHandler<lwmAudioCommonInfo>::SIZE];
	lwmUInt8 audioStreamInfoBytes[lwmPlanHandler<lwmAudioStreamInfo>::SIZE];
	lwmUInt8 packetHeaderBytes[lwmPlanHandler<lwmPacketHeader>::SIZE];
	lwmUInt8 packetHeaderFullBytes[lwmPlanHandler<lwmPacketHeaderFull>::SIZE];
	lwmUInt8 packetHeaderCompactBytes[lwmPlanHandler<lwmPacketHeaderCompact>::SIZE];

	lwmUInt8 *packetDataBytesEscaped;
	lwmUInt8 *packetDataBytes;
	lwmUInt32 packetSize;
	lwmUInt8 packetStreamIndex;

	lwmFillableBuffer headerFillable;
	lwmFillableBuffer videoFillable;
	lwmFillableBuffer audioFillable;

	lwmFillableBuffer packetHeaderFillable;
	lwmFillableBuffer packetHeaderFullFillable;
	lwmFillableBuffer packetHeaderCompactFillable;
	lwmFillableBuffer packetDataFillable;

	lwmDemuxState demuxState;
	lwmUInt32 numParsedAudioHeaders;

	lwmSAllocator *alloc;
	lwmUInt32 userFlags;

	bool isAudioSynchronized;
	bool needVideoStreamParameters;
	//bool needAudioStreamParameters;

	lwmUInt32 streamSyncPeriods[lwmSTREAMTYPE_Count];

	lwmSWorkNotifier *videoDigestWorkNotifier;
	lwmIVideoReconstructor *videoReconstructor;
	lwmovie::m1v::CVidStream *m1vDecoder;
	lwmovie::roq::CVideoDecoder *roqDecoder;
	lwmovie::CAudioCodec **audioDecoders;

	lwmMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags);
	void DesyncAudio();

	lwmovie::CAudioBuffer *GetAudioBuffer(lwmLargeUInt audioStreamIndex) const;
};

lwmMovieState::lwmMovieState(lwmSAllocator *pAlloc, lwmUInt32 pUserFlags)
	: demuxState(lwmDEMUX_MovieHeader)
	, numParsedAudioHeaders(0)
	, packetDataBytesEscaped(NULL)
	, packetDataBytes(NULL)
	, userFlags(pUserFlags)
	, videoDigestWorkNotifier(NULL)
	, alloc(pAlloc)
	, m1vDecoder(NULL)
	, roqDecoder(NULL)
	, audioDecoders(NULL)
	, audioStreamInfos(NULL)
	, isAudioSynchronized(false)
	, needVideoStreamParameters(false)
	//, needAudioStreamParameters(false)
{
	this->headerFillable.Init(this->pkgHeaderBytes);
	this->videoFillable.Init(this->videoInfoBytes);
	this->audioFillable.Init(this->audioCommonInfoBytes);
}

void lwmMovieState::DesyncAudio()
{
	this->isAudioSynchronized = false;
	if(movieInfo.audioStreamType != lwmAST_None)
		for(lwmFastUInt8 i=0;i<audioCommonInfo.numAudioStreams;i++)
			if(lwmovie::CAudioBuffer *audioBuffer = this->GetAudioBuffer(i))
				audioBuffer->ClearAll();
}

lwmovie::CAudioBuffer *lwmMovieState::GetAudioBuffer(lwmLargeUInt streamIndex) const
{
	if(this->audioDecoders && streamIndex < this->audioCommonInfo.numAudioStreams)
	{
		if(lwmovie::CAudioCodec *codec = this->audioDecoders[streamIndex])
			return codec->GetAudioBuffer();
	}
	return NULL;
}

static void UnescapePacket(const void *inBuffer, void *outBuffer, lwmUInt32 inSize, lwmUInt32 *pOutSize)
{
	lwmUInt32 outSize = 0;
	const lwmUInt8 *inBytes = static_cast<const lwmUInt8 *>(inBuffer);
	lwmUInt8 *outBytes = static_cast<lwmUInt8*>(outBuffer);

	if(inSize <= 3)
	{
		for(lwmUInt32 i=0;i<inSize;i++)
			outBytes[i] = inBytes[i];
		*pOutSize = inSize;
		return;
	}

	lwmUInt8 *outByteBase = outBytes;

	for(lwmUInt32 i=0;i<3;i++)
		outBytes[i] = inBytes[i];

	outSize = 3;
	lwmUInt32 prevBytes = (inBytes[0] << 16) | (inBytes[1] << 8) | inBytes[2];

	outBytes += 3;
	inBytes += 3;
	for(lwmUInt32 i=3;i<inSize;i++)
	{
		lwmUInt32 inByte = *inBytes++;
		prevBytes = (prevBytes << 8) | inByte;
		if(prevBytes != 0x000001fe)
			*outBytes++ = inByte;
	}

	*pOutSize = static_cast<lwmUInt32>(outBytes - outByteBase);
}

static void DigestPacket(lwmMovieState *movieState, lwmUInt32 *outResult)
{
	const void *packetData = movieState->packetDataBytesEscaped;
	lwmUInt32 packetSize = movieState->packetSize;
	if(movieState->packetInfo.packetTypeAndFlags & lwmPacketHeader::EFlag_Escaped)
	{
		UnescapePacket(packetData, movieState->packetDataBytes, movieState->packetSize, &packetSize);
		packetData = movieState->packetDataBytes;
	}

	lwmEPacketType packetType = static_cast<lwmEPacketType>(movieState->packetInfo.packetTypeAndFlags & ~(lwmPacketHeader::EFlag_All));
	bool expectVSP = false;
	bool expectASP = false;

	if(movieState->needVideoStreamParameters)
		expectVSP = true;
	//else if(movieState->needAudioStreamParameters)
	//	expectASP = true;

	if((expectVSP && packetType != lwmEPT_Video_StreamParameters) || (expectASP && packetType != lwmEPT_Audio_StreamParameters))
	{
		*outResult = lwmDIGEST_Error;
		return;
	}

	switch(packetType)
	{
	case lwmEPT_Video_StreamParameters:
		if(!expectVSP || movieState->movieInfo.videoStreamType == lwmVST_None)
			*outResult = lwmDIGEST_Error;
		else
		{
			lwmEDigestResult result = lwmDIGEST_Worked;
			if(movieState->m1vDecoder)
				movieState->m1vDecoder->WaitForDigestFinish();
			movieState->videoReconstructor->WaitForFinish();
			if(movieState->m1vDecoder)
			{
				if(movieState->m1vDecoder->DigestStreamParameters(packetData, packetSize))
					movieState->needVideoStreamParameters = false;
				else
					result = lwmDIGEST_Error;
			}
			*outResult = result;
		}
		break;
	case lwmEPT_Video_InlinePacket:
		if(movieState->movieInfo.videoStreamType == lwmVST_None)
			*outResult = lwmDIGEST_Error;
		else
		{
			lwmEDigestResult result = lwmDIGEST_Worked;
			if (movieState->m1vDecoder)
			{
				if (!movieState->m1vDecoder->DigestDataPacket(packetData, packetSize, outResult))
					result = lwmDIGEST_Error;
			}
			else if (movieState->roqDecoder)
			{
				if (!movieState->roqDecoder->DigestDataPacket(packetData, packetSize, outResult))
					result = lwmDIGEST_Error;
			}
			*outResult = result;
		}
		break;
	case lwmEPT_Video_Synchronization:
		{
			if(movieState->movieInfo.videoStreamType == lwmVST_None)
				*outResult = lwmDIGEST_Error;
			else
			{
				lwmVideoSynchronizationPoint syncPoint;
				if(packetSize == lwmPlanHandler<lwmVideoSynchronizationPoint>::SIZE && 
					lwmPlanHandler<lwmVideoSynchronizationPoint>::Read(syncPoint, packetData))
				{
					bool frameAvailable = false;
					if (movieState->m1vDecoder)
					{
						movieState->m1vDecoder->WaitForDigestFinish();
						frameAvailable = movieState->m1vDecoder->EmitFrame();
					}
					if (movieState->roqDecoder)
					{
						//movieState->roqDecoder->WaitForFinish();
						frameAvailable = true;
					}
					movieState->videoReconstructor->WaitForFinish();
					movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] = syncPoint.videoPeriod;
					if(frameAvailable)
						*outResult = lwmDIGEST_VideoSync;
					else
						*outResult = lwmDIGEST_VideoSync_Dropped;
				}
				else
				{
					*outResult = lwmDIGEST_Error;
				}
			}
		}
		break;
	case lwmEPT_Audio_StreamParameters:
		if(!expectASP)
			*outResult = lwmDIGEST_Error;
		else
		{
			lwmEDigestResult result = lwmDIGEST_Worked;
			*outResult = result;
		}
		break;
	case lwmEPT_Audio_Frame:
		{
			if(movieState->movieInfo.audioStreamType == lwmAST_None || movieState->packetStreamIndex >= movieState->audioCommonInfo.numAudioStreams)
				*outResult = lwmDIGEST_Error;
			else
			{
				bool overrun = false;
				bool mustDesync = false;
				lwmUInt8 streamIndex = movieState->packetStreamIndex;

				if(movieState->audioDecoders && movieState->audioDecoders[streamIndex] && !movieState->audioDecoders[streamIndex]->DigestDataPacket(packetData, packetSize, overrun))
				{
					*outResult = lwmDIGEST_Error;
					mustDesync = true;
				}
				else
					*outResult = lwmDIGEST_Worked;

				// Only need to desync if audio wasn't synchronized.  Overruns are permitted at the start to absorb algorithmic delay.
				if(overrun && movieState->isAudioSynchronized)
					mustDesync = true;
				if(mustDesync)
					movieState->DesyncAudio();
			}
		}
		break;
	case lwmEPT_Audio_Synchronization:
		{
			if(movieState->movieInfo.audioStreamType == lwmAST_None)
			{
				*outResult = lwmDIGEST_Error;
			}
			else
			{
				lwmAudioSynchronizationPoint syncPoint;
				if(packetSize == lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE && 
					lwmPlanHandler<lwmAudioSynchronizationPoint>::Read(syncPoint, packetData))
				{
					lwmLargeUInt numAudioStreams = movieState->audioCommonInfo.numAudioStreams;

					if(movieState->isAudioSynchronized)
					{
						lwmUInt32 oldPeriod = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio];
						bool needSoftDesync = false;

						for(lwmLargeUInt i=0;i<numAudioStreams;i++)
						{
							lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(i);
							if(audioBuffer && (syncPoint.audioPeriod < oldPeriod || syncPoint.audioPeriod - oldPeriod != audioBuffer->GetNumUncommittedSamples()))
							{
								needSoftDesync = true;
								break;
							}
						}

						if(needSoftDesync)
						{
							// Desynchronized, but new samples are still OK
							movieState->isAudioSynchronized = false;
							for(lwmLargeUInt i=0;i<numAudioStreams;i++)
							{
								if (lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(i))
									audioBuffer->ClearCommittedSamples();
							}
						}
					}
					
					for(lwmLargeUInt i=0;i<numAudioStreams;i++)
					{
						if (lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(i))
							audioBuffer->CommitSamples();
					}
					movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] = syncPoint.audioPeriod;
					*outResult = lwmDIGEST_Worked;
				}
				else
				{
					movieState->DesyncAudio();
					*outResult = lwmDIGEST_Error;
				}
			}
		}
		*outResult = lwmDIGEST_Worked;
		break;
	default:
		*outResult = lwmDIGEST_Error;
		break;
	};
}

class lwmCAudioCodecRegistry
{
private:
	template<class T>
	static lwmovie::CAudioCodec *CreateAudioCodecTpl(lwmSAllocator *alloc, const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo)
	{
		T *codec = alloc->NAlloc<T>(1);
		if(!codec)
			return NULL;
		new (codec) T(alloc);
		if(!codec->Init(movieHeader, audioCommonInfo, audioStreamInfo))
		{
			codec->~T();
			alloc->Free(codec);
			return NULL;
		}
		return codec;
	}
	
public:
	static lwmovie::CAudioCodec *CreateAudioCodec(lwmEAudioStreamType codecType, lwmSAllocator *alloc, const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo)
	{
		switch(codecType)
		{
		case lwmAST_MP2:
			return CreateAudioCodecTpl<lwmovie::layerii::CDecoder>(alloc, movieHeader, audioCommonInfo, audioStreamInfo);
		case lwmAST_OpusCustom:
			return CreateAudioCodecTpl<lwmovie::celt::CDecoder>(alloc, movieHeader, audioCommonInfo, audioStreamInfo);
		case lwmAST_ADPCM:
			return CreateAudioCodecTpl<lwmovie::adpcm::CDecoder>(alloc, movieHeader, audioCommonInfo, audioStreamInfo);
		default:
			return NULL;
		};
	}
};

static bool InitDecoding(lwmSAllocator *alloc, lwmMovieState *movieState)
{
	switch(movieState->movieInfo.videoStreamType)
	{
	case lwmVST_None:
		// TODO: Test
		break;
	case lwmVST_M1V_Variant:
		{
			if(movieState->videoInfo.frameFormat != lwmFRAMEFORMAT_8Bit_420P_Planar)
				goto cleanup;
			if(movieState->videoInfo.channelLayout != lwmVIDEOCHANNELLAYOUT_YCbCr_BT601 &&
				movieState->videoInfo.channelLayout != lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG)
				goto cleanup;
			if(movieState->videoInfo.videoWidth > 4095)
				goto cleanup;
			if(movieState->videoInfo.videoHeight > 4095)
				goto cleanup;
			movieState->m1vDecoder = alloc->NAlloc<lwmovie::m1v::CVidStream>(1);
			if(!movieState->m1vDecoder)
				goto cleanup;
			new (movieState->m1vDecoder) lwmovie::m1v::CVidStream(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight, (movieState->videoInfo.numWriteOnlyWorkFrames > 0), movieState, movieState->videoDigestWorkNotifier, ((movieState->userFlags) & lwmUSERFLAG_ThreadedDeslicer));
			movieState->needVideoStreamParameters = true;
		}
		break;
	case lwmVST_RoQ:
		{
			if (movieState->videoInfo.frameFormat != lwmFRAMEFORMAT_8Bit_3Channel_Interleaved)
				goto cleanup;
			if (movieState->videoInfo.channelLayout != lwmVIDEOCHANNELLAYOUT_RGB)
				goto cleanup;
			if (movieState->videoInfo.numWriteOnlyWorkFrames != 0 || movieState->videoInfo.numReadWriteWorkFrames != 2)
				goto cleanup;
			movieState->roqDecoder = alloc->NAlloc<lwmovie::roq::CVideoDecoder>(1);
			if (!movieState->roqDecoder)
				goto cleanup;
			new (movieState->roqDecoder) lwmovie::roq::CVideoDecoder(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight);
			movieState->needVideoStreamParameters = false;
		}
		break;
	default:
		goto cleanup;
	};

	switch(movieState->movieInfo.audioStreamType)
	{
	case lwmAST_None:
		// TODO: Test
		break;
	case lwmAST_MP2:
	case lwmAST_OpusCustom:
	case lwmAST_ADPCM:
		movieState->audioDecoders = alloc->NAlloc<lwmovie::CAudioCodec*>(movieState->audioCommonInfo.numAudioStreams);
		for(lwmLargeUInt i=0;i<movieState->audioCommonInfo.numAudioStreams;i++)
			movieState->audioDecoders[i] = NULL;
		break;
	default:
		goto cleanup;
	};

	return true;
cleanup:
	// TODO?
	// Shouldn't actually need to clean anything up, the demux state will fatal error,
	// lwmMovieState_Destroy should do the actual cleanup.
	return false;
}

LWMOVIE_API_LINK lwmMovieState *lwmCreateMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags)
{
	lwmMovieState *movieState = alloc->NAlloc<lwmMovieState>(1);

	if(!movieState)
		return NULL;
	new (movieState) lwmMovieState(alloc, userFlags);
	return movieState;
}

LWMOVIE_API_LINK void lwmMovieState_FeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested)
{
	lwmUInt32 bytesAvailable = numBytes;
	const void *bytes = inBytes;

repeatFeed:;

	switch(movieState->demuxState)
	{
	case lwmDEMUX_MovieHeader:
		{
			if(movieState->headerFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmMovieHeader>::Read(movieState->movieInfo, movieState->pkgHeaderBytes)
					&& movieState->movieInfo.largestPacketSize > 0)
				{
					// Determine next state
					if(movieState->movieInfo.videoStreamType != lwmVST_None)
					{
						movieState->demuxState = lwmDEMUX_VideoHeader;
						goto repeatFeed;
					}
					else if(movieState->movieInfo.audioStreamType != lwmAST_None)
					{
						movieState->demuxState = lwmDEMUX_AudioCommonHeader;
						goto repeatFeed;
					}
					else
						movieState->demuxState = lwmDEMUX_FatalError;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_VideoHeader:
		{
			if(movieState->videoFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmVideoStreamInfo>::Read(movieState->videoInfo, movieState->videoInfoBytes))
				{
					if(movieState->movieInfo.audioStreamType != lwmAST_None)
					{
						movieState->demuxState = lwmDEMUX_AudioCommonHeader;
						goto repeatFeed;
					}
					else
					{
						movieState->demuxState = lwmDEMUX_InitDecoding;
						goto repeatFeed;
					}
				}
				else
				{
					movieState->demuxState = lwmDEMUX_FatalError;
					goto repeatFeed;
				}
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_AudioCommonHeader:
		{
			if(movieState->audioFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmAudioCommonInfo>::Read(movieState->audioCommonInfo, movieState->audioCommonInfoBytes))
				{
					movieState->audioStreamInfos = movieState->alloc->NAlloc<lwmAudioStreamInfo>(movieState->audioCommonInfo.numAudioStreams);

					if(movieState->audioStreamInfos)
					{
						movieState->audioFillable.Init(movieState->audioStreamInfoBytes);
						movieState->numParsedAudioHeaders = 0;
						movieState->demuxState = lwmDEMUX_AudioStreamHeader;
						goto repeatFeed;
					}
					else
						movieState->demuxState = lwmDEMUX_FatalError;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
	case lwmDEMUX_AudioStreamHeader:
		{
			if(movieState->audioFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmAudioStreamInfo>::Read(movieState->audioStreamInfos[movieState->numParsedAudioHeaders], movieState->audioStreamInfoBytes))
				{
					if((++movieState->numParsedAudioHeaders) == movieState->audioCommonInfo.numAudioStreams)
						movieState->demuxState = lwmDEMUX_InitDecoding;
					else
					{
						movieState->audioFillable.Init(movieState->audioStreamInfoBytes);
						movieState->demuxState = lwmDEMUX_AudioStreamHeader;
					}
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_InitDecoding:
		{
			movieState->packetDataBytesEscaped = movieState->alloc->NAlloc<lwmUInt8>(movieState->movieInfo.largestPacketSize);
			movieState->packetDataBytes = movieState->alloc->NAlloc<lwmUInt8>(movieState->movieInfo.largestPacketSize);
			movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

			if(movieState->packetDataBytesEscaped == NULL || movieState->packetDataBytes == NULL)
			{
				movieState->demuxState = lwmDEMUX_FatalError;
				*outResult = lwmDIGEST_Nothing;
			}
			else
			{
				movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

				if(InitDecoding(movieState->alloc, movieState))
				{
					movieState->demuxState = lwmDEMUX_PacketHeader;
					*outResult = lwmDIGEST_Initialize;
				}
				else
				{
					movieState->demuxState = lwmDEMUX_FatalError;
					*outResult = lwmDIGEST_Nothing;
				}
			}

			*outBytesDigested = numBytes - bytesAvailable;
		}
		break;
	case lwmDEMUX_PacketHeader:
		{
			if(movieState->packetHeaderFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmPacketHeader>::Read(movieState->packetInfo, movieState->packetHeaderBytes))
				{
					if(movieState->packetInfo.packetTypeAndFlags & lwmPacketHeader::EFlag_Compact)
					{
						movieState->packetHeaderCompactFillable.Init(movieState->packetHeaderCompactBytes);
						movieState->demuxState = lwmDEMUX_PacketHeaderCompact;
					}
					else
					{
						movieState->packetHeaderFullFillable.Init(movieState->packetHeaderFullBytes);
						movieState->demuxState = lwmDEMUX_PacketHeaderFull;
					}
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;	// TODO: Recovery
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
		}
		break;
	case lwmDEMUX_PacketStart:
		{
			if(movieState->packetSize > movieState->movieInfo.largestPacketSize || movieState->packetSize == 0)
				movieState->demuxState = lwmDEMUX_FatalError;	// TODO: Recovery
			else
			{
				movieState->packetDataFillable.Init(movieState->packetDataBytesEscaped, movieState->packetSize);
				movieState->demuxState = lwmDEMUX_PacketData;
				goto repeatFeed;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketHeaderCompact:
		{
			if(movieState->packetHeaderCompactFillable.FeedData(&bytes, &bytesAvailable))
			{
				lwmPacketHeaderCompact compactHeader;
				if(lwmPlanHandler<lwmPacketHeaderCompact>::Read(compactHeader, movieState->packetHeaderCompactBytes) &&
					(compactHeader.packetSize & 0x01) == 1 &&
					(compactHeader.packetSize & 0xfffe) != 0)
				{
					movieState->packetSize = compactHeader.packetSize >> 1;
					movieState->packetStreamIndex = 0;
					movieState->demuxState = lwmDEMUX_PacketStart;
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketHeaderFull:
		{
			if(movieState->packetHeaderFullFillable.FeedData(&bytes, &bytesAvailable))
			{
				lwmPacketHeaderFull fullHeader;
				if(lwmPlanHandler<lwmPacketHeaderFull>::Read(fullHeader, movieState->packetHeaderFullBytes))
				{
					movieState->packetSize = fullHeader.packetSize;
					movieState->packetStreamIndex = fullHeader.streamIndex;
					movieState->demuxState = lwmDEMUX_PacketStart;
					goto repeatFeed;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}
			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_PacketData:
		{
			if(movieState->packetDataFillable.FeedData(&bytes, &bytesAvailable))
			{
				DigestPacket(movieState, outResult);
				*outBytesDigested = numBytes - bytesAvailable;

				movieState->demuxState = lwmDEMUX_PacketHeader;
				movieState->packetHeaderFillable.Init(movieState->packetHeaderBytes);

				return;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
	case lwmDEMUX_FatalError:
		*outResult = lwmDIGEST_Error;
		*outBytesDigested = 0;
		return;
	};
}

LWMOVIE_API_LINK lwmUInt8 lwmMovieState_GetAudioStreamCount(const lwmMovieState *movieState)
{
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;
	return movieState->audioCommonInfo.numAudioStreams;
}

LWMOVIE_API_LINK int lwmMovieState_SetAudioStreamEnabled(struct lwmMovieState *movieState, lwmUInt8 streamIndex, int enable)
{
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;
	if(streamIndex >= movieState->audioCommonInfo.numAudioStreams)
		return 0;

	if(enable != 0)
	{
		if(movieState->audioDecoders[streamIndex] == NULL)
		{
			lwmovie::CAudioCodec *codec = lwmCAudioCodecRegistry::CreateAudioCodec(movieState->movieInfo.audioStreamType, movieState->alloc, &movieState->movieInfo, &movieState->audioCommonInfo, movieState->audioStreamInfos + streamIndex);
			if(codec)
			{
				movieState->DesyncAudio();
				movieState->audioDecoders[streamIndex] = codec;
				return 1;
			}
			else
				return 0;
		}
		else
			return 1;	// Codec already enabled
	}
	else
	{
		// Disable
		if(movieState->audioDecoders[streamIndex] != NULL)
		{
			movieState->audioDecoders[streamIndex]->~CAudioCodec();
			movieState->alloc->Free(movieState->audioDecoders[streamIndex]);
			movieState->audioDecoders[streamIndex] = NULL;

			// Desyncing audio isn't necessary
		}
		return 1;
	}
}

LWMOVIE_API_LINK int lwmMovieState_GetStreamParameterU32(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, lwmUInt32 streamParameterU32, lwmUInt32 *outValue)
{
	switch(streamType)
	{
	case lwmSTREAMTYPE_Video:
		{
			if(streamIndex != 0)
				return 0;

			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_Width:
				*outValue = movieState->videoInfo.videoWidth;
				return 1;
			case lwmSTREAMPARAM_U32_Height:
				*outValue = movieState->videoInfo.videoHeight;
				return 1;
			case lwmSTREAMPARAM_U32_NumReadWriteWorkFrames:
				*outValue = movieState->videoInfo.numReadWriteWorkFrames;
				return 1;
			case lwmSTREAMPARAM_U32_NumWriteOnlyWorkFrames:
				*outValue = movieState->videoInfo.numWriteOnlyWorkFrames;
				return 1;
			case lwmSTREAMPARAM_U32_PPSNumerator:
				*outValue = movieState->videoInfo.periodsPerSecondNum;
				return 1;
			case lwmSTREAMPARAM_U32_PPSDenominator:
				*outValue = movieState->videoInfo.periodsPerSecondDenom;
				return 1;
			case lwmSTREAMPARAM_U32_ReconType:
				{
					switch(movieState->movieInfo.videoStreamType)
					{
					case lwmVST_M1V_Variant:
						*outValue = lwmRC_MPEG1Video;
						return 1;
					case lwmVST_RoQ:
						*outValue = lwmRC_RoQ;
						return 1;
					default:
						return 0;
					};
				}
				return 1;
			case lwmSTREAMPARAM_U32_VideoFrameFormat:
				{
					*outValue = movieState->videoInfo.frameFormat;
					return 1;
				}
				return 1;
			case lwmSTREAMPARAM_U32_VideoChannelLayout:
				{
					*outValue = movieState->videoInfo.channelLayout;
					return 1;
				}
				return 1;
			case lwmSTREAMPARAM_U32_SyncPeriod:
				*outValue = movieState->streamSyncPeriods[lwmSTREAMTYPE_Video];
				return 1;
			case lwmSTREAMPARAM_U32_LongestFrameReadAhead:
				*outValue = movieState->movieInfo.longestFrameReadahead;
				return 1;
			};
			break;
		}
	case lwmSTREAMTYPE_Audio:
		{
			if(movieState->movieInfo.audioStreamType == lwmAST_None)
				return 0;
			if(streamIndex >= movieState->audioCommonInfo.numAudioStreams)
				return 0;

			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_SampleRate:
				*outValue = movieState->audioCommonInfo.sampleRate;
				return 1;
			case lwmSTREAMPARAM_U32_SpeakerLayout:
				*outValue = movieState->audioStreamInfos[streamIndex].speakerLayout;
				return 1;
			default:
				return 0;
			};
			break;
		}
	};
	return 0;
}

LWMOVIE_API_LINK void lwmMovieState_SetStreamParameterU32(lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, lwmUInt32 streamParameterU32, lwmUInt32 value)
{
	switch(streamType)
	{
	case lwmSTREAMTYPE_Video:
		{
			if(streamIndex != 0)
				return;
			
			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_DropAggressiveness:
				if(movieState->m1vDecoder)
					movieState->m1vDecoder->SetDropAggressiveness(static_cast<lwmEDropAggressiveness>(value));
				return;
			default:
				return;
			};
		}
	default:
		return;
	}
}

LWMOVIE_API_LINK int lwmMovieState_GetStreamMetaID(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt8 streamIndex, char outMetaID[8])
{
	const lwmUInt32 *metaIDChunks = NULL;
	if (streamType == lwmSTREAMTYPE_Audio)
		metaIDChunks = movieState->audioStreamInfos[streamIndex].metaID;

	if (metaIDChunks != NULL)
	{
		for (lwmFastUInt8 i = 0; i < 8; i++)
			outMetaID[i] = static_cast<char>((metaIDChunks[i / 4] >> ((i % 4) * 8)) & 0xff);
		return 1;
	}
	return 0;
}

LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmUInt32 flags, lwmSVideoFrameProvider *frameProvider)
{
	switch(reconstructorType)
	{
	case lwmRC_MPEG1Video:
		{
			bool useRowThreading = ((flags & lwmUSERFLAG_ThreadedReconstructor) != 0);
			lwmovie::m1v::CSoftwareReconstructor *recon = alloc->NAlloc<lwmovie::m1v::CSoftwareReconstructor>(1);
			if(!recon)
				return NULL;
			new (recon) lwmovie::m1v::CSoftwareReconstructor();
			// TODO: Low memory flag
			if(!recon->Initialize(alloc, frameProvider, movieState, useRowThreading))
			{
				lwmIVideoReconstructor_Destroy(recon);
				recon = NULL;
			}
			return recon;
		}
		break;
	case lwmRC_RoQ:
		{
			lwmovie::roq::CSoftwareReconstructor *recon = alloc->NAlloc<lwmovie::roq::CSoftwareReconstructor>(1);
			if (!recon)
				return NULL;
			new (recon) lwmovie::roq::CSoftwareReconstructor();
			if (!recon->Initialize(alloc, frameProvider, movieState))
			{
				lwmIVideoReconstructor_Destroy(recon);
				recon = NULL;
			}
			return recon;
		}
		break;
	};
	return NULL;
}

LWMOVIE_API_LINK void lwmIVideoReconstructor_Destroy(lwmIVideoReconstructor *videoRecon)
{
	videoRecon->Destroy();
}

LWMOVIE_API_LINK void lwmMovieState_SetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon)
{
	movieState->videoReconstructor = recon;
	if (movieState->m1vDecoder)
		movieState->m1vDecoder->SetReconstructor(recon);
	if (movieState->roqDecoder)
		movieState->roqDecoder->SetReconstructor(recon);
}

LWMOVIE_API_LINK void lwmMovieState_SetVideoDigestWorkNotifier(lwmMovieState *movieState, lwmSWorkNotifier *videoDigestWorkNotifier)
{
	movieState->videoDigestWorkNotifier = videoDigestWorkNotifier;
}

LWMOVIE_API_LINK void lwmMovieState_Destroy(lwmMovieState *movieState)
{
	lwmSAllocator *alloc = movieState->alloc;
	if(movieState->packetDataBytesEscaped)
		alloc->Free(movieState->packetDataBytesEscaped);
	if(movieState->packetDataBytes)
		alloc->Free(movieState->packetDataBytes);
	if(movieState->m1vDecoder)
	{
		movieState->m1vDecoder->~CVidStream();
		alloc->Free(movieState->m1vDecoder);
	}
	if (movieState->roqDecoder)
	{
		movieState->roqDecoder->~CVideoDecoder();
		alloc->Free(movieState->roqDecoder);
	}
	if(movieState->audioDecoders)
	{
		for(lwmLargeUInt i=0;i<movieState->audioCommonInfo.numAudioStreams;i++)
		{
			lwmovie::CAudioCodec *audioDecoder = movieState->audioDecoders[i];
			if(audioDecoder)
			{
				audioDecoder->~CAudioCodec();
				alloc->Free(audioDecoder);
			}
		}
		alloc->Free(movieState->audioDecoders);
	}
	if(movieState->audioStreamInfos)
		alloc->Free(movieState->audioStreamInfos);
	alloc->Free(movieState);
}


LWMOVIE_API_LINK void lwmFlushProfileTags(lwmMovieState *movieState, lwmCProfileTagSet *tagSet)
{
	if(movieState->videoReconstructor)
		movieState->videoReconstructor->FlushProfileTags(tagSet);
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->FlushProfileTags(tagSet);
}

LWMOVIE_API_LINK void lwmVideoRecon_SetWorkNotifier(lwmIVideoReconstructor *recon, lwmSWorkNotifier *workNotifier)
{
	recon->SetWorkNotifier(workNotifier);
}

LWMOVIE_API_LINK void lwmMovieState_VideoDigestParticipate(lwmMovieState *movieState)
{
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->Participate();
}

LWMOVIE_API_LINK int lwmMovieState_IsAudioPlaybackSynchronized(lwmMovieState *movieState)
{
	return movieState->isAudioSynchronized ? 1 : 0;
}

LWMOVIE_API_LINK int lwmMovieState_SynchronizeAudioPlayback(lwmMovieState *movieState)
{
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;	// No audio
	if(movieState->movieInfo.videoStreamType == lwmVST_None)
		return 0;	// No video
	if(movieState->isAudioSynchronized)
		return 1;	// Already synchronized
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] == 0 || movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] == 0)
		return 0;	// No streams
	// audioTime = videoPeriod * sampleRate / (ppsNum/ppsDenom)
	lwmUInt64 timeCalc = static_cast<lwmUInt64>(movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] - 1) * movieState->videoInfo.periodsPerSecondDenom;
	if(timeCalc > static_cast<lwmUInt32>(0xffffffff))
		return 0;	// Too big
	timeCalc *= movieState->audioCommonInfo.sampleRate;
	timeCalc /= movieState->videoInfo.periodsPerSecondNum;

	if(timeCalc > static_cast<lwmUInt32>(0xffffffff))
		return 0;	// Too big

	lwmUInt32 audioTime = static_cast<lwmUInt32>(timeCalc);

	lwmLargeUInt numAudioStreams = movieState->audioCommonInfo.numAudioStreams;
	lwmovie::CAudioBuffer *firstAudioBuffer = NULL;
	
	for(lwmLargeUInt i=0;i<numAudioStreams;i++)
	{
		if(movieState->audioDecoders[i])
		{
			firstAudioBuffer = movieState->audioDecoders[i]->GetAudioBuffer();
			break;
		}
	}

	// No streams enabled
	if(!firstAudioBuffer)
		return 0;

	lwmUInt32 numCommittedSamples = firstAudioBuffer->GetNumCommittedSamples();
	for(lwmLargeUInt i=0;i<movieState->audioCommonInfo.numAudioStreams;i++)
	{
		lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(i);
		if(!audioBuffer)
			continue;

		if(audioBuffer->GetNumCommittedSamples() != numCommittedSamples)
			return 0;	// TODO: Assert.  This should never happen, audio sync packets with inconsistent commit counts should desync.

		// If decoded samples are below the 0 point, trim them.  This is normal due to algorithmic start delay.
		if(numCommittedSamples > movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio])
			audioBuffer->SkipSamples(numCommittedSamples - movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio]);
	}

	lwmUInt32 audioBufferStartPoint = 0;
	lwmUInt32 truncatedSamples = 0;
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] > numCommittedSamples)
		audioBufferStartPoint = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] - numCommittedSamples;
	else
		truncatedSamples = numCommittedSamples - movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio];

	if(audioBufferStartPoint > audioTime)
		return 0;	// Audio starts in the future
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] <= audioTime)
		return 0;	// Audio ends in the past

	// Audio starts immediately or in the past, trim any preceding samples and complete
	if(audioTime != audioBufferStartPoint || truncatedSamples > 0)
	{
		for(lwmLargeUInt i=0;i<movieState->audioCommonInfo.numAudioStreams;i++)
		{
			if (lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(i))
				audioBuffer->SkipSamples(audioTime - audioBufferStartPoint + truncatedSamples);
		}
	}
	movieState->isAudioSynchronized = true;
	return 1;
}

LWMOVIE_API_LINK lwmUInt32 lwmMovieState_GetNumAudioSamplesAvailable(struct lwmMovieState *movieState, lwmUInt8 streamIndex)
{
	if(!movieState->isAudioSynchronized)
		return 0;
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;
	if(streamIndex >= movieState->audioCommonInfo.numAudioStreams)
		return 0;
	return movieState->GetAudioBuffer(streamIndex)->GetNumCommittedSamples();
}

LWMOVIE_API_LINK lwmUInt32 lwmMovieState_ReadAudioSamples(lwmMovieState *movieState, lwmUInt8 streamIndex, void *samples, lwmUInt32 numSamples)
{
	if(!movieState->isAudioSynchronized)
		return 0;
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;
	if(streamIndex >= movieState->audioCommonInfo.numAudioStreams)
		return 0;
	lwmovie::CAudioBuffer *audioBuffer = movieState->GetAudioBuffer(streamIndex);
	if(!audioBuffer)
		return 0;

	return audioBuffer->ReadCommittedSamples(samples, numSamples);
}

LWMOVIE_API_LINK void lwmMovieState_NotifyAudioPlaybackUnderrun(lwmMovieState *movieState)
{
	return movieState->DesyncAudio();
}

LWMOVIE_API_LINK void lwmVideoRecon_Participate(lwmIVideoReconstructor *recon)
{
	recon->Participate();
}

LWMOVIE_API_LINK lwmUInt32 lwmVideoRecon_GetWorkFrameIndex(const lwmIVideoReconstructor *recon)
{
	return recon->GetWorkFrameIndex();
}
