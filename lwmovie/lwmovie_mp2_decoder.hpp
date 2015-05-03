/*
* Copyright (c) 2014 Eric Lasota
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
#ifndef __LWMOVIE_MP2_DECODER_HPP__
#define __LWMOVIE_MP2_DECODER_HPP__

#include "../mp2dec2/lwmovie_layer2_decodestate.hpp"
#include "lwmovie_audiobuffer.hpp"
#include "lwmovie_audiocodec.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;
struct lwmMovieHeader;

namespace lwmovie
{
	namespace layerii
	{
		class CDecoder : public CAudioCodec
		{
		public:
			explicit CDecoder(lwmSAllocator *alloc);
			~CDecoder();
			bool Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *commonInfo, const lwmAudioStreamInfo *audioStreamInfo);
			bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
			CAudioBuffer *GetAudioBuffer();

		private:
			lwmSAllocator *m_alloc;
			lwmUInt8 m_numChannels;
			lwmovie::layerii::CDecodeState m_decodeState;
			lwmUInt8 m_frameData[lwmovie::layerii::MAX_FRAME_SIZE_BYTES];
			CAudioBuffer m_audioBuffer;
		};
	}
}

#endif
