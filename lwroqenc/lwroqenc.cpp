/*
* Copyright (C) 2015 Eric Lasota
*
* lwroqenc is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* lwroqenc is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE

#define _CRT_SECURE_NO_WARNINGS		// Shut up MSVC warnings

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <stdarg.h>

#include "../lwmux/lwmux_planio.hpp"
#include "../lwmovie/lwmovie_package.hpp"

#include "SB4Encoder.hpp"
#include "SB4Image.hpp"

enum ROQFileOutputFormat
{
	ROQFILEFORMAT_RoQ,
	ROQFILEFORMAT_LWIV,
};

ROQFileOutputFormat g_outputFormat = ROQFILEFORMAT_RoQ;
lwmUInt16 g_fpsNum, g_fpsDenom;
lwmUInt16 g_vidWidth, g_vidHeight;
lwmUInt32 g_lwmvPeriod = 0;

static void MyWrite(void *h, const void *bytes, lwmLargeUInt nBytes, SB4WriteType writeType)
{
	if (g_outputFormat == ROQFILEFORMAT_RoQ)
		fwrite(bytes, 1, nBytes, static_cast<FILE*>(h));
	else if (g_outputFormat == ROQFILEFORMAT_LWIV)
	{
		lwmOSFile osFileWrapper;
		osFileWrapper.Attach(static_cast<FILE*>(h));

		if (writeType == SB4WT_MovieHeader)
		{
			lwmMovieHeader movieHeader;
			movieHeader.audioStreamType = lwmAST_None;
			movieHeader.largestPacketSize = 0;
			movieHeader.longestFrameReadahead = 0;
			movieHeader.numTOC = 0;
			movieHeader.videoStreamType = lwmVST_RoQ;
			lwmWritePlanToFile(movieHeader, &osFileWrapper);

			lwmVideoStreamInfo vsi;
			vsi.channelLayout = lwmVIDEOCHANNELLAYOUT_RGB;
			vsi.frameFormat = lwmFRAMEFORMAT_8Bit_3Channel_Interleaved;
			vsi.numReadWriteWorkFrames = 2;
			vsi.numWriteOnlyWorkFrames = 0;
			vsi.periodsPerSecondDenom = g_fpsDenom;
			vsi.periodsPerSecondNum = g_fpsNum;
			vsi.videoWidth = g_vidWidth;
			vsi.videoHeight = g_vidHeight;
			lwmWritePlanToFile(vsi, &osFileWrapper);
		}
		else if (writeType == SB4WT_Frame)
		{
			{
				lwmPacketHeader packetHeader;
				lwmPacketHeaderFull packetHeaderFull;
				packetHeader.packetTypeAndFlags = lwmEPT_Video_InlinePacket;
				packetHeaderFull.streamIndex = 0;
				packetHeaderFull.packetSize = static_cast<lwmUInt32>(nBytes);
				lwmWritePlanToFile(packetHeader, &osFileWrapper);
				lwmWritePlanToFile(packetHeaderFull, &osFileWrapper);
				osFileWrapper.WriteBytes(bytes, static_cast<lwmUInt64>(nBytes));
			}

			{
				lwmPacketHeader packetHeader;
				lwmPacketHeaderFull packetHeaderFull;
				packetHeader.packetTypeAndFlags = lwmEPT_Video_Synchronization;
				packetHeaderFull.streamIndex = 0;
				packetHeaderFull.packetSize = lwmPlanHandler<lwmVideoSynchronizationPoint>::SIZE;
				lwmWritePlanToFile(packetHeader, &osFileWrapper);
				lwmWritePlanToFile(packetHeaderFull, &osFileWrapper);

				lwmVideoSynchronizationPoint syncPoint;
				syncPoint.flags = lwmVideoSynchronizationPoint::EFlag_AlwaysSet;
				syncPoint.videoPeriod = ++g_lwmvPeriod;
				lwmWritePlanToFile(syncPoint, &osFileWrapper);
			}
		}
	}
}

static void MyLog(void *h, const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);

	FILE *f = static_cast<FILE*>(h);
	vfprintf(f, fmt, argptr);
	fflush(f);
}

int main(int argc, const char **argv)
{
#ifdef _MSC_VER
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	FILE *inFile = stdin;
	FILE *outFile = stdout;

	if (argc != 9)
	{
		fprintf(stderr, "lwroqenc <infile> <outfile> <i bitrate> <p bitrate> <keyrate> <high threshold> <num threshold phases> <output format>");
		return -1;
	}

	const char *paramInFile = argv[1];
	const char *paramOutFile = argv[2];
	const char *paramIBitrate = argv[3];
	const char *paramPBitrate = argv[4];
	const char *paramKeyRate = argv[5];
	lwmUInt32 highThreshold = static_cast<lwmUInt32>(atoi(argv[6]));
	lwmUInt32 numThresholdPhases = static_cast<lwmUInt32>(atoi(argv[7]));

	if (!strcmp(argv[8], "roq"))
		g_outputFormat = ROQFILEFORMAT_RoQ;
	else if (!strcmp(argv[8], "lwiv"))
		g_outputFormat = ROQFILEFORMAT_LWIV;

	if (strcmp(paramInFile, "-"))
	{
		inFile = fopen(paramInFile, "rb");
		if (!inFile)
		{
			fprintf(stderr, "Could not open input file");
			return -1;
		}
	}

	if (strcmp(paramOutFile, "-"))
	{
		outFile = fopen(paramOutFile, "wb");
		if (!outFile)
		{
			fprintf(stderr, "Could not open output file");
			return -1;
		}
	}

	std::vector<std::string> headerTags;

	{
		std::string currentTag;
		while (int c = fgetc(inFile))
		{
			if (c == ' ' || c == '\n')
			{
				headerTags.push_back(currentTag);
				if (c == 0x0a)
					break;
				currentTag = "";
			}
			else
				currentTag += static_cast<char>(c);
		}
	}

	bool foundCSTag = false;

	unsigned int width = 0;
	unsigned int height = 0;

	int ranges[2][2];

	// Process header tags
	bool isSubsampled = false;
	lwmUInt64 fpsNum = 0;
	lwmUInt64 fpsDenom = 0;
	for (std::vector<std::string>::iterator it = headerTags.begin(), itEnd = headerTags.end(); it != itEnd; ++it)
	{
		std::string &tag = *it;
		if (tag[0] == 'C')
		{
			if (tag == "C420p9" || tag == "C444p9")
			{
				foundCSTag = true;
				ranges[0][0] = 0x20;
				ranges[0][1] = 0x1d6;
				ranges[1][0] = 0x20;
				ranges[1][1] = 0x1e0;
				isSubsampled = (tag == "C420p9");
			}
			else if (tag == "C420p10" || tag == "C444p10")
			{
				foundCSTag = true;
				ranges[0][0] = 0x40;
				ranges[0][1] = 0x3ac;
				ranges[1][0] = 0x40;
				ranges[1][1] = 0xec0;
				isSubsampled = (tag == "C420p10");
			}
			else if (tag == "C420p12" || tag == "C444p12")
			{
				foundCSTag = true;
				ranges[0][0] = 0x100;
				ranges[0][1] = 0xeb0;
				ranges[1][0] = 0x100;
				ranges[1][1] = 0xf00;
				isSubsampled = (tag == "C420p12");
			}
			else
			{
				fprintf(stderr, "ERROR: Color space %s not supported by lwrerange.  Use yuv420p9, yuv420p10, or yuv420p12.", tag.c_str());
				return -1;
			}
			if (isSubsampled)
				tag = "C420jpeg";
			else
				tag = "C444";
		}
		else if (tag[0] == 'I')
		{
			if (tag != "Ip")
			{
				fprintf(stderr, "ERROR: Only progressive scan is supported.");
				return -1;
			}
		}
		else if (tag[0] == 'X')
		{

			if (tag == "XYSCSS=420P12" ||
				tag == "XYSCSS=420P10" ||
				tag == "XYSCSS=420P9")
			{
				tag = "XYSCSS=420JPEG";
			}
			else if (tag == "XYSCSS=444P12" ||
				tag == "XYSCSS=444P10" ||
				tag == "XYSCSS=444P9")
			{
				tag = "XYSCSS=444";
			}
			else
			{
				fprintf(stderr, "ERROR: Color space %s not supported by lwroqenc.  Use yuv420p9, yuv420p10, or yuv420p12.", tag.c_str());
				return -1;
			}
		}
		else if (tag[0] == 'W')
			width = static_cast<unsigned int>(atoi(tag.c_str() + 1));
		else if (tag[0] == 'H')
			height = static_cast<unsigned int>(atoi(tag.c_str() + 1));
		else if (tag[0] == 'F')
		{
			size_t scanLoc = 1;
			while (tag[scanLoc] != ':')
				fpsNum = fpsNum * 10 + (tag[scanLoc++] - '0');
			scanLoc++;
			while (scanLoc != tag.length())
				fpsDenom = fpsDenom * 10 + (tag[scanLoc++] - '0');
		}
	}

	if (foundCSTag == false || width == 0 || height == 0 || fpsNum == 0 || fpsDenom == 0)
	{
		fprintf(stderr, "lwroqenc: Missing C, W, or H tag.\n");
		return -1;
	}

	if (fpsNum > 65535 || fpsDenom > 65535)
	{
		while (fpsNum > 65535 || fpsDenom > 65535)
		{
			fpsNum /= 2;
			fpsDenom /= 2;
			if (fpsNum == 0 || fpsDenom == 0)
			{
				fprintf(stderr, "lwroqenc: Unusable frame rate");
				return -1;
			}
		}
		fprintf(stderr, "lwroqenc: WARNING: Frame rate is higher-resolution than lwmovie supports.  Time distortion may occur.");
	}
	g_fpsNum = static_cast<lwmUInt16>(fpsNum);
	g_fpsDenom = static_cast<lwmUInt16>(fpsDenom);

	if (width > 65535 || height > 65535)
	{
		fprintf(stderr, "lwroqenc: Dimension too big");
		return -1;
	}
	g_vidWidth = static_cast<lwmUInt16>(width);
	g_vidHeight = static_cast<lwmUInt16>(height);

	lwmLargeUInt nLumaSamples = width * height;
	lwmLargeUInt nChromaSamples = isSubsampled ? (((width + 1) / 2) * ((height + 1) / 2) * 2) : nLumaSamples;

	short *inputSamples = new short[nLumaSamples + nChromaSamples * 2];
	unsigned char *outputSamples = reinterpret_cast<unsigned char*>(inputSamples);

	// Read frames and encode
	lwmUInt32 expandedWidth = (width + 15) / 16 * 16;
	lwmUInt32 expandedHeight = (height + 15) / 16 * 16;

	lwmUInt32 iFrameSize = static_cast<lwmUInt32>(static_cast<lwmUInt32>(atoi(paramIBitrate)) * fpsDenom / fpsNum / 8);
	lwmUInt32 pFrameSize = static_cast<lwmUInt32>(static_cast<lwmUInt32>(atoi(paramPBitrate)) * fpsDenom / fpsNum / 8);

	SB4RoQEncoder encoder;
	encoder.Init(expandedWidth, expandedHeight, static_cast<lwmUInt32>(atoi(paramKeyRate)), iFrameSize, pFrameSize);

	encoder.WriteFileHeader(outFile, MyWrite);

	char frameTag[6];
	int frameNum = 0;
	while (int frameTagSize = fread(frameTag, 1, 6, inFile))
	{
		if (frameTagSize != 6 || memcmp(frameTag, "FRAME\n", 6))
		{
			fprintf(stderr, "lwroqenc: Unexpected frame boundary\n");
			return -1;
		}

		unsigned int sgSizes[2];
		unsigned int chromaWidth = isSubsampled ? ((width + 1) / 2) : width;
		unsigned int chromaHeight = isSubsampled ? ((height + 1) / 2) : height;
		unsigned int chromaPlaneSize = chromaWidth * chromaHeight;
		sgSizes[0] = width * height;
		sgSizes[1] = chromaPlaneSize * 2;
		for (int sg = 0; sg<2; sg++)
		{
			unsigned int sgSize = sgSizes[sg];
			int sMin = ranges[sg][0];
			int sRange = ranges[sg][1] - ranges[sg][0];
			short *inputStart = (sg == 0) ? inputSamples : (inputSamples + sgSizes[0]);
			fread(inputStart, 2, sgSize, inFile);
			unsigned char *planeStart = (sg == 0) ? outputSamples : (outputSamples + sgSizes[0]);
			for (unsigned int i = 0; i<sgSize; i++)
			{
				int inSample = inputStart[i];
				int outSample = (inSample - sMin) * 255 / sRange;
				if (outSample < 0)
					planeStart[i] = 0;
				else if (outSample > 255)
					planeStart[i] = 255;
				else
					planeStart[i] = static_cast<unsigned char>(outSample);
			}
		}

		// Convert into a SB4 image
		SB4Image image;
		image.Init(expandedWidth, expandedHeight);
		for (lwmUInt32 p = 0; p < 3; p++)
		{
			const lwmUInt8 *inputBytes = NULL;
			if (p == 0)
				inputBytes = outputSamples;
			else if (p == 1)
				inputBytes = outputSamples + sgSizes[0];
			else if (p == 2)
				inputBytes = outputSamples + sgSizes[0] + chromaPlaneSize;

			for (lwmUInt32 y = 0; y < height; y++)
			{
				const lwmUInt8 *inputRow;
				if (p == 0 || !isSubsampled)
					inputRow = inputBytes + y * width;
				else
					inputRow = inputBytes + (y / 2) * chromaWidth;

				lwmUInt8 *outputRow = image.Plane(p)->Row(y);
				for (lwmUInt32 x = 0; x < width; x++)
				{
					if (p == 0 || !isSubsampled)
						outputRow[x] = inputRow[x];
					else
						outputRow[x] = inputRow[x / 2];
				}
				for (lwmUInt32 x = width; x < expandedWidth; x++)
					outputRow[x] = outputRow[x - 1];
			}
			for (lwmUInt32 y = height; y < expandedHeight; y++)
				memcpy(image.Plane(p)->Row(y), image.Plane(p)->Row(y - 1), expandedWidth);
		}
		fprintf(stderr, "Encoding frame %i...\n", frameNum++);
		encoder.EncodeFrame(&image, highThreshold, numThresholdPhases, outFile, MyWrite, stderr, MyLog);
		if (frameNum == 0)
			break;
	}
	delete[] inputSamples;
	return 0;
}
