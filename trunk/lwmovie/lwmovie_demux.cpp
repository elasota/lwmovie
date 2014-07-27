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
#include "lwmovie_demux.hpp"
#include "lwmovie_fillable.hpp"
#include "lwmovie_package.hpp"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_recon_m1vsw.hpp"

namespace lwmovie
{
	class lwmVidStream;
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

	lwmUInt32 streamSyncPeriods[lwmSTREAMTYPE_Count];

	lwmSWorkNotifier *videoDigestWorkNotifier;
	lwmIVideoReconstructor *videoReconstructor;
	lwmovie::lwmVidStream *m1vDecoder;
};

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

	switch(movieState->packetInfo.packetTypeAndFlags & ~(lwmPacketHeader::EFlag_All))
	{
	case lwmEPT_Video_StreamParameters:
		movieState->m1vDecoder->WaitForDigestFinish();
		movieState->videoReconstructor->WaitForFinish();
		movieState->m1vDecoder->DigestStreamParameters(packetData, packetSize);
		*outResult = lwmDIGEST_Worked;
		break;
	case lwmEPT_Video_InlinePacket:
		if(!movieState->m1vDecoder->DigestDataPacket(packetData, packetSize, outResult))
			*outResult = lwmDIGEST_Error;
		break;
	case lwmEPT_Video_Synchronization:
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
		break;
		/*
	case lwmEPT_Audio_StreamParameters:
		break;
	case lwmEPT_Audio_Frame:
		break;
	case lwmEPT_Audio_Synchronization:
		break;
		*/
	default:
		*outResult = lwmDIGEST_Error;
		break;
	};
}

static bool InitDecoding(lwmSAllocator *alloc, lwmMovieState *movieState)
{
	switch(movieState->movieInfo.videoStreamType)
	{
	case lwmVST_M1V_Variant:
		if(movieState->videoInfo.videoWidth > 4095)
			return false;
		if(movieState->videoInfo.videoHeight > 4095)
			return false;
		movieState->m1vDecoder = new lwmovie::lwmVidStream(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight, movieState, movieState->videoDigestWorkNotifier, ((movieState->userFlags) & lwmUSERFLAG_ThreadedDeslicer));
		break;
	default:
		return false;
	};
	return true;
}

extern "C" lwmMovieState *lwmCreateMovieState(lwmSAllocator *alloc, lwmUInt32 userFlags)
{
	lwmMovieState *movieState = static_cast<lwmMovieState *>(alloc->allocFunc(alloc, sizeof(lwmMovieState)));

	if(!movieState)
		return NULL;

	movieState->headerFillable.Init(movieState->pkgHeaderBytes);
	movieState->videoFillable.Init(movieState->videoInfoBytes);
	movieState->audioFillable.Init(movieState->audioInfoBytes);

	movieState->demuxState = lwmDEMUX_MovieHeader;
	movieState->packetDataBytesEscaped = NULL;
	movieState->packetDataBytes = NULL;
	movieState->userFlags = userFlags;

	movieState->videoDigestWorkNotifier = NULL;

	movieState->alloc = alloc;

	return movieState;
}

extern "C" void lwmMovieState_FeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested)
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
			movieState->packetDataBytesEscaped = new lwmUInt8[movieState->movieInfo.largestPacketSize];
			movieState->packetDataBytes = new lwmUInt8[movieState->movieInfo.largestPacketSize];
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
	};
}


extern "C" int lwmMovieState_GetStreamParameterU32(const lwmMovieState *movieState, lwmUInt32 streamType, lwmUInt32 streamParameterU32, lwmUInt32 *outValue)
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
				return 0;
			};
		}
	case lwmSTREAMTYPE_Audio:
		{
		}
	};
	return 0;
}


extern "C" lwmIVideoReconstructor *lwmCreateSoftwareVideoReconstructor(lwmMovieState *movieState, lwmSAllocator *alloc, lwmUInt32 reconstructorType, lwmSVideoFrameProvider *frameProvider)
{
	switch(reconstructorType)
	{
	case lwmRC_MPEG1Video:
		{
			// TODO MUSTFIX: Use the right allocator
			lwmovie::lwmCM1VSoftwareReconstructor *recon = new lwmovie::lwmCM1VSoftwareReconstructor();
			if(!recon->Initialize(alloc, frameProvider, movieState))
			{
				delete recon;
				recon = NULL;
			}
			return recon;
		}
		break;
	};
	return NULL;
}

extern "C" void lwmMovieState_SetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon)
{
	movieState->videoReconstructor = recon;
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->SetReconstructor(recon);
}

extern "C" void lwmMovieState_SetVideoDigestWorkNotifier(lwmMovieState *movieState, lwmSWorkNotifier *videoDigestWorkNotifier)
{
	movieState->videoDigestWorkNotifier = videoDigestWorkNotifier;
}

extern "C" void lwmFlushProfileTags(lwmMovieState *movieState, lwmCProfileTagSet *tagSet)
{
	if(movieState->videoReconstructor)
		movieState->videoReconstructor->FlushProfileTags(tagSet);
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->FlushProfileTags(tagSet);
}

extern "C" void lwmVideoRecon_SetWorkNotifier(lwmIVideoReconstructor *recon, lwmSWorkNotifier *workNotifier)
{
	recon->SetWorkNotifier(workNotifier);
}

extern "C" void lwmMovieState_VideoDigestParticipate(lwmMovieState *movieState)
{
	if(movieState->m1vDecoder)
		movieState->m1vDecoder->Participate();
}

extern "C" void lwmVideoRecon_Participate(lwmIVideoReconstructor *recon)
{
	recon->Participate();
}

extern "C" lwmUInt32 lwmVideoRecon_GetWorkFrameIndex(const lwmIVideoReconstructor *recon)
{
	return recon->GetWorkFrameIndex();
}
