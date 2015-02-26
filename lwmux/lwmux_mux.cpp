#include <stdio.h>
#include <string.h>
#include "lwmux_osfile.hpp"
#include "../lwmovie/lwmovie_package.hpp"

enum
{
	STREAM_SOURCE_VIDEO,
	STREAM_SOURCE_AUDIO,

	STREAM_SOURCE_COUNT,
};

struct audioLink
{
	lwmUInt64 packetStart;
	lwmUInt32 syncPeriod;
	audioLink *next;
};

static audioLink *headAL = NULL;
static audioLink *tailAL = NULL;

static void StreamCopy(lwmOSFile *inFile, lwmOSFile *outFile, lwmUInt32 numBytes)
{
	lwmUInt8 copyBuffer[1000];
	lwmUInt32 remaining = numBytes;
	while(remaining)
	{
		lwmUInt32 chunkSize = remaining;
		if(remaining > sizeof(copyBuffer))
			chunkSize = sizeof(copyBuffer);
		inFile->ReadBytes(copyBuffer, chunkSize);
		outFile->WriteBytes(copyBuffer, chunkSize);
		remaining -= chunkSize;
	}
}

static void CopyPacket(const lwmPacketHeader& packet, const lwmPacketHeaderFull& packetFull, lwmOSFile *inFile, lwmOSFile *outFile)
{
	if(packetFull.packetSize < 32768)
	{
		lwmPacketHeader compactedHeader(packet);
		lwmPacketHeaderCompact cptHeader;
		compactedHeader.packetTypeAndFlags |= lwmPacketHeader::EFlag_Compact;
		cptHeader.packetSize = (static_cast<lwmUInt16>(packetFull.packetSize << 1) | 1);
		lwmWritePlanToFile(compactedHeader, outFile);
		lwmWritePlanToFile(cptHeader, outFile);
	}
	else
	{
		lwmWritePlanToFile(packet, outFile);
		lwmWritePlanToFile(packetFull, outFile);
	}
	StreamCopy(inFile, outFile, packetFull.packetSize);
}

// Returns the new audio period
static lwmUInt32 CopyAudioPackets(audioLink **currentAudioBlock, lwmUInt32 startPeriod, lwmUInt32 minEndPeriod, lwmOSFile *audioFile, lwmOSFile *outFile)
{
	audioLink *link = *currentAudioBlock;

	if(link == tailAL || link->syncPeriod > minEndPeriod)
		return startPeriod;
	
	lwmPacketHeader pktHeader;
	lwmPacketHeaderFull pktHeaderFull;
	lwmAudioSynchronizationPoint audioSync;

	if(startPeriod >= minEndPeriod)
		return startPeriod;

	lwmUInt32 currentEndPeriod = startPeriod;
	
	while(link != tailAL && currentEndPeriod < minEndPeriod)
	{
		// Copy up to the sync point
		audioFile->Seek(link->packetStart);
		while(true)
		{
			lwmReadPlanFromFile(pktHeader, audioFile);
			lwmReadPlanFromFile(pktHeaderFull, audioFile);

			if(pktHeader.GetPacketType() == lwmEPT_Audio_Frame ||
				pktHeader.GetPacketType() == lwmEPT_Audio_StreamParameters)
			{
				CopyPacket(pktHeader, pktHeaderFull, audioFile, outFile);
			}
			else if(pktHeader.GetPacketType() == lwmEPT_Audio_Synchronization)
			{
				lwmReadPlanFromFile(audioSync, audioFile);
				break;
			}
			else
				audioFile->Seek(audioFile->FilePos() + pktHeaderFull.packetSize);
		}

		currentEndPeriod = link->syncPeriod;

		link = link->next;
	}

	// Write the audio sync packet
	lwmWritePlanToFile(pktHeader, outFile);
	lwmWritePlanToFile(pktHeaderFull, outFile);
	lwmWritePlanToFile(audioSync, outFile);

	*currentAudioBlock = link;

	return currentEndPeriod;
}

static void WriteHeaders(lwmMovieHeader *muxPkgHeader, lwmVideoStreamInfo *muxVideoInfo, lwmAudioStreamInfo *muxAudioInfo, lwmOSFile *outFile)
{
	lwmWritePlanToFile<lwmMovieHeader>(*muxPkgHeader, outFile);
	if(muxPkgHeader->videoStreamType != lwmVST_None)
		lwmWritePlanToFile<lwmVideoStreamInfo>(*muxVideoInfo, outFile);
	if(muxPkgHeader->audioStreamType != lwmAST_None)
		lwmWritePlanToFile<lwmAudioStreamInfo>(*muxAudioInfo, outFile);
}

void Mux(lwmUInt32 extraAudioReadAhead, lwmOSFile *audioFile, lwmOSFile *videoFile, lwmOSFile *outFile)
{
	lwmOSFile *streamFiles[STREAM_SOURCE_COUNT];
	streamFiles[STREAM_SOURCE_VIDEO] = videoFile;
	streamFiles[STREAM_SOURCE_AUDIO] = audioFile;

	lwmMovieHeader pkgHeaders[STREAM_SOURCE_COUNT];
	lwmVideoStreamInfo videoStreamInfos[STREAM_SOURCE_COUNT];
	lwmAudioStreamInfo audioStreamInfos[STREAM_SOURCE_COUNT];
	
	lwmMovieHeader muxPkgHeader;
	lwmVideoStreamInfo muxVideoInfo;
	lwmAudioStreamInfo muxAudioInfo;

	memset(&muxPkgHeader, 0, sizeof(muxPkgHeader));
	memset(&muxVideoInfo, 0, sizeof(muxVideoInfo));
	memset(&muxAudioInfo, 0, sizeof(muxAudioInfo));

	for(int i=0;i<STREAM_SOURCE_COUNT;i++)
	{
		if(streamFiles[i] == NULL)
		{
			pkgHeaders[i].audioStreamType = lwmAST_None;
			pkgHeaders[i].videoStreamType = lwmVST_None;
			pkgHeaders[i].largestPacketSize = 0;
			pkgHeaders[i].longestFrameReadahead = 0;
			pkgHeaders[i].numTOC = 0;
		}
		else
		{
			lwmReadPlanFromFile<lwmMovieHeader>(pkgHeaders[i], streamFiles[i]);

			if(pkgHeaders[i].videoStreamType != lwmVST_None)
				lwmReadPlanFromFile<lwmVideoStreamInfo>(videoStreamInfos[i], streamFiles[i]);

			if(pkgHeaders[i].audioStreamType != lwmAST_None)
				lwmReadPlanFromFile<lwmAudioStreamInfo>(audioStreamInfos[i], streamFiles[i]);
		}
	}

	// Write package headers
	muxPkgHeader.videoStreamType = streamFiles[STREAM_SOURCE_VIDEO] ? pkgHeaders[STREAM_SOURCE_VIDEO].videoStreamType : lwmVST_None;
	muxPkgHeader.audioStreamType = streamFiles[STREAM_SOURCE_AUDIO] ? pkgHeaders[STREAM_SOURCE_AUDIO].audioStreamType : lwmAST_None;
	WriteHeaders(&muxPkgHeader, &muxVideoInfo, &muxAudioInfo, outFile);


	lwmUInt32 largestPacketSize = 0;

	// Scan audio packets
	if(audioFile)
	{
		headAL = tailAL = new audioLink();
		tailAL->packetStart = audioFile->FilePos();
		tailAL->syncPeriod = 0;
		tailAL->next = NULL;
		while(true)
		{
			lwmPacketHeader pktHeader;
			lwmPacketHeaderFull pktHeaderFull;

			if(!lwmReadPlanFromFile<lwmPacketHeader>(pktHeader, audioFile) ||
				!lwmReadPlanFromFile<lwmPacketHeaderFull>(pktHeaderFull, audioFile))
				break;

			if(pktHeaderFull.packetSize > largestPacketSize)
				largestPacketSize = pktHeaderFull.packetSize;

			lwmUInt64 nextPacket = audioFile->FilePos() + pktHeaderFull.packetSize;

			if(pktHeader.GetPacketType() == lwmEPT_Audio_Synchronization)
			{
				lwmAudioSynchronizationPoint syncPoint;
				lwmReadPlanFromFile<lwmAudioSynchronizationPoint>(syncPoint, audioFile);
				tailAL->syncPeriod = syncPoint.audioPeriod;
				audioLink *newLink = new audioLink();
				newLink->packetStart = nextPacket;
				newLink->syncPeriod = 0;
				newLink->next = NULL;
				tailAL->next = newLink;
				tailAL = newLink;
			}

			audioFile->Seek(nextPacket);
		}
	}

	bool anyVideo = false;
	lwmUInt64 lastVideoSync = 0;
	lwmUInt32 longestFrameReadahead = 0;
	lwmUInt32 audioReadAhead = 0;
	lwmUInt32 currentAudioEndPeriod = 0;

	// Dump everything
	audioLink *currentAudioBlock = headAL;
	while(true)
	{
		lwmPacketHeader vidPacket;
		lwmPacketHeaderFull vidPacketFull;
		if(!lwmReadPlanFromFile(vidPacket, videoFile) ||
			!lwmReadPlanFromFile(vidPacketFull, videoFile))
			break;

		if(vidPacketFull.packetSize > largestPacketSize)
			largestPacketSize = vidPacketFull.packetSize;

		switch(vidPacket.GetPacketType())
		{
		case lwmEPT_Video_StreamParameters:
		case lwmEPT_Video_InlinePacket:
			CopyPacket(vidPacket, vidPacketFull, videoFile, outFile);
			break;
		case lwmEPT_Video_Synchronization:
			{
				lwmUInt64 currentPos = outFile->FilePos();

				if(anyVideo)
				{
					lwmUInt64 posDiff = currentPos - lastVideoSync;
					if(posDiff > longestFrameReadahead)
						longestFrameReadahead = static_cast<lwmUInt32>(posDiff);
				}

				lastVideoSync = currentPos;
				anyVideo = true;

				lwmVideoSynchronizationPoint vidSync;
				lwmReadPlanFromFile(vidSync, videoFile);

				if(currentAudioBlock)
				{
					lwmUInt32 ppsNum = videoStreamInfos[STREAM_SOURCE_VIDEO].periodsPerSecondNum;
					lwmUInt32 ppsDenom = videoStreamInfos[STREAM_SOURCE_VIDEO].periodsPerSecondDenom;
					lwmUInt32 sampleRate = audioStreamInfos[STREAM_SOURCE_AUDIO].sampleRate;

					lwmUInt32 targetAudioEndPeriod = static_cast<lwmUInt32>(((static_cast<lwmUInt64>(vidSync.videoPeriod + 1) * ppsDenom * sampleRate) + (ppsNum - 1)) / ppsNum) + extraAudioReadAhead;

					lwmUInt32 syncAudioPeriod = static_cast<lwmUInt32>(static_cast<lwmUInt64>(vidSync.videoPeriod - 1) * ppsDenom * sampleRate / ppsNum);

					//printf("Audio period: %i\n", audioPeriod);
					currentAudioEndPeriod = CopyAudioPackets(&currentAudioBlock, currentAudioEndPeriod, targetAudioEndPeriod, audioFile, outFile);
					lwmUInt32 frameReadAhead = currentAudioEndPeriod - syncAudioPeriod;
					if(frameReadAhead > audioReadAhead)
						audioReadAhead = frameReadAhead;
				}

				lwmPacketHeaderCompact compactHeader;
				vidPacket.packetTypeAndFlags |= lwmPacketHeader::EFlag_Compact;
				compactHeader.packetSize = static_cast<lwmUInt16>(vidPacketFull.packetSize << 1) | 1;
				lwmWritePlanToFile(vidPacket, outFile);
				lwmWritePlanToFile(compactHeader, outFile);
				lwmWritePlanToFile(vidSync, outFile);
			}
			break;
		};
	}

	// Write back header info
	muxPkgHeader.audioStreamType = pkgHeaders[STREAM_SOURCE_AUDIO].audioStreamType;
	muxPkgHeader.videoStreamType = pkgHeaders[STREAM_SOURCE_VIDEO].videoStreamType;
	muxPkgHeader.numTOC = 0;
	muxPkgHeader.largestPacketSize = largestPacketSize;
	muxPkgHeader.longestFrameReadahead = longestFrameReadahead;

	muxAudioInfo = audioStreamInfos[STREAM_SOURCE_AUDIO];
	muxVideoInfo = videoStreamInfos[STREAM_SOURCE_VIDEO];
	muxAudioInfo.audioReadAhead = audioReadAhead;

	outFile->Seek(0);
	WriteHeaders(&muxPkgHeader, &muxVideoInfo, &muxAudioInfo, outFile);
}
