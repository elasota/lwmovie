#include "lwmovie_demux.hpp"
#include "lwmovie_fillable.hpp"
#include "lwmovie_package.hpp"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_recon_m1vsw.hpp"

namespace lwmovie
{
	class lwmVidStream;
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

	lwmSAllocator alloc;

	lwmovie::lwmVidStream *m1vDecoder;

	lwmIVideoReconstructor *videoReconstructor;
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
		movieState->m1vDecoder->DigestStreamParameters(packetData, packetSize, static_cast<lwmovie::lwmIM1VReconstructor*>(movieState->videoReconstructor));
		*outResult = lwmDIGEST_Worked;
		break;
	case lwmEPT_Video_InlinePacket:
		if(!movieState->m1vDecoder->DigestDataPacket(packetData, packetSize, static_cast<lwmovie::lwmIM1VReconstructor*>(movieState->videoReconstructor), outResult))
			*outResult = lwmDIGEST_Error;
		break;
	case lwmEPT_Video_Synchronization:
		movieState->videoReconstructor->WaitForFinish();
		*outResult = lwmDIGEST_VideoSync;
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
		movieState->m1vDecoder = new lwmovie::lwmVidStream(alloc, movieState->videoInfo.videoWidth, movieState->videoInfo.videoHeight, true);
		break;
	default:
		return false;
	};
	return true;
}

extern "C" lwmMovieState *lwmInitialize(const lwmSAllocator *alloc)
{
	lwmMovieState *movieState = static_cast<lwmMovieState *>(alloc->allocFunc(alloc->opaque, sizeof(lwmMovieState)));

	if(!movieState)
		return NULL;

	movieState->headerFillable.Init(movieState->pkgHeaderBytes);
	movieState->videoFillable.Init(movieState->videoInfoBytes);
	movieState->audioFillable.Init(movieState->audioInfoBytes);

	movieState->demuxState = lwmDEMUX_MovieHeader;
	movieState->packetDataBytesEscaped = NULL;
	movieState->packetDataBytes = NULL;

	movieState->alloc = *alloc;

	return movieState;
}

extern "C" void lwmFeedData(lwmMovieState *movieState, const void *inBytes, lwmUInt32 numBytes, lwmUInt32 *outResult, lwmUInt32 *outBytesDigested)
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

			if(InitDecoding(&movieState->alloc, movieState))
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


extern "C" void lwmGetVideoParameters(const lwmMovieState *movieState, lwmUInt32 *outWidth, lwmUInt32 *outHeight, lwmUInt32 *outPPSNum, lwmUInt32 *outPPSDenom, lwmUInt32 *outReconstructorType)
{
	*outWidth = movieState->videoInfo.videoWidth;
	*outHeight = movieState->videoInfo.videoHeight;
	*outPPSNum = movieState->videoInfo.periodsPerSecondNum;
	*outPPSDenom = movieState->videoInfo.periodsPerSecondDenom;

	switch(movieState->movieInfo.videoStreamType)
	{
	case lwmVST_M1V_Variant:
		*outReconstructorType = lwmRC_MPEG1Video;
		break;
	default:
		*outReconstructorType = lwmRC_Unknown;
	};
}


extern "C" void lwmCreateSoftwareVideoReconstructor(const lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height, lwmUInt32 reconstructorType, const lwmSWorkNotifier *workNotifier, lwmIVideoReconstructor **outReconstructor)
{
	switch(reconstructorType)
	{
	case lwmRC_MPEG1Video:
		{
			lwmovie::lwmCM1VSoftwareReconstructor *recon = new lwmovie::lwmCM1VSoftwareReconstructor();
			if(!recon->Initialize(alloc, width, height, workNotifier))
			{
				delete recon;
			}
			*outReconstructor = recon;
		}
		break;
	};
}

extern "C" void lwmSetVideoReconstructor(lwmMovieState *movieState, lwmIVideoReconstructor *recon)
{
	movieState->videoReconstructor = recon;
}

extern "C" void lwmReconParticipate(lwmIVideoReconstructor *recon)
{
	recon->Participate();
}

extern "C" void lwmReconGetChannel(lwmIVideoReconstructor *recon, lwmUInt32 channelNum, const lwmUInt8 **outPChannel, lwmUInt32 *outStride)
{
	recon->GetChannel(channelNum, outPChannel, outStride);
}
