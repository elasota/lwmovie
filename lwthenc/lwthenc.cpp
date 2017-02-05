/*
* Copyright (C) 2017 Eric Lasota
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

#include <theora/theoraenc.h>

#include "../lwmux/lwmux_planio.hpp"
#include "../lwmovie/lwmovie_package.hpp"

lwmUInt32 g_lwmvPeriod = 0;

class CPacket
{
public:
	explicit CPacket(ogg_packet *packet);
	~CPacket();

	void Append(CPacket *next);

	size_t Size() const;
	const unsigned char *Data() const;

	CPacket *Next() const;
	ogg_int64_t GranulePos() const;
	bool IsEOS() const;
	bool IsBOS() const;

private:
	unsigned char *m_bytes;
	size_t m_size;
	CPacket *m_nextPacket;
	ogg_int64_t m_granulePos;
	bool m_isEOS;
	bool m_isBOS;
};

class CPacketCluster
{
public:
	CPacketCluster();
	~CPacketCluster();

	CPacket *First() const;
	void Append(CPacket *packet);
	void Clear();
	bool IsEmpty() const;
	size_t ComputeSize(bool isInitial) const;

	void Write(FILE *f, bool isInitial) const;

private:
	CPacket *m_firstPacket;
	CPacket *m_lastPacket;
};

CPacket::CPacket(ogg_packet *packet)
	: m_bytes(NULL)
	, m_granulePos(packet->granulepos)
	, m_isBOS(packet->b_o_s != 0)
	, m_isEOS(packet->e_o_s != 0)
	, m_nextPacket(NULL)
{
	m_size = static_cast<size_t>(packet->bytes);

	if (m_size > 0)
	{
		m_bytes = new unsigned char[m_size];
		memcpy(m_bytes, packet->packet, m_size);
	}
}

CPacket::~CPacket()
{
	if (m_bytes)
		delete[] m_bytes;
}

void CPacket::Append(CPacket *next)
{
	m_nextPacket = next;
}

size_t CPacket::Size() const
{
	return m_size;
}

const unsigned char *CPacket::Data() const
{
	return m_bytes;
}

CPacket *CPacket::Next() const
{
	return m_nextPacket;
}

ogg_int64_t CPacket::GranulePos() const
{
	return m_granulePos;
}

bool CPacket::IsEOS() const
{
	return m_isEOS;
}

bool CPacket::IsBOS() const
{
	return m_isBOS;
}

CPacketCluster::CPacketCluster()
	: m_firstPacket(NULL)
	, m_lastPacket(NULL)
{
}

CPacketCluster::~CPacketCluster()
{
	this->Clear();
}

CPacket *CPacketCluster::First() const
{
	return m_firstPacket;
}

void CPacketCluster::Append(CPacket *packet)
{
	if (m_firstPacket)
	{
		m_lastPacket->Append(packet);
		m_lastPacket = packet;
	}
	else
		m_firstPacket = m_lastPacket = packet;
}

void CPacketCluster::Clear()
{
	CPacket *nextPacket = m_firstPacket;
	while (nextPacket)
	{
		CPacket *packet = nextPacket;
		nextPacket = packet->Next();
		delete packet;
	}
	m_firstPacket = m_lastPacket = NULL;
}

bool CPacketCluster::IsEmpty() const
{
	return m_firstPacket == NULL;
}

size_t CPacketCluster::ComputeSize(bool isInitial) const
{
	size_t numPackets = 0;
	size_t totalSize = 0;
	for (CPacket *packet = m_firstPacket; packet; packet = packet->Next())
	{
		totalSize += packet->Size();
		totalSize += 4;		// Catalog size
	}

	if (!isInitial)
		totalSize += 8;		// Granule position

	totalSize += 1;	// Packet count + EOS flag

	return totalSize;
}

void CPacketCluster::Write(FILE *f, bool isInitial) const
{
	lwmUInt16 numPackets = 0;

	for (CPacket *packet = m_firstPacket; packet; packet = packet->Next())
		numPackets++;

	unsigned char buf[8];

	if (!isInitial)
	{
		lwmUInt64 granulePos = m_firstPacket->GranulePos();
		buf[0] = (granulePos >> 0) & 0xff;
		buf[1] = (granulePos >> 8) & 0xff;
		buf[2] = (granulePos >> 16) & 0xff;
		buf[3] = (granulePos >> 24) & 0xff;
		buf[4] = (granulePos >> 32) & 0xff;
		buf[5] = (granulePos >> 40) & 0xff;
		buf[6] = (granulePos >> 48) & 0xff;
		buf[7] = (granulePos >> 56) & 0xff;

		fwrite(buf, 8, 1, f);
	}

	if (m_lastPacket->IsEOS())
		numPackets |= 0x80;

	buf[0] = numPackets & 0xff;

	fwrite(buf, 1, 1, f);

	for (CPacket *packet = m_firstPacket; packet; packet = packet->Next())
	{
		lwmUInt32 size = static_cast<lwmUInt32>(packet->Size());
		buf[0] = (size >> 0) & 0xff;
		buf[1] = (size >> 8) & 0xff;
		buf[2] = (size >> 16) & 0xff;
		buf[3] = (size >> 24) & 0xff;
		fwrite(buf, 4, 1, f);
	}

	for (CPacket *packet = m_firstPacket; packet; packet = packet->Next())
		fwrite(packet->Data(), packet->Size(), 1, f);
}

void FlushClusterToFile(CPacketCluster *cluster, bool isInitial, FILE *f)
{
	size_t clusterSize = cluster->ComputeSize(isInitial);
	lwmOSFile osFile;
	osFile.Attach(f);

	{
		lwmPacketHeader packetHeader;
		lwmPacketHeaderFull packetHeaderFull;
		packetHeader.packetTypeAndFlags = isInitial ? lwmEPT_Video_StreamParameters : lwmEPT_Video_InlinePacket;
		packetHeaderFull.streamIndex = 0;
		packetHeaderFull.packetSize = static_cast<lwmUInt32>(clusterSize);
		lwmWritePlanToFile(packetHeader, &osFile);
		lwmWritePlanToFile(packetHeaderFull, &osFile);

		cluster->Write(f, isInitial);
	}

	if (!isInitial)
	{
		lwmPacketHeader packetHeader;
		lwmPacketHeaderFull packetHeaderFull;
		packetHeader.packetTypeAndFlags = lwmEPT_Video_Synchronization;
		packetHeaderFull.streamIndex = 0;
		packetHeaderFull.packetSize = lwmPlanHandler<lwmVideoSynchronizationPoint>::SIZE;
		lwmWritePlanToFile(packetHeader, &osFile);
		lwmWritePlanToFile(packetHeaderFull, &osFile);

		lwmVideoSynchronizationPoint syncPoint;
		syncPoint.flags = lwmVideoSynchronizationPoint::EFlag_AlwaysSet;
		syncPoint.videoPeriod = ++g_lwmvPeriod;
		lwmWritePlanToFile(syncPoint, &osFile);
	}

	cluster->Clear();
}


bool g_isPass1 = false;
bool g_isPass2 = false;
FILE *g_metricsFile = NULL;
char g_metricsData[1000];
size_t g_metricsDataStart = 0;
size_t g_metricsDataEnd = 0;

CPacketCluster g_currentCluster;

void DumpPacket(FILE *f, ogg_packet *pkt)
{
	fwrite(&pkt->bytes, sizeof(pkt->bytes), 1, f);
	fwrite(&pkt->b_o_s, sizeof(pkt->b_o_s), 1, f);
	fwrite(&pkt->e_o_s, sizeof(pkt->e_o_s), 1, f);
	fwrite(&pkt->granulepos, sizeof(pkt->granulepos), 1, f);
	fwrite(&pkt->packetno, sizeof(pkt->packetno), 1, f);
	fwrite(pkt->packet, pkt->bytes, 1, f);
}

void *myRealloc(void *ctx, void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void PushMetrics(th_enc_ctx *ctx, FILE *outFile)
{
	char *metricsData;
	int numMetrics = th_encode_ctl(ctx, TH_ENCCTL_2PASS_OUT, &metricsData, sizeof(metricsData));

	fwrite(metricsData, 1, numMetrics, outFile);
}

void PullMetrics(th_enc_ctx *ctx)
{
	size_t available = sizeof(g_metricsData) - g_metricsDataEnd;
	size_t numRead = (available == 0) ? 0 : fread(g_metricsData + g_metricsDataEnd, 1, available, g_metricsFile);
	g_metricsDataEnd += numRead;

	size_t numBytesBuffered = g_metricsDataEnd - g_metricsDataStart;

	while (true)
	{
		int numConsumed = th_encode_ctl(ctx, TH_ENCCTL_2PASS_IN, g_metricsData + g_metricsDataStart, numBytesBuffered);
		if (numConsumed > 0)
		{
			g_metricsDataStart += numConsumed;
			if (g_metricsDataStart == g_metricsDataEnd)
				g_metricsDataStart = g_metricsDataEnd = 0;
		}
		else
			break;
	}
}

void PushFrame(th_enc_ctx *ctx, th_ycbcr_buffer buffer, int isEOF, FILE *outFile)
{
	if (g_isPass2)
		PullMetrics(ctx);

	th_encode_ycbcr_in(ctx, buffer);

	if (g_isPass1)
		PushMetrics(ctx, outFile);

	while (true)
	{
		ogg_packet pkt;
		int result = th_encode_packetout(ctx, isEOF, &pkt);
		if (!result)
			break;

		if (!g_isPass1)
		{
			if (!g_currentCluster.IsEmpty())
			{
				if (g_currentCluster.First()->GranulePos() != pkt.granulepos)
					FlushClusterToFile(&g_currentCluster, false, outFile);
			}
			CPacket *packet = new CPacket(&pkt);
			g_currentCluster.Append(packet);
		}
	}
}


int main(int argc, const char **argv)
{
#ifdef _MSC_VER
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	FILE *inFile = stdin;
	FILE *outFile = stdout;

	bool isCBR = false;
	int cbrBitrate = 4000000;

	bool isVBR = true;
	int qualityLevel = 32;

	bool haveKeyrate = false;
	int keyrate = 0;

	bool haveSpeedLevel = false;
	int speedLevel = 0;

	bool haveRateBuffer = false;
	int rateBuffer;

	bool isFullRange = false;

	if (argc < 3)
	{
		fprintf(stderr, "lwthenc <infile> <outfile> [options]");
		return -1;
	}

	const char *paramInFile = argv[1];
	const char *paramOutFile = argv[2];

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

	for (int i = 3; i < argc; i++)
	{
		if (!strcmp(argv[i], "-b"))
		{
			i++;
			if (i == argc)
				return -1;
			isCBR = true;
			isVBR = false;
			cbrBitrate = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-q"))
		{
			i++;
			if (i == argc)
				return -1;
			isCBR = false;
			isVBR = true;
			qualityLevel = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-k"))
		{
			i++;
			if (i == argc)
				return -1;
			haveKeyrate = true;
			keyrate = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-speed"))
		{
			i++;
			if (i == argc)
				return -1;
			haveSpeedLevel = true;
			speedLevel = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-buffer"))
		{
			i++;
			if (i == argc)
				return -1;
			haveRateBuffer = true;
			rateBuffer = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-pass1"))
		{
			g_isPass1 = true;
			g_isPass2 = false;
		}
		else if (!strcmp(argv[i], "-pass2"))
		{
			i++;
			if (i == argc)
				return -1;
			g_isPass1 = false;
			g_isPass2 = true;
			g_metricsFile = fopen(argv[i], "rb");
		}
		else if (!strcmp(argv[i], "-full"))
			isFullRange = true;
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
				if (isFullRange)
				{
					ranges[0][0] = 0x20;
					ranges[0][1] = 0x1d6;
					ranges[1][0] = 0x20;
					ranges[1][1] = 0x1e0;
				}
				else
				{
					ranges[0][0] = 0;
					ranges[0][1] = 0x1ff;
					ranges[1][0] = 0;
					ranges[1][1] = 0x1ff;
				}
				isSubsampled = (tag == "C420p9");
			}
			else if (tag == "C420p10" || tag == "C444p10")
			{
				foundCSTag = true;
				if (isFullRange)
				{
					ranges[0][0] = 0x40;
					ranges[0][1] = 0x3ac;
					ranges[1][0] = 0x40;
					ranges[1][1] = 0xec0;
				}
				else
				{
					ranges[0][0] = 0;
					ranges[0][1] = 0x3ff;
					ranges[1][0] = 0;
					ranges[1][1] = 0x3ff;
				}
				isSubsampled = (tag == "C420p10");
			}
			else if (tag == "C420p12" || tag == "C444p12")
			{
				foundCSTag = true;
				if (isFullRange)
				{
					ranges[0][0] = 0x100;
					ranges[0][1] = 0xeb0;
					ranges[1][0] = 0x100;
					ranges[1][1] = 0xf00;
				}
				else
				{
					ranges[0][0] = 0;
					ranges[0][1] = 0xfff;
					ranges[1][0] = 0;
					ranges[1][1] = 0xfff;
				}
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
				fprintf(stderr, "lwthenc: Unusable frame rate");
				return -1;
			}
		}
		fprintf(stderr, "lwthenc: WARNING: Frame rate is higher-resolution than lwmovie supports.  Time distortion may occur.");
	}

	if (width > 65535 || height > 65535)
	{
		fprintf(stderr, "lwthenc: Dimension too big");
		return -1;
	}

	// Write movie header
	if (!g_isPass1)
	{
		lwmOSFile osFileWrapper;
		osFileWrapper.Attach(outFile);

		lwmMovieHeader movieHeader;
		movieHeader.audioStreamType = lwmAST_None;
		movieHeader.largestPacketSize = 0;
		movieHeader.longestFrameReadahead = 0;
		movieHeader.numTOC = 0;
		movieHeader.videoStreamType = lwmVST_Theora_Variant;
		lwmWritePlanToFile(movieHeader, &osFileWrapper);

		lwmVideoStreamInfo vsi;
		vsi.channelLayout = isFullRange ? lwmVIDEOCHANNELLAYOUT_YCbCr_JPEG : lwmVIDEOCHANNELLAYOUT_YCbCr_BT601;
		vsi.frameFormat = isSubsampled ? lwmFRAMEFORMAT_8Bit_420P_Planar : lwmFRAMEFORMAT_8Bit_3Channel_Planar;
		vsi.numReadWriteWorkFrames = 0;
		vsi.numWriteOnlyWorkFrames = 1;
		vsi.periodsPerSecondDenom = fpsDenom;
		vsi.periodsPerSecondNum = fpsNum;
		vsi.videoWidth = width;
		vsi.videoHeight = height;
		lwmWritePlanToFile(vsi, &osFileWrapper);
	}

	lwmLargeUInt nLumaSamples = width * height;
	lwmLargeUInt nChromaSamples = isSubsampled ? (((width + 1) / 2) * ((height + 1) / 2) * 2) : nLumaSamples;

	short *inputSamples = new short[nLumaSamples + nChromaSamples * 2];
	unsigned char *outputSamples = reinterpret_cast<unsigned char*>(inputSamples);

	// Read frames and encode
	lwmUInt32 expandedWidth = (width + 15) / 16 * 16;
	lwmUInt32 expandedHeight = (height + 15) / 16 * 16;

	ogg_allocator alloc;
	alloc.ctx = NULL;
	alloc.reallocfunc = myRealloc;

	th_info info;
	th_info_init(&info, &alloc);
	info.keyframe_granule_shift = 31;	// No keyframes by default
	info.frame_width = width;
	info.frame_height = height;
	info.pic_width = width;
	info.pic_height = height;
	info.pic_x = 0;
	info.pic_y = 0;
	info.colorspace = TH_CS_ITU_REC_470BG;
	info.pixel_fmt = isSubsampled ? TH_PF_420 : TH_PF_444;
	info.target_bitrate = isCBR ? cbrBitrate : 0;
	info.quality = isVBR ? qualityLevel : 0;
	info.fps_numerator = fpsNum;
	info.fps_denominator = fpsDenom;
	info.aspect_numerator = 1;
	info.aspect_denominator = 1;

	th_enc_ctx *ctx = th_encode_alloc(&info);

	// Configure encoder

	if (haveSpeedLevel)
	{
		int maxLevel;
		th_encode_ctl(ctx, TH_ENCCTL_GET_SPLEVEL_MAX, &maxLevel, sizeof(maxLevel));
		if (speedLevel > maxLevel)
			speedLevel = maxLevel;
		else if (speedLevel < 0)
			speedLevel = 0;

		th_encode_ctl(ctx, TH_ENCCTL_SET_SPLEVEL, &speedLevel, sizeof(speedLevel));
	}
	if (haveRateBuffer)
		th_encode_ctl(ctx, TH_ENCCTL_SET_RATE_BUFFER, &rateBuffer, sizeof(rateBuffer));
	if (haveKeyrate)
		th_encode_ctl(ctx, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &keyrate, sizeof(keyrate));

	th_comment comments;
	th_comment_init(&comments, &info.allocator);

	CPacketCluster cluster;
	while (true)
	{
		ogg_packet pkt;
		int result = th_encode_flushheader(ctx, &comments, &pkt);
		if (!result)
			break;

		if (!g_isPass1)
		{
			CPacket *packet = new CPacket(&pkt);
			g_currentCluster.Append(packet);
		}
	}

	if (!g_isPass1)
	{
		FlushClusterToFile(&g_currentCluster, true, outFile);
		g_currentCluster.Clear();
	}

	char frameTag[6];
	int frameNum = 0;

	if (g_isPass1)
		PushMetrics(ctx, outFile);

	th_ycbcr_buffer buffer;

	while (int frameTagSize = fread(frameTag, 1, 6, inFile))
	{
		if (frameNum != 0)
			PushFrame(ctx, buffer, 0, outFile);

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

		memset(&buffer, 0, sizeof(buffer));
		buffer[0].data = outputSamples;
		buffer[0].width = width;
		buffer[0].height = height;
		buffer[0].stride = width;

		buffer[1].data = outputSamples + sgSizes[0];
		buffer[1].width = chromaWidth;
		buffer[1].height = chromaHeight;
		buffer[1].stride = chromaWidth;

		buffer[2].data = outputSamples + sgSizes[0] + chromaPlaneSize;
		buffer[2].width = chromaWidth;
		buffer[2].height = chromaHeight;
		buffer[2].stride = chromaWidth;

		fprintf(stderr, "Encoding frame %i...\n", frameNum++);
		fflush(stderr);
	}

	if (frameNum != 0)
		PushFrame(ctx, buffer, 1, outFile);


	if (g_isPass1)
	{
		fseek(outFile, 0, SEEK_SET);
		PushMetrics(ctx, outFile);
	}
	else
	{
		if (!g_currentCluster.IsEmpty())
			FlushClusterToFile(&g_currentCluster, false, outFile);
	}

	th_encode_free(ctx);

	delete[] inputSamples;
	return 0;
}
