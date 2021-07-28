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
#ifndef __LWMOVIE_LAYER2_CONSTANTS_HPP__
#define __LWMOVIE_LAYER2_CONSTANTS_HPP__

#include "../common/lwmovie_coretypes.h"
#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_fixedreal.hpp"
#include "lwmovie_layer2_csf.hpp"

#include "../common/lwmovie_coretypes.h"

namespace lwmovie
{
	namespace layerii
	{
		static const int NUM_SUBBANDS			= 32;
		static const int FILTER_SIZE			= 16;
		static const int MAX_CHANNELS			= 2;
		static const int MAX_BITALLOC			= 4;
		static const int SCALE_BLOCK_SIZE		= 12;

		static const int OUT_SCALE_SHIFT		= 15;
		static const int OUT_SCALE_MAX			= ((1 << OUT_SCALE_SHIFT)-1);
		static const int OUT_SCALE_MIN			= (-(1 << OUT_SCALE_SHIFT));
		static const int NUM_BITRATE_INDEXES	= 15;
		static const int NUM_SAMPLERATE_INDEXES	= 3;
		static const int NUM_TABLESETS			= 3;
		static const int NUM_QINDEX_TABLES		= 6;
		static const int NUM_QINDEX				= 17;

		static const int MAX_FRAME_SIZE_BYTES	= 4800;		// Maximum size of a frame in bytes, including header
		static const int HEADER_SIZE_BYTES		= 4;		// Size of a frame header in bytes
		static const int FRAME_NUM_SAMPLES		= 1152;

		enum EMPEGVersion
		{
			EMPEGVersion_MPEG1,
			EMPEGVersion_MPEG2,

			EMPEGVersion_Count,
		};

		struct MP2QuantInfo
		{
			lwmUInt8 codeLength;
			lwmUInt8 modGroup;

			// Generated
			lwmFixedReal29 rcp;
		};

		extern const lwmUInt16	MP2_BITRATE_KBPS[EMPEGVersion_Count][NUM_BITRATE_INDEXES];
		extern const lwmUInt8	MP2_TABLESET_MPEG1[MAX_CHANNELS][NUM_BITRATE_INDEXES];
		extern const lwmUInt16	MP2_SAMPLERATE[EMPEGVersion_Count][NUM_SAMPLERATE_INDEXES];
		extern const lwmUInt8	MP2_SBLIMITS_MPEG1[MAX_CHANNELS][NUM_SAMPLERATE_INDEXES][NUM_BITRATE_INDEXES];
		extern const lwmUInt8	MP2_BITALLOC_SIZE[NUM_TABLESETS][NUM_SUBBANDS];
		extern lwmCompressedSF	MP2_SCALEFACTORS[64];

		extern const lwmUInt8 MP2_QINDEX_TABLE_SELECT[NUM_TABLESETS][NUM_SUBBANDS];
		extern const lwmUInt8 MP2_QINDEX[NUM_QINDEX_TABLES][1 << MAX_BITALLOC];
		extern const lwmUInt8 MP2_QUANT_GROUP[NUM_QINDEX_TABLES][NUM_QINDEX];
		extern MP2QuantInfo MP2_QUANT_INFO[NUM_QINDEX];

		extern LWMOVIE_FIXEDREAL_SIMD_ALIGN_ATTRIB lwmSInt32 MP2_DEWINDOW_BASE[FILTER_SIZE][NUM_SUBBANDS];
		extern LWMOVIE_FIXEDREAL_SIMD_ALIGN_ATTRIB lwmFixedReal29 MP2_DEWINDOW[FILTER_SIZE][NUM_SUBBANDS];

		
		void InitializeTables();
	}
}

#endif
