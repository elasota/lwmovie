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
#ifndef __LWMOVIE_LAYER2_DECODESTATE_HPP__
#define __LWMOVIE_LAYER2_DECODESTATE_HPP__

#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_constants.hpp"
#include "lwmovie_layer2_dspops.hpp"

namespace lwmovie
{
	namespace layerii
	{
		class lwmCMP2DecodeState
		{
			LWMOVIE_FIXEDREAL_SIMD_ALIGN_ATTRIB	lwmFixedReal22		m_accumulators[MAX_CHANNELS][FILTER_SIZE][NUM_SUBBANDS];
			int					m_sbsRotator;

			lwmUInt8			m_tableSet;	// 0 = 0/1, 1 = 2/3
			bool				m_hasPadding;
			bool				m_hasChecksum;
			bool				m_isJointStereo;
			//lwmUInt8			m_jsLimit;
			lwmUInt16			m_sampleRate;
			lwmUInt16			m_bitrateKbps;
			lwmUInt8			m_bitrateIndex;
			lwmUInt8			m_samplerateIndex;
			lwmUInt8			m_numChannels;
			lwmUInt8			m_modeExt;
			EMPEGVersion		m_mpegVersion;

		public:
			lwmCMP2DecodeState();
			bool ParseHeader(const void *bytes);

			bool DecodeFrame(const void *bytes, lwmSInt16 *output);

			lwmUInt16 GetFrameSizeBytes() const;
			lwmUInt8 GetChannelCount() const;
			lwmUInt32 GetSampleRate() const;

		private:
			template<lwmFastUInt8 numChannels>
			bool DecodeFrameCH(const void *bytes, lwmSInt16 *output);
		};
	}
}

#endif
