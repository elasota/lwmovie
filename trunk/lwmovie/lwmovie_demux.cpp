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
#include "../mp2dec2/lwmovie_layer2_decodestate.hpp"

namespace lwmovie
{
	class lwmVidStream;
	class lwmCMP2Decoder;
	class lwmCCELTDecoder;
}


static void lwmPassThroughDigestNotifyAvailable(void *opaque)
{
	lwmMovieState_VideoDigestParticipate(static_cast<lwmMovieState*>(opaque));
}

static void lwmPassThroughJoin(void *opaque)
{
}

enum lwmDemuxState
{
	lwmDEMUX_MovieHeader,
	lwmDEMUX_AudioHeader,
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
	lwmAudioStreamInfo audioInfo;
	lwmPacketHeader packetInfo;

	lwmUInt8 pkgHeaderBytes[lwmPlanHandler<lwmMovieHeader>::SIZE];
	lwmUInt8 videoInfoBytes[lwmPlanHandler<lwmVideoStreamInfo>::SIZE];
	lwmUInt8 audioInfoBytes[lwmPlanHandler<lwmAudioStreamInfo>::SIZE];
	lwmUInt8 packetHeaderBytes[lwmPlanHandler<lwmPacketHeader>::SIZE];
	lwmUInt8 packetHeaderFullBytes[lwmPlanHandler<lwmPacketHeaderFull>::SIZE];
	lwmUInt8 packetHeaderCompactBytes[lwmPlanHandler<lwmPacketHeaderCompact>::SIZE];

	lwmUInt8 *packetDataBytesEscaped;
	lwmUInt8 *packetDataBytes;

	lwmUInt32 packetSize;
	lwmFillableBuffer headerFillable;
	lwmFillableBuffer videoFillable;
	lwmFillableBuffer audioFillable;

	lwmFillableBuffer packetHeaderFillable;
	lwmFillableBuffer packetHeaderFullFillable;
	lwmFillableBuffer packetHeaderCompactFillable;
	lwmFillableBuffer packetDataFillable;

	lwmDemuxState demuxState;

	lwmSAllocator *alloc;
	lwmUInt32 userFlags;

	bool isAudioSynchronized;
	bool needVideoStreamParameters;
	bool needAudioStreamParameters;

	lwmUInt32 streamSyncPeriods[lwmSTREAMTYPE_Count];

	lwmSWorkNotifier *videoDigestWorkNotifier;
	lwmIVideoReconstructor *videoReconstructor;
	lwmovie::lwmVidStream *m1vDecoder;
	lwmovie::lwmCMP2Decoder *mp2Decoder;
	lwmovie::lwmCCELTDecoder *celtDecoder;

	lwmMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags);
	void DesyncAudio();

	lwmCAudioBuffer *GetAudioBuffer() const;
};

lwmMovieState::lwmMovieState(lwmSAllocator *pAlloc, lwmUInt32 pUserFlags)
	: demuxState(lwmDEMUX_MovieHeader)
	, packetDataBytesEscaped(NULL)
	, packetDataBytes(NULL)
	, userFlags(pUserFlags)
	, videoDigestWorkNotifier(NULL)
	, alloc(pAlloc)
	, mp2Decoder(NULL)
	, m1vDecoder(NULL)
	, celtDecoder(NULL)
	, isAudioSynchronized(false)
	, needVideoStreamParameters(false)
	, needAudioStreamParameters(false)
{
	this->headerFillable.Init(this->pkgHeaderBytes);
	this->videoFillable.Init(this->videoInfoBytes);
	this->audioFillable.Init(this->audioInfoBytes);
}

void lwmMovieState::DesyncAudio()
{
	this->isAudioSynchronized = false;
	if(lwmCAudioBuffer *audioBuffer = this->GetAudioBuffer())
		audioBuffer->ClearAll();
}

lwmCAudioBuffer *lwmMovieState::GetAudioBuffer() const
{
	if(this->mp2Decoder)
		return this->mp2Decoder->GetAudioBuffer();
	if(this->celtDecoder)
		return this->celtDecoder->GetAudioBuffer();
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
	else if(movieState->needAudioStreamParameters)
		expectASP = true;

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
			if(movieState->m1vDecoder)
				if(!movieState->m1vDecoder->DigestDataPacket(packetData, packetSize, outResult))
					result = lwmDIGEST_Error;
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
					if(movieState->m1vDecoder)
					{
						movieState->m1vDecoder->WaitForDigestFinish();
						movieState->m1vDecoder->EmitFrame();
					}
					movieState->videoReconstructor->WaitForFinish();
					movieState->streamSyncPeriods[lwmSTREAMTYPE_Video] = syncPoint.videoPeriod;
					*outResult = lwmDIGEST_VideoSync;
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
			if(movieState->movieInfo.audioStreamType == lwmAST_None)
				*outResult = lwmDIGEST_Error;
			else
			{
				bool overrun = false;
				bool mustDesync = false;

				if(
					(movieState->mp2Decoder && !movieState->mp2Decoder->DigestDataPacket(packetData, packetSize, overrun))
					|| (movieState->celtDecoder && !movieState->celtDecoder->DigestDataPacket(packetData, packetSize, overrun))
					)
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
				lwmCAudioBuffer *audioBuffer = movieState->GetAudioBuffer();

				lwmAudioSynchronizationPoint syncPoint;
				if(packetSize == lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE && 
					lwmPlanHandler<lwmAudioSynchronizationPoint>::Read(syncPoint, packetData))
				{
					if(movieState->isAudioSynchronized)
					{
						lwmUInt32 oldPeriod = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio];
						if(syncPoint.audioPeriod < oldPeriod || syncPoint.audioPeriod - oldPeriod != audioBuffer->GetNumUncommittedSamples())
						{
							// Desynchronized, but new samples are still OK
							movieState->isAudioSynchronized = false;
							audioBuffer->ClearCommittedSamples();
						}
					}

					audioBuffer->CommitSamples();
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

static bool InitDecoding(lwmSAllocator *alloc, lwmMovieState *movieState)
{
	switch(movieState->movieInfo.videoStreamType)
	{
	case lwmVST_None:
		// TODO: Test
		break;
	case lwmVST_M1V_Variant:
		{
			if(movieState->videoInfo.videoWidth > 4095)
				goto cleanup;
			if(movieState->videoInfo.videoHeight > 4095)
				goto cleanup;
			movieState->m1vDecoder = static_cast<lwmovie::lwmVidStream*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmVidStream)));
			if(!movieState->m1vDecoder)
				goto cleanup;
			new (movieState->m1vDecoder) lwmovie::lwmVidStream(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight, movieState, movieState->videoDigestWorkNotifier, ((movieState->userFlags) & lwmUSERFLAG_ThreadedDeslicer));
			movieState->needVideoStreamParameters = true;
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
		movieState->mp2Decoder = static_cast<lwmovie::lwmCMP2Decoder*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmCMP2Decoder)));
		if(!movieState->mp2Decoder)
			goto cleanup;
		new (movieState->mp2Decoder) lwmovie::lwmCMP2Decoder(alloc);
		if(!movieState->mp2Decoder->Init(&movieState->audioInfo))
			goto cleanup;
		break;
	case lwmAST_CELT_0_11_1:
		movieState->celtDecoder = static_cast<lwmovie::lwmCCELTDecoder*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmCCELTDecoder)));
		if(!movieState->celtDecoder)
			goto cleanup;
		new (movieState->celtDecoder) lwmovie::lwmCCELTDecoder(alloc);
		if(!movieState->celtDecoder->Init(&movieState->audioInfo))
			goto cleanup;
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
	lwmMovieState *movieState = static_cast<lwmMovieState *>(alloc->allocFunc(alloc, sizeof(lwmMovieState)));

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
					if(movieState->movieInfo.videoStreamType != lwmVST_None)
					{
						movieState->demuxState = lwmDEMUX_VideoHeader;
						goto repeatFeed;
					}
					else if(movieState->movieInfo.audioStreamType != lwmAST_None)
					{
						movieState->demuxState = lwmDEMUX_AudioHeader;
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
						movieState->demuxState = lwmDEMUX_AudioHeader;
						goto repeatFeed;
					}
					else
						movieState->demuxState = lwmDEMUX_InitDecoding;
				}
				else
					movieState->demuxState = lwmDEMUX_FatalError;
			}

			*outResult = lwmDIGEST_Nothing;
			*outBytesDigested = numBytes - bytesAvailable;
			return;
		}
		break;
	case lwmDEMUX_AudioHeader:
		{
			if(movieState->audioFillable.FeedData(&bytes, &bytesAvailable))
			{
				if(lwmPlanHandler<lwmAudioStreamInfo>::Read(movieState->audioInfo, movieState->audioInfoBytes))
				{
					movieState->demuxState = lwmDEMUX_InitDecoding;
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
			movieState->packetDataBytesEscaped = static_cast<lwmUInt8*>(movieState->alloc->allocFunc(movieState->alloc, movieState->movieInfo.largestPacketSize));
			movieState->packetDataBytes = static_cast<lwmUInt8*>(movieState->alloc->allocFunc(movieState->alloc, movieState->movieInfo.largestPacketSize));
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
		break;
	};
}


LWMOVIE_API_LINK int lwmMovieState_GetStreamParameterU32(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt32 streamParameterU32, lwmUInt32 *outValue)
{
	switch(streamType)
	{
	case lwmSTREAMTYPE_Video:
		{
			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_Width:
				*outValue = movieState->videoInfo.videoWidth;
				return 1;
			case lwmSTREAMPARAM_U32_Height:
				*outValue = movieState->videoInfo.videoHeight;
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
					default:
						return 0;
					};
				}
				return 1;
			case lwmSTREAMPARAM_U32_SyncPeriod:
				*outValue = movieState->streamSyncPeriods[lwmSTREAMTYPE_Video];
				return 1;
			case lwmSTREAMPARAM_U32_LongestFrameReadAhead:
				*outValue = movieState->movieInfo.longestFrameReadahead;
				return 1;
			};
		}
	case lwmSTREAMTYPE_Audio:
		{
			if(movieState->movieInfo.audioStreamType == lwmAST_None)
				return 0;

			switch(streamParameterU32)
			{
			case lwmSTREAMPARAM_U32_SampleRate:
				*outValue = movieState->audioInfo.sampleRate;
				return 1;
			case lwmSTREAMPARAM_U32_SpeakerLayout:
				*outValue = movieState->audioInfo.speakerLayout;
				return 1;
			default:
				return 0;
			};
		}
	};
	return 0;
}


LWMOVIE_API_LINK lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmSVideoFrameProvider *frameProvider)
{
	switch(reconstructorType)
	{
	case lwmRC_MPEG1Video:
		{
			lwmovie::lwmCM1VSoftwareReconstructor *recon = static_cast<lwmovie::lwmCM1VSoftwareReconstructor*>(alloc->allocFunc(alloc, sizeof(lwmovie::lwmCM1VSoftwareReconstructor)));
			if(!recon)
				return NULL;
			new (recon) lwmovie::lwmCM1VSoftwareReconstructor();
			if(!recon->Initialize(alloc, frameProvider, movieState))
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
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->SetReconstructor(recon);
}

LWMOVIE_API_LINK void lwmMovieState_SetVideoDigestWorkNotifier(lwmMovieState *movieState, lwmSWorkNotifier *videoDigestWorkNotifier)
{
	movieState->videoDigestWorkNotifier = videoDigestWorkNotifier;
}

LWMOVIE_API_LINK void lwmMovieState_Destroy(lwmMovieState *movieState)
{
	lwmSAllocator *alloc = movieState->alloc;
	if(movieState->packetDataBytesEscaped)
		alloc->freeFunc(alloc, movieState->packetDataBytesEscaped);
	if(movieState->packetDataBytes)
		alloc->freeFunc(alloc, movieState->packetDataBytes);
	if(movieState->m1vDecoder)
	{
		movieState->m1vDecoder->~lwmVidStream();
		alloc->freeFunc(alloc, movieState->m1vDecoder);
	}
	if(movieState->mp2Decoder)
	{
		movieState->mp2Decoder->~lwmCMP2Decoder();
		alloc->freeFunc(alloc, movieState->mp2Decoder);
	}
	if(movieState->celtDecoder)
	{
		movieState->celtDecoder->~lwmCCELTDecoder();
		alloc->freeFunc(alloc, movieState->celtDecoder);
	}
	alloc->freeFunc(alloc, movieState);
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
	timeCalc *= movieState->audioInfo.sampleRate;
	timeCalc /= movieState->videoInfo.periodsPerSecondNum;

	if(timeCalc > static_cast<lwmUInt32>(0xffffffff))
		return 0;	// Too big

	lwmUInt32 audioTime = static_cast<lwmUInt32>(timeCalc);

	lwmCAudioBuffer *audioBuffer = movieState->GetAudioBuffer();
	// If decoded samples are below the 0 point, trim them.  This is normal due to algorithmic start delay.
	if(audioBuffer->GetNumCommittedSamples() > movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio])
		audioBuffer->SkipSamples(audioBuffer->GetNumCommittedSamples() - movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio]);
	lwmUInt32 audioBufferStartPoint = movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] - audioBuffer->GetNumCommittedSamples();

	if(audioBufferStartPoint > audioTime)
		return 0;	// Audio starts in the future
	if(movieState->streamSyncPeriods[lwmSTREAMTYPE_Audio] <= audioTime)
		return 0;	// Audio ends in the past

	// Audio starts immediately or in the past, trim any preceding samples and complete
	audioBuffer->SkipSamples(audioTime - audioBufferStartPoint);
	movieState->isAudioSynchronized = true;
	return 1;
}

LWMOVIE_API_LINK lwmUInt32 lwmMovieState_ReadAudioSamples(lwmMovieState *movieState, void *samples, lwmUInt32 numSamples)
{
	if(!movieState->isAudioSynchronized)
		return 0;
	if(movieState->movieInfo.audioStreamType == lwmAST_None)
		return 0;

	return movieState->GetAudioBuffer()->ReadCommittedSamples(samples, numSamples);
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
