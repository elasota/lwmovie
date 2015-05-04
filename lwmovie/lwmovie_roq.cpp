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
#include <string.h>

#include "lwmovie_roq.hpp"
#include "lwmovie_recon.hpp"
#include "lwmovie_recon_roqsw.hpp"
#include "lwmovie.h"

#ifdef _MSC_VER
#pragma intrinsic(memcpy)
#endif

namespace lwmovie
{
	namespace roq
	{
		const lwmLargeUInt PACKET_HEADER_SIZE = 8;
		const lwmUInt16 MARKER_CODEBOOK = 0x1002;
		const lwmUInt16 MARKER_VIDEO = 0x1011;

		enum
		{
			BLOCKMARK_SKIP,
			BLOCKMARK_MOTION,
			BLOCKMARK_VECTOR,
			BLOCKMARK_QUAD
		};
	}
}

lwmovie::roq::CVideoDecoder::CVideoDecoder(lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height)
	: m_alloc(alloc)
	, m_width(width)
	, m_height(height)
	, m_recon(NULL)
{
	memset(m_cb2, 0, sizeof(m_cb2));
	memset(m_cb4, 0, sizeof(m_cb4));
	memset(m_cb2double, 0, sizeof(m_cb2double));
	memset(m_cb4complete, 0, sizeof(m_cb4complete));
}

bool lwmovie::roq::CVideoDecoder::DigestDataPacket(const void *packetData, lwmLargeUInt packetSize, lwmUInt32 *outResult)
{
	if (packetSize < 8)
		return false;

	m_recon->WaitForFinish();

	if (!ProcessFrame(packetData, packetSize))
	{
		*outResult = lwmDIGEST_Error;
		return false;
	}
	else
	{
		*outResult = lwmDIGEST_Worked;
		return true;
	}
}

void lwmovie::roq::CVideoDecoder::SetReconstructor(lwmIVideoReconstructor *videoRecon)
{
	m_recon = static_cast<lwmovie::roq::CSoftwareReconstructor*>(videoRecon);
}

void lwmovie::roq::CVideoDecoder::ParseHeader(const void *headerData, lwmUInt16 &outMarker, lwmUInt32 &outSize, lwmUInt16 &outArgument)
{
	const lwmUInt8 *headerBytes = static_cast<const lwmUInt8 *>(headerData);
	outMarker = (headerBytes[0] | (headerBytes[1] << 8));
	outSize = (headerBytes[2] | (headerBytes[3] << 8) | (headerBytes[4] << 16) | (headerBytes[5] << 24));
	outArgument = (headerBytes[6] | (headerBytes[7] << 8));

}

bool lwmovie::roq::CVideoDecoder::ProcessFrame(const void *packetData, lwmLargeUInt packetSize)
{
	bool reachedVideo = false;

	while (packetSize > 0)
	{
		const lwmUInt8 *packetBytes = static_cast<const lwmUInt8 *>(packetData);
		if (packetSize < PACKET_HEADER_SIZE)
			return false;	// Overrun
		lwmUInt16 subPacketMarker;
		lwmUInt32 subPacketSize;
		lwmUInt16 subPacketArg;
		this->ParseHeader(packetData, subPacketMarker, subPacketSize, subPacketArg);

		packetSize -= PACKET_HEADER_SIZE;
		packetBytes += PACKET_HEADER_SIZE;
		if (subPacketSize > packetSize)
			return false;	// Overrun

		if (subPacketMarker == MARKER_CODEBOOK)
		{
			if (!ParseCodebooks(packetBytes, subPacketSize, subPacketArg))
				return false;
		}
		else if (subPacketMarker == MARKER_VIDEO)
		{
			if (reachedVideo)
				return false;	// Multiple video frames
			reachedVideo = true;
			if (!ParseVideoFrame(packetBytes, subPacketSize, subPacketArg))
				return false;
		}

		packetSize -= subPacketSize;
		packetBytes += subPacketSize;
		packetData = packetBytes;
	}

	return reachedVideo;
}

bool lwmovie::roq::CVideoDecoder::ParseCodebooks(const void *packetData, lwmLargeUInt packetSize, lwmUInt16 packetArg)
{
	Codebook2 *cb2 = m_cb2;
	Codebook4 *cb4 = m_cb4;
	Codebook4Complete *cb4complete = m_cb4complete;
	Codebook2Double *cb2double = m_cb2double;

	lwmFastUInt16 blockCount2 = (packetArg >> 8) & 0xff;
	lwmFastUInt16 blockCount4 = packetArg & 0xff;
	if (blockCount2 == 0)
		blockCount2 = 256;
	if (blockCount4 == 0 && static_cast<lwmLargeUInt>(blockCount2) * 6 < packetSize)
		blockCount4 = 256;

	if (packetSize != blockCount2 * 6 + blockCount4 * 4)
		return false;

	const lwmUInt8 *packetBytes = static_cast<const lwmUInt8 *>(packetData);
	for (lwmFastUInt16 cbi = 0; cbi < blockCount2; cbi++)
	{
		lwmUInt8 y[4];
		lwmUInt8 cb, cr;

		for (int i = 0; i < 4; i++)
			y[i] = packetBytes[i];
		cb = packetBytes[4];
		cr = packetBytes[5];
		packetBytes += 6;

		lwmFastSInt16 cbNormalized = static_cast<lwmSInt16>(cb) - 128;
		lwmFastSInt16 crNormalized = static_cast<lwmSInt16>(cr) - 128;

		Codebook2 *cb2entry = cb2 + cbi;
		Codebook2Double *cb2doubleentry = cb2double + cbi;
		for (int pxi = 0; pxi < 4; pxi++)
		{
			lwmFastSInt16 yFactor = static_cast<lwmSInt16>(y[pxi] << 6);
			lwmFastSInt16 rc = yFactor + (90 * crNormalized);
			lwmFastSInt16 gc = yFactor - (22 * cbNormalized) - (46 * crNormalized);
			lwmFastSInt16 bc = yFactor + (113 * cbNormalized);

			lwmUInt8 r, g, b;
			if (rc < 0) r = 0; else if (rc > 16320) r = 255; else r = static_cast<lwmUInt8>(rc >> 6);
			if (gc < 0) g = 0; else if (gc > 16320) g = 255; else g = static_cast<lwmUInt8>(gc >> 6);
			if (bc < 0) b = 0; else if (bc > 16320) b = 255; else b = static_cast<lwmUInt8>(bc >> 6);

			cb2entry->pixels[pxi * NUM_CHANNELS + 0] = r;
			cb2entry->pixels[pxi * NUM_CHANNELS + 1] = g;
			cb2entry->pixels[pxi * NUM_CHANNELS + 2] = b;
			for (unsigned int ch = 3; ch < NUM_CHANNELS; ch++)
				cb2entry->pixels[pxi * NUM_CHANNELS + ch] = 255;
		}

		for (unsigned int row = 0; row < 4; row++)
		{
			for (unsigned int col = 0; col < 4; col++)
			{
				unsigned int sourceIndex = ((col / 2) * 2 + (row / 2)) * NUM_CHANNELS;
				unsigned int destIndex = (col * 4 + row) * NUM_CHANNELS;
				cb2doubleentry->pixels[destIndex + 0] = cb2entry->pixels[sourceIndex + 0];
				cb2doubleentry->pixels[destIndex + 1] = cb2entry->pixels[sourceIndex + 1];
				cb2doubleentry->pixels[destIndex + 2] = cb2entry->pixels[sourceIndex + 2];
				for (unsigned int ch = 3; ch < NUM_CHANNELS; ch++)
					cb2doubleentry->pixels[destIndex + ch] = 255;
			}
		}
	}

	// CB4 is raw indexes
	for (lwmFastUInt16 cbi = 0; cbi < blockCount4; cbi++)
	{
		Codebook4 *cb4entry = cb4 + cbi;
		Codebook4Complete *cb4completeentry = cb4complete + cbi;
		lwmUInt8 indexes[4];
		for (int i = 0; i < 4; i++)
			indexes[i] = packetBytes[i];
		for (int i = 0; i < 4; i++)
			cb4entry->indexes[i] = indexes[i];
		packetBytes += 4;

		Blit<2 * NUM_CHANNELS, 2>(cb2[indexes[0]].pixels, cb4completeentry->pixels + 0,
			2 * NUM_CHANNELS, 4 * NUM_CHANNELS);
		Blit<2 * NUM_CHANNELS, 2>(cb2[indexes[1]].pixels, cb4completeentry->pixels + 2 * NUM_CHANNELS,
			2 * NUM_CHANNELS, 4 * NUM_CHANNELS);
		Blit<2 * NUM_CHANNELS, 2>(cb2[indexes[2]].pixels, cb4completeentry->pixels + 2 * (4 * NUM_CHANNELS),
			2 * NUM_CHANNELS, 4 * NUM_CHANNELS);
		Blit<2 * NUM_CHANNELS, 2>(cb2[indexes[3]].pixels, cb4completeentry->pixels + 2 * (4 * NUM_CHANNELS) + 2 * NUM_CHANNELS,
			2 * NUM_CHANNELS, 4 * NUM_CHANNELS);
	}

	return true;
}

bool lwmovie::roq::CVideoDecoder::ParseVideoFrame(const void *packetData, lwmLargeUInt packetSize, lwmUInt16 packetArg)
{
	lwmUInt32 frontFrame = m_recon->StartFrame();

	lwmSVideoFrameProvider *vfp = m_recon->GetFrameProvider();
	vfp->lockWorkFrameFunc(vfp, frontFrame, lwmVIDEOLOCK_Write_ReadLater);
	vfp->lockWorkFrameFunc(vfp, 1 - frontFrame, lwmVIDEOLOCK_Read);

	lwmUInt32 frontFramePitch;
	lwmUInt8 *frontPixelData = static_cast<lwmUInt8*>(vfp->getWorkFramePlaneFunc(vfp, frontFrame, 0, &frontFramePitch));
	lwmUInt32 backFramePitch;
	lwmUInt8 *backPixelData = static_cast<lwmUInt8*>(vfp->getWorkFramePlaneFunc(vfp, 1 - frontFrame, 0, &backFramePitch));


	ParseState parseState;
	parseState.outPitch = frontFramePitch;
	parseState.backPitch = backFramePitch;
	parseState.inBytes = static_cast<const lwmUInt8*>(packetData);
	parseState.byteLimit = parseState.inBytes + packetSize;
	parseState.parameterBits = 0;
	parseState.baseMX = static_cast<lwmSInt8>((packetArg >> 8) & 0xff);
	parseState.baseMY = static_cast<lwmSInt8>(packetArg & 0xff);

	lwmUInt8 *frontRow = frontPixelData;
	const lwmUInt8 *backRow = backPixelData;
	for (lwmUInt32 y = 0; y < m_height; y += 16)
	{
		lwmUInt8 *frontCel = frontRow;
		const lwmUInt8 *backCel = backRow;
		for (lwmUInt32 x = 0; x < m_width; x += 16)
		{
			if (!this->ParseQuad16(parseState, x, y, frontCel, backCel))
			{
				int bp = 0;
				goto finishFrame;
			}
			frontCel += 16 * NUM_CHANNELS;
			backCel += 16 * NUM_CHANNELS;
		}
		frontRow += frontFramePitch * 16;
		backRow += backFramePitch * 16;
	}

finishFrame:
	vfp->unlockWorkFrameFunc(vfp, 0);
	vfp->unlockWorkFrameFunc(vfp, 1);
	m_recon->FinishFrame();
	return true;
}

#define GETBYTE(x) \
	do\
		{\
		if(parseState.inBytes == parseState.byteLimit)\
			return false;\
		x = *parseState.inBytes++;\
		} while(0)

#define GETWORD(x) \
	do\
	{\
		if (parseState.byteLimit - parseState.inBytes < 2)\
			return false; \
		x = static_cast<lwmUInt16>(parseState.inBytes[0] | (parseState.inBytes[1] << 8));\
		parseState.inBytes += 2;\
	} while(0)

#define UNSPOOL(x) \
	do\
	{\
		if(parseState.parameterBits==0)\
		{\
			GETWORD(parseState.parameterWord);\
			parseState.parameterBits=16;\
		}\
		parseState.parameterBits -= 2;\
		x = ((parseState.parameterWord >> parseState.parameterBits) & 3);\
	} while (0)

bool lwmovie::roq::CVideoDecoder::ParseQuad16(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame)
{
	lwmLargeUInt outPitch8 = parseState.outPitch * 8;
	lwmLargeUInt backPitch8 = parseState.backPitch * 8;

	// Process 8x8 quads
	if (!ParseQuad8(parseState, x, y, frontFrame, backFrame))
		return false;
	if (!ParseQuad8(parseState, x + 8, y, frontFrame + 8 * NUM_CHANNELS, backFrame + 8 * NUM_CHANNELS))
		return false;
	if (!ParseQuad8(parseState, x, y + 8, frontFrame + outPitch8, backFrame + backPitch8))
		return false;
	if (!ParseQuad8(parseState, x + 8, y + 8, frontFrame + outPitch8 + 8 * NUM_CHANNELS, backFrame + backPitch8 + 8 * NUM_CHANNELS))
		return false;
	return true;
}

bool lwmovie::roq::CVideoDecoder::ParseQuad8(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame)
{
	int blockmark;

	UNSPOOL(blockmark);

	switch (blockmark)
	{
	case BLOCKMARK_SKIP:
		// In Id/Trilobyte videos, SKIP on the first frame blits from the back buffer.
		// lwmovie does not support this behavior and lwroqenc does not encode it.
		break;
	case BLOCKMARK_MOTION:
		{
			lwmUInt8 movement;
			GETBYTE(movement);

			// Decode offsets
			lwmSInt32 dx = 8 - ((movement >> 4) & 15) - parseState.baseMX;
			lwmSInt32 dy = 8 - (movement & 15) - parseState.baseMY;
			lwmSInt32 mx = x + dx;
			lwmSInt32 my = y + dy;

			if (mx < 0 || my < 0 || static_cast<lwmUInt32>(mx) > m_width - 8 || static_cast<lwmUInt32>(my) > m_height - 8)
				return false;

			// Blit the entire block
			Blit<8 * NUM_CHANNELS, 8>(backFrame + (dx * NUM_CHANNELS) + (dy * static_cast<lwmSInt32>(parseState.backPitch)), frontFrame, parseState.backPitch, parseState.outPitch);
		}
		break;
	case BLOCKMARK_VECTOR:
		{
			lwmUInt8 blockid;
			GETBYTE(blockid);

			// Blit an oversized 4x4 block
			const Codebook4 *cb4 = m_cb4 + blockid;
			lwmUInt32 frontPitch = parseState.outPitch;
			lwmUInt32 frontPitch4 = frontPitch * 4;
			lwmUInt32 backPitch4 = parseState.backPitch * 4;
			Blit<4 * NUM_CHANNELS, 4>(m_cb2double[cb4->indexes[0]].pixels, frontFrame, 4 * NUM_CHANNELS, frontPitch);
			Blit<4 * NUM_CHANNELS, 4>(m_cb2double[cb4->indexes[1]].pixels, frontFrame + 4 * NUM_CHANNELS, 4 * NUM_CHANNELS, frontPitch);
			Blit<4 * NUM_CHANNELS, 4>(m_cb2double[cb4->indexes[2]].pixels, frontFrame + frontPitch4, 4 * NUM_CHANNELS, frontPitch);
			Blit<4 * NUM_CHANNELS, 4>(m_cb2double[cb4->indexes[3]].pixels, frontFrame + frontPitch4 + 4 * NUM_CHANNELS, 4 * NUM_CHANNELS, frontPitch);
		}
		break;
	case BLOCKMARK_QUAD:
		{
			lwmUInt32 frontPitch4 = parseState.outPitch * 4;
			lwmUInt32 backPitch4 = parseState.backPitch * 4;
			if (!ParseQuad4(parseState, x, y, frontFrame, backFrame))
				return false;
			if (!ParseQuad4(parseState, x + 4, y, frontFrame + 4 * NUM_CHANNELS, backFrame + 4 * NUM_CHANNELS))
				return false;
			if (!ParseQuad4(parseState, x, y + 4, frontFrame + frontPitch4, backFrame + backPitch4))
				return false;
			if (!ParseQuad4(parseState, x + 4, y + 4, frontFrame + 4 * NUM_CHANNELS + frontPitch4, backFrame + 4 * NUM_CHANNELS + backPitch4))
				return false;
		}
		break;
	default:
		break;
	};

	return true;
}


bool lwmovie::roq::CVideoDecoder::ParseQuad4(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame)
{
	int blockmark;

	UNSPOOL(blockmark);

	switch (blockmark)
	{
	case BLOCKMARK_SKIP:
		// In Id/Trilobyte videos, SKIP on the first frame blits from the back buffer.
		// lwmovie does not support this behavior and lwroqenc does not encode it.
		break;
	case BLOCKMARK_MOTION:
		{
			lwmUInt8 movement;
			GETBYTE(movement);

			// Decode offsets
			lwmSInt32 dx = 8 - ((movement >> 4) & 15) - parseState.baseMX;
			lwmSInt32 dy = 8 - (movement & 15) - parseState.baseMY;
			lwmSInt32 mx = x + dx;
			lwmSInt32 my = y + dy;

			if (mx < 0 || my < 0 || static_cast<lwmUInt32>(mx) > m_width - 4 || static_cast<lwmUInt32>(my) > m_height - 4)
				return false;

			// Blit the entire block
			Blit<4 * NUM_CHANNELS, 4>(backFrame + (dx * NUM_CHANNELS) + (dy * static_cast<lwmSInt32>(parseState.backPitch)), frontFrame, parseState.backPitch, parseState.outPitch);
		}
		break;
	case BLOCKMARK_VECTOR:
		{
			lwmUInt8 blockid;
			GETBYTE(blockid);

			// Blit a 4x4 block
			Blit<4 * NUM_CHANNELS, 4>(this->m_cb4complete[blockid].pixels, frontFrame, 4 * NUM_CHANNELS, parseState.outPitch);
		}
		break;
	case BLOCKMARK_QUAD:
		{
			// Process 4 subsection quads
			if (parseState.byteLimit - parseState.inBytes < 4)
				return true;
			lwmUInt8 blockids[4];
			for (int i = 0; i < 4; i++)
				blockids[i] = parseState.inBytes[i];
			parseState.inBytes += 4;

			lwmLargeUInt pitch2 = parseState.outPitch * 2;
			ParseQuad2(parseState, frontFrame, blockids[0]);
			ParseQuad2(parseState, frontFrame + 2 * NUM_CHANNELS, blockids[1]);
			ParseQuad2(parseState, frontFrame + pitch2, blockids[2]);
			ParseQuad2(parseState, frontFrame + 2 * NUM_CHANNELS + pitch2, blockids[3]);
			return true;
		}
		break;
	};
	return true;
}

void lwmovie::roq::CVideoDecoder::ParseQuad2(ParseState &parseState, lwmUInt8 *frontFrame, lwmUInt8 blockid)
{
	// Blit in a 2x2 block
	Blit<2 * NUM_CHANNELS, 2>(m_cb2[blockid].pixels, frontFrame, 2 * NUM_CHANNELS, parseState.outPitch);
}

template<lwmLargeUInt TWidth, lwmLargeUInt THeight>
LWMOVIE_FORCEINLINE void lwmovie::roq::CVideoDecoder::Blit(const lwmUInt8 *source, lwmUInt8 *dest, lwmLargeUInt sourcePitch, lwmLargeUInt destPitch)
{
	for (lwmLargeUInt row = 0; row < THeight; row++)
	{
		memcpy(dest, source, TWidth);
		source += sourcePitch;
		dest += destPitch;
	}
}

template<lwmLargeUInt TWidth, lwmLargeUInt THeight>
LWMOVIE_FORCEINLINE void lwmovie::roq::CVideoDecoder::BlackFill(lwmUInt8 *dest, lwmLargeUInt pitch)
{
	for (lwmLargeUInt row = 0; row < THeight; row++)
	{
		memset(dest, 0, TWidth);
		dest += pitch;
	}
}
