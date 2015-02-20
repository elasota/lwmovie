#include <stdio.h>
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

static int audioCopied = 0;

static void CopyAudioPackets(audioLink **currentAudioBlock, lwmUInt32 audioPeriod, lwmOSFile *audioFile, lwmOSFile *outFile)
{
	audioLink *link = *currentAudioBlock;

	if(!link || link->syncPeriod > audioPeriod)
		return;
	
	lwmPacketHeader pktHeader;
	lwmPacketHeaderFull pktHeaderFull;
	lwmAudioSynchronizationPoint audioSync;

	while(link && link->syncPeriod < audioPeriod)
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
				audioCopied += pktHeaderFull.packetSize;
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

		link = link->next;
	}

	// Write the audio sync packet
	lwmWritePlanToFile(pktHeader, outFile);
	lwmWritePlanToFile(pktHeaderFull, outFile);
	lwmWritePlanToFile(audioSync, outFile);

	*currentAudioBlock = link;
}


void Mux(lwmUInt32 audioReadAhead, lwmOSFile *audioFile, lwmOSFile *videoFile, lwmOSFile *outFile)
{
	lwmOSFile *streamFiles[STREAM_SOURCE_COUNT];
	streamFiles[STREAM_SOURCE_VIDEO] = videoFile;
	streamFiles[STREAM_SOURCE_AUDIO] = audioFile;

	lwmMovieHeader pkgHeaders[STREAM_SOURCE_COUNT];
	lwmVideoStreamInfo videoStreamInfos[STREAM_SOURCE_COUNT];
	lwmAudioStreamInfo audioStreamInfos[STREAM_SOURCE_COUNT];

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
	{
		lwmMovieHeader outPkgHeader;
		outPkgHeader.audioStreamType = pkgHeaders[STREAM_SOURCE_AUDIO].audioStreamType;
		outPkgHeader.videoStreamType = pkgHeaders[STREAM_SOURCE_VIDEO].videoStreamType;
		outPkgHeader.numTOC = 0;
		outPkgHeader.largestPacketSize = 0;
		outPkgHeader.longestFrameReadahead = 0;
		lwmWritePlanToFile<lwmMovieHeader>(outPkgHeader, outFile);

		audioStreamInfos[STREAM_SOURCE_AUDIO].audioReadAhead = audioReadAhead;
		
		if(outPkgHeader.videoStreamType != lwmVST_None)
			lwmWritePlanToFile<lwmVideoStreamInfo>(videoStreamInfos[STREAM_SOURCE_VIDEO], outFile);
		if(outPkgHeader.audioStreamType != lwmAST_None)
			lwmWritePlanToFile<lwmAudioStreamInfo>(audioStreamInfos[STREAM_SOURCE_AUDIO], outFile);
	}

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
				lwmUInt32 audioPeriod = static_cast<lwmUInt32>(
					static_cast<lwmUInt64>(vidSync.videoPeriod - 1)
					* static_cast<lwmUInt64>(videoStreamInfos[STREAM_SOURCE_VIDEO].periodsPerSecondDenom
					* static_cast<lwmUInt64>(audioStreamInfos[STREAM_SOURCE_AUDIO].sampleRate)
					/ videoStreamInfos[STREAM_SOURCE_VIDEO].periodsPerSecondNum));
				//printf("Audio period: %i\n", audioPeriod);
				CopyAudioPackets(&currentAudioBlock, audioPeriod + audioReadAhead, audioFile, outFile);

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

	// Write back largest packet size
	{
		outFile->Seek(0);
		lwmMovieHeader pkgHeader;
		lwmReadPlanFromFile(pkgHeader, outFile);
		pkgHeader.largestPacketSize = largestPacketSize;
		pkgHeader.longestFrameReadahead = longestFrameReadahead;
		outFile->Seek(0);
		lwmWritePlanToFile(pkgHeader, outFile);
	}
}
