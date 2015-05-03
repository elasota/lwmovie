/*
* Copyright (c) 2015 Eric Lasota
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
#include <stdio.h>
#include <string.h>
#include "lwmux_osfile.hpp"
#include "lwmux_planio.hpp"
#include "../lwmovie/lwmovie_package.hpp"

struct audioLink
{
	lwmUInt64 packetStart;
	lwmUInt32 syncPeriod;
	audioLink *next;
};

struct StreamFileState
{
	lwmOSFile *osFile;
	lwmMovieHeader pkgHeader;
	lwmVideoStreamInfo videoStreamInfo;
	lwmAudioCommonInfo audioCommonInfo;
	lwmAudioStreamInfo audioStreamInfo;
	lwmUInt32 currentAudioEndPeriod;

	bool haveASP;
	audioLink *currentAudioBlock;
	
	audioLink *headAL;
	audioLink *tailAL;
};

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

static void CopyPacket(const lwmPacketHeader& packet, const lwmPacketHeaderFull& packetFull, lwmOSFile *inFile, lwmOSFile *outFile, lwmUInt8 outStreamIndex)
{
	if(packetFull.packetSize < 32768 && outStreamIndex == 0)
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
		lwmPacketHeaderFull packetFullCopy = packetFull;
		packetFullCopy.streamIndex = outStreamIndex;
		lwmWritePlanToFile(packet, outFile);
		lwmWritePlanToFile(packetFullCopy, outFile);
	}
	StreamCopy(inFile, outFile, packetFull.packetSize);
}

// Returns the new audio period
static lwmUInt32 CopyAudioPackets(StreamFileState *ss, audioLink **currentAudioBlock, lwmUInt32 startPeriod, lwmUInt32 minEndPeriod, lwmOSFile *audioFile, lwmOSFile *outFile, lwmUInt8 streamIndex, bool writeSync)
{
	audioLink *link = *currentAudioBlock;

	if(link == ss->tailAL || link->syncPeriod > minEndPeriod)
		return startPeriod;
	
	lwmPacketHeader pktHeader;
	lwmPacketHeaderFull pktHeaderFull;
	lwmAudioSynchronizationPoint audioSync;

	if(startPeriod >= minEndPeriod)
		return startPeriod;

	lwmUInt32 currentEndPeriod = startPeriod;
	
	while(link != ss->tailAL && currentEndPeriod < minEndPeriod)
	{
		// Copy up to the sync point
		audioFile->Seek(link->packetStart);
		while(true)
		{
			lwmReadPlanFromFile(pktHeader, audioFile);
			lwmReadPlanFromFile(pktHeaderFull, audioFile);

			// Don't copy stream parameters here, stream parameters are copied with the video parameters
			if(pktHeader.GetPacketType() == lwmEPT_Audio_Frame)
			{
				CopyPacket(pktHeader, pktHeaderFull, audioFile, outFile, streamIndex);
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
	if (writeSync)
	{
		lwmPacketHeaderCompact pktHeaderCompact;
		pktHeaderCompact.packetSize = static_cast<lwmUInt16>((pktHeaderFull.packetSize << 1) | 1);
		pktHeader.packetTypeAndFlags |= lwmPacketHeader::EFlag_Compact;

		lwmWritePlanToFile(pktHeader, outFile);
		lwmWritePlanToFile(pktHeaderCompact, outFile);
		lwmWritePlanToFile(audioSync, outFile);
	}

	*currentAudioBlock = link;

	return currentEndPeriod;
}

static void WriteHeaders(lwmMovieHeader *muxPkgHeader, lwmVideoStreamInfo *muxVideoInfo, lwmAudioCommonInfo *audioCommonInfo, lwmAudioStreamInfo *muxAudioInfo, lwmLargeUInt numAudioStreams, lwmOSFile *outFile)
{
	lwmWritePlanToFile<lwmMovieHeader>(*muxPkgHeader, outFile);
	if(muxPkgHeader->videoStreamType != lwmVST_None)
		lwmWritePlanToFile<lwmVideoStreamInfo>(*muxVideoInfo, outFile);
	if(numAudioStreams > 0)
	{
		lwmWritePlanToFile<lwmAudioCommonInfo>(*audioCommonInfo, outFile);
		for(lwmLargeUInt i=0;i<numAudioStreams;i++)
			lwmWritePlanToFile<lwmAudioStreamInfo>(muxAudioInfo[i], outFile);
	}
}

int Mux(lwmLargeUInt extraAudioReadAhead, lwmOSFile **audioFiles, lwmLargeUInt numAudioFiles, lwmOSFile *videoFile, lwmOSFile *outFile)
{
	lwmLargeUInt totalStreams = numAudioFiles + 1;
	lwmLargeUInt firstAudioStream = 1;
	lwmLargeUInt videoStreamIndex = 0;
	StreamFileState *streamFileStates = new StreamFileState[totalStreams];

	memset(streamFileStates, 0, sizeof(StreamFileState) * totalStreams);

	streamFileStates[videoStreamIndex].osFile = videoFile;

	for(lwmLargeUInt i=0;i<numAudioFiles;i++)
		streamFileStates[firstAudioStream+i].osFile = audioFiles[i];

	for(lwmLargeUInt i=0;i<totalStreams;i++)
	{
		lwmReadPlanFromFile<lwmMovieHeader>(streamFileStates[i].pkgHeader, streamFileStates[i].osFile);

		if(streamFileStates[i].pkgHeader.videoStreamType != lwmVST_None)
			lwmReadPlanFromFile<lwmVideoStreamInfo>(streamFileStates[i].videoStreamInfo, streamFileStates[i].osFile);

		if(streamFileStates[i].pkgHeader.audioStreamType != lwmAST_None)
		{
			lwmReadPlanFromFile<lwmAudioCommonInfo>(streamFileStates[i].audioCommonInfo, streamFileStates[i].osFile);
			if(streamFileStates[i].audioCommonInfo.numAudioStreams != 1)
			{
				fprintf(stderr, "Number of audio streams in audio source not 1");
				return -1;
			}
			
			lwmReadPlanFromFile<lwmAudioStreamInfo>(streamFileStates[i].audioStreamInfo, streamFileStates[i].osFile);
		}
	}

	if(streamFileStates[videoStreamIndex].pkgHeader.videoStreamType == lwmVST_None)
	{
		fprintf(stderr, "Video stream has no video");
		return -1;
	}
	if(streamFileStates[firstAudioStream].pkgHeader.audioStreamType == lwmAST_None)
	{
		fprintf(stderr, "Audio stream has no audio");
		return -1;
	}
	for(lwmLargeUInt i=1;i<numAudioFiles;i++)
	{
		if(streamFileStates[i].pkgHeader.audioStreamType != streamFileStates[firstAudioStream+i].pkgHeader.audioStreamType)
		{
			fprintf(stderr, "Audio streams have different stream types");
			return -1;
		}
	}

	// Write package headers
	lwmMovieHeader muxPkgHeader;
	lwmVideoStreamInfo muxVideoInfo;
	lwmAudioCommonInfo muxAudioCommonInfo;
	lwmAudioStreamInfo *muxAudioInfo = NULL;
	if(numAudioFiles > 0)
		muxAudioInfo = new lwmAudioStreamInfo[numAudioFiles];
	muxPkgHeader.videoStreamType = streamFileStates[videoStreamIndex].pkgHeader.videoStreamType;
	muxPkgHeader.audioStreamType = (numAudioFiles > 0) ? streamFileStates[firstAudioStream].pkgHeader.audioStreamType : lwmAST_None;
	WriteHeaders(&muxPkgHeader, &muxVideoInfo, &muxAudioCommonInfo, muxAudioInfo, numAudioFiles, outFile);

	lwmUInt32 largestPacketSize = 0;

	// Scan audio packets
	for(lwmLargeUInt i=0;i<numAudioFiles;i++)
	{
		StreamFileState *ss = streamFileStates + firstAudioStream + i;

		ss->headAL = ss->tailAL = new audioLink();
		ss->tailAL->packetStart = ss->osFile->FilePos();
		ss->tailAL->syncPeriod = 0;
		ss->tailAL->next = NULL;
		while(true)
		{
			lwmPacketHeader pktHeader;
			lwmPacketHeaderFull pktHeaderFull;

			if(!lwmReadPlanFromFile<lwmPacketHeader>(pktHeader, ss->osFile) ||
				!lwmReadPlanFromFile<lwmPacketHeaderFull>(pktHeaderFull, ss->osFile))
				break;

			if(pktHeaderFull.packetSize > largestPacketSize)
				largestPacketSize = pktHeaderFull.packetSize;

			lwmUInt64 nextPacket = ss->osFile->FilePos() + pktHeaderFull.packetSize;

			if(pktHeader.GetPacketType() == lwmEPT_Audio_Synchronization)
			{
				lwmAudioSynchronizationPoint syncPoint;
				lwmReadPlanFromFile<lwmAudioSynchronizationPoint>(syncPoint, ss->osFile);
				ss->tailAL->syncPeriod = syncPoint.audioPeriod;
				audioLink *newLink = new audioLink();
				newLink->packetStart = nextPacket;
				newLink->syncPeriod = 0;
				newLink->next = NULL;
				ss->tailAL->next = newLink;
				ss->tailAL = newLink;
			}
			else if(pktHeader.GetPacketType() == lwmEPT_Audio_StreamParameters)
			{
				ss->haveASP = true;
			}

			ss->osFile->Seek(nextPacket);
		}
		
		ss->currentAudioBlock = ss->headAL;
	}

	bool anyVideo = false;
	lwmUInt64 lastVideoSync = 0;
	lwmUInt32 longestFrameReadahead = 0;
	lwmUInt32 audioReadAhead = 0;

	// Dump everything
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
			CopyPacket(vidPacket, vidPacketFull, videoFile, outFile, 0);
			for(lwmLargeUInt i=0;i<numAudioFiles;i++)
			{
				StreamFileState *ss = streamFileStates + firstAudioStream + i;
				if(ss->haveASP)
				{
					ss->osFile->Seek(ss->headAL->packetStart, lwmOSFile::SM_Start);
					lwmPacketHeader audPacket;
					lwmPacketHeaderFull audPacketFull;
					if(!lwmReadPlanFromFile(audPacket, videoFile) ||
						!lwmReadPlanFromFile(audPacketFull, videoFile))
					{
						CopyPacket(audPacket, audPacketFull, ss->osFile, outFile, static_cast<lwmUInt8>(i));
					}
				}
			}
			break;
		case lwmEPT_Video_InlinePacket:
			CopyPacket(vidPacket, vidPacketFull, videoFile, outFile, 0);
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
				
				for(lwmLargeUInt i=0;i<numAudioFiles;i++)
				{
					StreamFileState *ss = streamFileStates + firstAudioStream + i;
					if(ss->currentAudioBlock)
					{
						lwmUInt32 ppsNum = streamFileStates[videoStreamIndex].videoStreamInfo.periodsPerSecondNum;
						lwmUInt32 ppsDenom = streamFileStates[videoStreamIndex].videoStreamInfo.periodsPerSecondDenom;
						lwmUInt32 sampleRate = ss->audioCommonInfo.sampleRate;

						lwmUInt32 targetAudioEndPeriod = static_cast<lwmUInt32>(((static_cast<lwmUInt64>(vidSync.videoPeriod + 1) * ppsDenom * sampleRate) + (ppsNum - 1)) / ppsNum) + extraAudioReadAhead;

						lwmUInt32 syncAudioPeriod = static_cast<lwmUInt32>(static_cast<lwmUInt64>(vidSync.videoPeriod - 1) * ppsDenom * sampleRate / ppsNum);

						//printf("Audio period: %i\n", audioPeriod);
						ss->currentAudioEndPeriod = CopyAudioPackets(ss, &ss->currentAudioBlock, ss->currentAudioEndPeriod, targetAudioEndPeriod, ss->osFile, outFile, static_cast<lwmUInt8>(i), (i == numAudioFiles - 1));
						lwmUInt32 frameReadAhead = ss->currentAudioEndPeriod - syncAudioPeriod;
						if(frameReadAhead > audioReadAhead)
							audioReadAhead = frameReadAhead;
					}
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
	muxPkgHeader.numTOC = 0;
	muxPkgHeader.largestPacketSize = largestPacketSize;
	muxPkgHeader.longestFrameReadahead = longestFrameReadahead;

	for(lwmLargeUInt i=0;i<numAudioFiles;i++)
		muxAudioInfo[i] = streamFileStates[firstAudioStream+i].audioStreamInfo;
	muxVideoInfo = streamFileStates[videoStreamIndex].videoStreamInfo;

	if(numAudioFiles > 0)
		muxAudioCommonInfo = streamFileStates[firstAudioStream].audioCommonInfo;
	muxAudioCommonInfo.numAudioStreams = static_cast<lwmUInt8>(numAudioFiles);
	muxAudioCommonInfo.audioReadAhead = audioReadAhead;

	outFile->Seek(0);
	WriteHeaders(&muxPkgHeader, &muxVideoInfo, &muxAudioCommonInfo, muxAudioInfo, numAudioFiles, outFile);

	return 0;
}
