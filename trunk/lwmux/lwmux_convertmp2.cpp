#include <stdio.h>
#include <string.h>
#include "../lwmovie/lwmovie_package.hpp"
#include "lwmux_osfile.hpp"


static bool DecodeMP2Header(lwmAudioStreamInfo &asi, const void *data, bool &outIsPadded, bool &outHasChecksum, lwmUInt32 &outFrameSize)
{
	static const lwmUInt32 MP2_SAMPLERATE[2][3] =
	{
		{ 44100, 48000, 32000 },
		{ 22050, 2400,  1600  },
	};
	const lwmUInt16 MP2_BITRATE_KBPS[2][15] =
	{
		{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },
		{ 0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160 },
	};

	const lwmUInt8 *bytes = static_cast<const lwmUInt8 *>(data);
	lwmUInt32 header = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]);

	lwmUInt16 syncword = (header & 0xffe00000) >> 21;
	lwmUInt8 version = (header & 0x180000) >> 19;
	lwmUInt8 layer = (header & 0x60000) >> 17;
	lwmUInt8 protection = (header & 0x10000) >> 16;
	lwmUInt8 bitrateIndex = (header & 0xf000) >> 12;
	lwmUInt8 samplerateIndex = (header & 0xc00) >> 10;
	lwmUInt8 padding = (header & 0x200) >> 9;
	lwmUInt8 mode = (header & 0xc0) >> 6;

	if(syncword != 0x7ff		// Bad syncword
		|| version == 0			// MPEG 2.5, not supported
		|| version == 1			// Reserved
		|| layer != 2
		|| bitrateIndex == 0	// Free format, not supported
		|| bitrateIndex == 15	// Bad
		|| samplerateIndex == 3	// Reserved
		)
		return false;

	int mpegVersion;
	if(version == 2)
		mpegVersion = 1;
	else if(version == 3)
		mpegVersion = 0;

	int numChannels;
	if(mode != 3)
		numChannels = asi.speakerLayout = lwmSPEAKERLAYOUT_Stereo_LR;
	else
		numChannels = asi.speakerLayout = lwmSPEAKERLAYOUT_Mono;
	asi.audioReadAhead = 0;
	asi.startTimeSamples = 480;
	asi.sampleRate = MP2_SAMPLERATE[mpegVersion][samplerateIndex];

	lwmUInt32 bitrateKbps = MP2_BITRATE_KBPS[mpegVersion][bitrateIndex];
	
	// 1152 * 1000 / 8
	// totalBitrate * 1000 * 1152 / sampleRate
	lwmUInt32 frameBytes = (static_cast<lwmUInt32>(bitrateKbps) * (1000*1152/8) / asi.sampleRate) - 4;
	if(padding != 0)
		frameBytes++;
	if(protection == 0)
		frameBytes += 2;
	outFrameSize = frameBytes;

	outIsPadded = (padding != 0);
	outHasChecksum = (protection == 0);

	return true;
}

static lwmUInt32 ComputeEscapes(const lwmUInt8 *bytes, lwmUInt32 unescapedSize)
{
	lwmUInt32 escapedSize = unescapedSize;
	for(lwmUInt32 i=2;i<unescapedSize;i++)
	{
		if(bytes[i] == 1 && bytes[i-1] == 0 && bytes[i-2] == 0)
			escapedSize++;
	}
	return escapedSize;
}

static void GenerateEscapedBytes(lwmUInt8 *outBytes, const lwmUInt8 *inBytes, lwmUInt32 unescapedSize)
{
	outBytes[0] = inBytes[0];
	outBytes[1] = inBytes[1];
	outBytes += 2;
	for(lwmUInt32 i=2;i<unescapedSize;i++)
	{
		*outBytes++ = inBytes[i];
		if(inBytes[i] == 1 && inBytes[i-1] == 0 && inBytes[i-2] == 0)
			*outBytes++ = 0xfe;
	}
}

void ConvertMP2(lwmOSFile *mpegFile, lwmOSFile *outFile)
{
	lwmUInt32 mp2FrameSamples = 1152;
	lwmUInt32 emittedSamples = 0;

	bool firstFrame = true;
	while(true)
	{
		lwmUInt8 frameHeader[4];
		if(!mpegFile->ReadBytes(frameHeader, 4))
			break;
		lwmAudioStreamInfo asi;
		lwmUInt32 inFrameSize;
		lwmUInt32 outFrameSize;
		bool isPadded;
		bool hasChecksum;
		if(!DecodeMP2Header(asi, frameHeader, isPadded, hasChecksum, inFrameSize))
		{
			fprintf(stderr, "FATAL ERROR: MP2 frame decode failed");
			return;
		}

		if(firstFrame)
		{
			lwmMovieHeader pkgHeader;
			pkgHeader.videoStreamType = lwmVST_None;
			pkgHeader.audioStreamType = lwmAST_MP2;
			pkgHeader.numTOC = 0;
			pkgHeader.largestPacketSize = 0;
			pkgHeader.longestFrameReadahead = 0;

			lwmWritePlanToFile(pkgHeader, outFile);
			lwmWritePlanToFile(asi, outFile);
			firstFrame = false;
		}

		outFrameSize = inFrameSize;
		if(isPadded)
			outFrameSize--;
		if(hasChecksum)
			outFrameSize -= 2;

		lwmPacketHeader pktHeader;
		lwmPacketHeaderFull pktHeaderFull;
		pktHeaderFull.packetSize = 4 + outFrameSize;
		pktHeader.packetTypeAndFlags = lwmEPT_Audio_Frame;

		lwmUInt8 *packetBytes = new lwmUInt8[4 + inFrameSize];
		lwmUInt8 *escapedPacketBytes = NULL;
		memcpy(packetBytes, frameHeader, 4);

		// Add protection bit
		packetBytes[1] |= 0x01;
		// Strip pad bit
		packetBytes[2] &= 0xfd;

		if(hasChecksum)
			mpegFile->Seek(mpegFile->FilePos() + 2);
		mpegFile->ReadBytes(packetBytes + 4, outFrameSize);
		if(isPadded)
			mpegFile->Seek(mpegFile->FilePos() + 1);

		lwmUInt32 packetSizeEscaped = ComputeEscapes(packetBytes, 4 + outFrameSize);
		if(packetSizeEscaped != pktHeaderFull.packetSize)
		{
			escapedPacketBytes = new lwmUInt8[packetSizeEscaped];
			GenerateEscapedBytes(escapedPacketBytes, packetBytes, pktHeaderFull.packetSize);
			pktHeader.packetTypeAndFlags |= lwmPacketHeader::EFlag_Escaped;
			pktHeaderFull.packetSize = packetSizeEscaped;
		}

		lwmWritePlanToFile(pktHeader, outFile);
		lwmWritePlanToFile(pktHeaderFull, outFile);

		if(escapedPacketBytes)
		{
			outFile->WriteBytes(escapedPacketBytes, pktHeaderFull.packetSize);
			delete[] escapedPacketBytes;
		}
		else
			outFile->WriteBytes(packetBytes, pktHeaderFull.packetSize);

		delete[] packetBytes;

		// Write synchronization
		emittedSamples += mp2FrameSamples;

		{
			lwmAudioSynchronizationPoint syncPoint;
			syncPoint.audioPeriod = emittedSamples - 480;	// Offset for MP2 delay

			lwmPacketHeader syncPacket;
			lwmPacketHeaderFull syncPacketFull;
			syncPacketFull.packetSize = lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE;
			syncPacket.packetTypeAndFlags = lwmEPT_Audio_Synchronization;

			lwmWritePlanToFile(syncPacket, outFile);
			lwmWritePlanToFile(syncPacketFull, outFile);
			lwmWritePlanToFile(syncPoint, outFile);
		}
	}
}
