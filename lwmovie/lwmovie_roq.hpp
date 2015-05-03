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
#ifndef __LWMOVIE_ROQ_HPP__
#define __LWMOVIE_ROQ_HPP__

#include "../common/lwmovie_coretypes.h"
#include "lwmovie_external_types.h"

struct lwmSAllocator;
struct lwmIVideoReconstructor;

namespace lwmovie
{
	namespace roq
	{
		class CSoftwareReconstructor;

		class CVideoDecoder
		{
		public:
			CVideoDecoder(lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height);
			bool DigestDataPacket(const void *packetData, lwmLargeUInt packetSize, lwmUInt32 *outResult);
			void SetReconstructor(lwmIVideoReconstructor *videoRecon);

		private:
			struct Codebook2
			{
				lwmUInt8 pixels[2 * 2 * 3];
			};

			struct Codebook2Double
			{
				lwmUInt8 pixels[4 * 4 * 3];
			};

			struct Codebook4
			{
				lwmUInt8 indexes[4];
			};

			struct Codebook4Complete
			{
				lwmUInt8 pixels[4 * 4 * 3];
			};

			struct ParseState
			{
				lwmUInt32 outPitch;
				lwmUInt32 backPitch;

				const lwmUInt8 *inBytes;
				const lwmUInt8 *byteLimit;
				lwmUInt16 parameterWord;
				lwmFastUInt8 parameterBits;
				lwmFastSInt8 baseMX;
				lwmFastSInt8 baseMY;
			};

			bool ProcessFrame(const void *packetData, lwmLargeUInt packetSize);
			static void ParseHeader(const void *headerData, lwmUInt16 &outMarker, lwmUInt32 &outSize, lwmUInt16 &outArgument);
			bool ParseCodebooks(const void *packetData, lwmLargeUInt packetSize, lwmUInt16 packetArg);
			bool ParseVideoFrame(const void *packetData, lwmLargeUInt packetSize, lwmUInt16 packetArg);
			bool ParseQuad16(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame);
			bool ParseQuad8(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame);
			bool ParseQuad4(ParseState &parseState, lwmUInt32 x, lwmUInt32 y, lwmUInt8 *frontFrame, const lwmUInt8 *backFrame);
			void ParseQuad2(ParseState &parseState, lwmUInt8 *frontFrame, lwmUInt8 blockID);

			template<lwmLargeUInt TWidth, lwmLargeUInt THeight>
			static void Blit(const lwmUInt8 *source, lwmUInt8 *dest, lwmLargeUInt sourcePitch, lwmLargeUInt destPitch);
			template<lwmLargeUInt TWidth, lwmLargeUInt THeight>
			void BlackFill(lwmUInt8 *dest, lwmLargeUInt pitch);

			Codebook2 m_cb2[256];
			Codebook4 m_cb4[256];
			Codebook4Complete m_cb4complete[256];
			Codebook2Double m_cb2double[256];

			lwmUInt32 m_width;
			lwmUInt32 m_height;
			lwmSAllocator *m_alloc;
			CSoftwareReconstructor *m_recon;
		};
	}
}

#endif
