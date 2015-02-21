#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "../common/lwmovie_coretypes.h"
#include "../lwmovie/lwmovie_constants.hpp"
#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"

using namespace lwmovie;
using namespace lwmovie::constants;

const char *mpegCode(lwmUInt32 code)
{
	switch(code)
	{
	case MPEG_SEQ_END_CODE:
		return "END";
	case MPEG_SEQ_START_CODE:
		return "SEQ_START";
	case MPEG_GOP_START_CODE:
		return "GOP_START";
	case MPEG_PICTURE_START_CODE:
		return "PICT_START";
	case MPEG_EXT_START_CODE:
		return "EXT";
	case MPEG_USER_START_CODE:
		return "USER";
	case MPEG_SEQUENCE_ERROR_CODE:
		return "ERROR";
	}
	if(code >= MPEG_SLICE_MIN_START_CODE && code <= MPEG_SLICE_MAX_START_CODE)
		return "SLICE";
	return "UNKNOWN";
}

struct videoTagLink
{
	lwmUInt32 code;
	lwmUInt64 fileLoc;
	lwmUInt64 packetSize;
	lwmUInt8 pictTypeCode;
	videoTagLink *next;
	videoTagLink *nextPict;
};

static bool ParseSeqStart(lwmVideoStreamInfo &vsi, lwmOSFile *mpegFile, videoTagLink *tagLink, void **pSeqData)
{
	lwmUInt8 *seqBytes = new lwmUInt8[static_cast<lwmLargeUInt>(tagLink->packetSize) - 4];
	mpegFile->Seek(tagLink->fileLoc + 4, lwmOSFile::SM_Start);
	mpegFile->ReadBytes(seqBytes, tagLink->packetSize - 4);

	vsi.videoWidth = static_cast<lwmUInt16>((seqBytes[0] << 4) | ((seqBytes[1] & 0xf0) >> 4));
	vsi.videoHeight = static_cast<lwmUInt16>(((seqBytes[1] & 0x0f) << 8) | (seqBytes[2] & 0xf0));

	lwmUInt8 frameRateCode = static_cast<lwmUInt8>(seqBytes[3] & 0x0f);

	switch(frameRateCode)
	{
	case 1:
		vsi.periodsPerSecondNum = 24000;
		vsi.periodsPerSecondDenom = 1001;
		break;
	case 2:
		vsi.periodsPerSecondNum = 24;
		vsi.periodsPerSecondDenom = 1;
		break;
	case 3:
		vsi.periodsPerSecondNum = 25;
		vsi.periodsPerSecondDenom = 1;
		break;
	case 4:
		vsi.periodsPerSecondNum = 30000;
		vsi.periodsPerSecondDenom = 1001;
		break;
	case 5:
		vsi.periodsPerSecondNum = 30;
		vsi.periodsPerSecondDenom = 1;
		break;
	case 6:
		vsi.periodsPerSecondNum = 50;
		vsi.periodsPerSecondDenom = 1;
		break;
	case 7:
		vsi.periodsPerSecondNum = 60000;
		vsi.periodsPerSecondDenom = 1001;
		break;
	case 8:
		vsi.periodsPerSecondNum = 60;
		vsi.periodsPerSecondDenom = 1;
		break;
	default:
		return false;
	};

	if(*pSeqData)
	{
		if(memcmp(*pSeqData, seqBytes, static_cast<size_t>(tagLink->packetSize) - 4))
		{
			fprintf(stderr, "FATAL ERROR: Multi-sequence MPEG with different sequence data");
			exit(1);
		}
		delete[] seqBytes;
	}
	else
		*pSeqData = seqBytes;

	return true;
}


int globalSync = 0;
int globalPacket = 0;
int globalOther = 0;

static void ParsePicture(lwmOSFile *mpegFile, videoTagLink *scanLink)
{
	lwmUInt8 pictInfo[2];
	mpegFile->Seek(scanLink->fileLoc + 4, lwmOSFile::SM_Start);
	mpegFile->ReadBytes(pictInfo, sizeof(pictInfo));
	scanLink->pictTypeCode = static_cast<lwmUInt8>((pictInfo[1] & 0x38) >> 3);
}

static void ConvertPacket(lwmEPacketType packetType, bool includeCode, videoTagLink *scanLink, lwmOSFile *mpegFile, lwmOSFile *outFile)
{
	lwmPacketHeader packetHeader;
	lwmPacketHeaderFull packetHeaderFull;
	packetHeader.packetTypeAndFlags = packetType;
	packetHeaderFull.packetSize = static_cast<lwmUInt32>(scanLink->packetSize - 4);
	if(includeCode)
		packetHeaderFull.packetSize += 1;
	lwmWritePlanToFile(packetHeader, outFile);
	lwmWritePlanToFile(packetHeaderFull, outFile);

	if(includeCode)
		mpegFile->Seek(scanLink->fileLoc + 3);
	else
		mpegFile->Seek(scanLink->fileLoc + 4);

	lwmUInt32 sizeRemaining = packetHeaderFull.packetSize;
	lwmUInt8 copyBuffer[1000];
	while(sizeRemaining)
	{
		lwmUInt32 copyBlockSize = sizeRemaining;
		if(sizeRemaining > sizeof(copyBuffer))
			copyBlockSize = sizeof(copyBuffer);
		mpegFile->ReadBytes(copyBuffer, copyBlockSize);
		outFile->WriteBytes(copyBuffer, copyBlockSize);
		sizeRemaining -= copyBlockSize;
	}

	globalOther += lwmPlanHandler<lwmPacketHeader>::SIZE;
	globalOther += lwmPlanHandler<lwmPacketHeaderFull>::SIZE;
	globalPacket += packetHeaderFull.packetSize;
}

void EmitFrameSync(lwmOSFile *outFile, lwmUInt32 frameNumber, bool isRandomAccess)
{
	lwmUInt8 packetData[lwmPlanHandler<lwmVideoSynchronizationPoint>::SIZE];
	
	lwmVideoSynchronizationPoint syncPoint;
	syncPoint.flags = lwmVideoSynchronizationPoint::EFlag_AlwaysSet;

	if(isRandomAccess)
		syncPoint.flags |= lwmVideoSynchronizationPoint::EFlag_RandomAccess;

	syncPoint.videoPeriod = frameNumber;
	lwmPlanHandler<lwmVideoSynchronizationPoint>::Write(syncPoint, packetData);

	lwmPacketHeader packetHeader;
	lwmPacketHeaderFull packetHeaderFull;
	packetHeaderFull.packetSize = sizeof(packetData);
	packetHeader.packetTypeAndFlags = lwmEPT_Video_Synchronization;

	lwmWritePlanToFile(packetHeader, outFile);
	lwmWritePlanToFile(packetHeaderFull, outFile);
	outFile->WriteBytes(packetData, sizeof(packetData));

	globalSync += 7;
}

void ConvertM1V(lwmOSFile *mpegFile, lwmOSFile *outFile)
{
	videoTagLink *linkHead = NULL;
	videoTagLink *linkTail = NULL;
	lwmUInt8 queue[4];
	mpegFile->ReadBytes(queue, 4);
	lwmUInt64 readTotal = 0;
	while(true)
	{
		if(queue[0] == 0 && queue[1] == 0 && queue[2] == 1)
		{
			videoTagLink *link = new videoTagLink();
			link->code = 0x100 | (queue[3]);
			link->fileLoc = mpegFile->FilePos() - 4;
			link->next = NULL;
			link->nextPict = NULL;
			if(linkTail)
			{
				linkTail->next = link;
				linkTail = link;
			}
			else
				linkHead = linkTail = link;
		}
		queue[0] = queue[1];
		queue[1] = queue[2];
		queue[2] = queue[3];
		readTotal++;
		if(!mpegFile->ReadBytes(queue + 3, 1))
			break;
	}

	lwmUInt64 vidFileSize = mpegFile->FilePos();

	lwmVideoStreamInfo vsi;
	videoTagLink *vsiLink = NULL;
	void *seqData = NULL;

	{
		// Determine tag packet sizes and establish links
		videoTagLink *prevLink = NULL;
		videoTagLink *prevPicture = NULL;
		lwmUInt64 lastOffset = 0;
		lwmUInt64 largestPacket = 0;
		for(videoTagLink *scanLink=linkHead;scanLink;scanLink=scanLink->next)
		{
			lwmUInt64 packetSize = scanLink->fileLoc - lastOffset;
			if(prevLink)
				prevLink->packetSize = packetSize;
			lastOffset = scanLink->fileLoc;

			if(scanLink->code == MPEG_PICTURE_START_CODE)
			{
				if(prevPicture != NULL)
				{
					while(prevPicture != scanLink)
					{
						prevPicture->nextPict = scanLink;
						prevPicture = prevPicture->next;
					}
				}
				ParsePicture(mpegFile, scanLink);
				prevPicture = scanLink;
			}

			prevLink = scanLink;
		}

		if(prevLink)
			prevLink->packetSize = vidFileSize - lastOffset;

		// Locate sequences (requires packet sizes)
		for(videoTagLink *scanLink=linkHead;scanLink;scanLink=scanLink->next)
		{
			if(scanLink->code == MPEG_SEQ_START_CODE)
			{
				ParseSeqStart(vsi, mpegFile, scanLink, &seqData);
				vsiLink = scanLink;
			}
		}
	}

	lwmMovieHeader movieHeader;
	movieHeader.audioStreamType = lwmAST_None;
	movieHeader.videoStreamType = lwmVST_M1V_Variant;
	movieHeader.numTOC = 0;

	// Write movie header
	lwmWritePlanToFile(movieHeader, outFile);

	// Write VSI
	lwmWritePlanToFile(vsi, outFile);

	// Write stream parameters
	ConvertPacket(lwmEPT_Video_StreamParameters, false, vsiLink, mpegFile, outFile);

	lwmUInt32 frameNumber = 0;

	bool hasFutureFrame = false;
	lwmUInt8 workingFrameCode = 0;

	for(videoTagLink *scanLink=linkHead;scanLink;scanLink=scanLink->next)
	{
		if(scanLink->code == MPEG_SEQ_START_CODE)
		{
			//ConvertPacket(lwmEPT_Video_StreamInfo, false, scanLink, mpegFile, outFile);
		}
		else if(scanLink->code == MPEG_PICTURE_START_CODE)
		{
			switch(scanLink->pictTypeCode)
			{
			case MPEG_I_TYPE:
				{
					bool isRandomAccess = (scanLink->nextPict == NULL) || (scanLink->nextPict->pictTypeCode == MPEG_P_TYPE) || (scanLink->nextPict->pictTypeCode == MPEG_I_TYPE);
					if(workingFrameCode == MPEG_I_TYPE)
						EmitFrameSync(outFile, ++frameNumber, isRandomAccess);
					else if(workingFrameCode == MPEG_P_TYPE)
						EmitFrameSync(outFile, ++frameNumber, isRandomAccess);
					else if(workingFrameCode == MPEG_B_TYPE)
					{
						EmitFrameSync(outFile, ++frameNumber, false);			// B frame
						EmitFrameSync(outFile, ++frameNumber, isRandomAccess);	// I/P frame
					}
				}
				break;
			case MPEG_P_TYPE:
				if(workingFrameCode == MPEG_I_TYPE)
					EmitFrameSync(outFile, ++frameNumber, false);
				else if(workingFrameCode == MPEG_P_TYPE)
					EmitFrameSync(outFile, ++frameNumber, false);
				else if(workingFrameCode == MPEG_B_TYPE)
				{
					EmitFrameSync(outFile, ++frameNumber, false);	// B frame
					EmitFrameSync(outFile, ++frameNumber, false);	// I/P frame
				}
				break;
			case MPEG_B_TYPE:
				// I/P frames preceding B don't emit frame syncs
				if(workingFrameCode == MPEG_B_TYPE)
					EmitFrameSync(outFile, ++frameNumber, false);
				break;
			default:
				fprintf(stderr, "D picture used, this is not supported!\n");
				// MUSTFIX - Recover
				break;
			};
			ConvertPacket(lwmEPT_Video_InlinePacket, true, scanLink, mpegFile, outFile);

			workingFrameCode = scanLink->pictTypeCode;
		}
		else if(scanLink->code >= MPEG_SLICE_MIN_START_CODE && scanLink->code <= MPEG_SLICE_MAX_START_CODE)
		{
			ConvertPacket(lwmEPT_Video_InlinePacket, true, scanLink, mpegFile, outFile);
		}
	}

	if(workingFrameCode == MPEG_B_TYPE)
	{
		EmitFrameSync(outFile, ++frameNumber, false);	// Flush B-frame
		EmitFrameSync(outFile, ++frameNumber, false);	// Flush I/P-frame
	}
	else if(workingFrameCode != 0)
		EmitFrameSync(outFile, ++frameNumber, false);	// Flush I/P-frame
}
