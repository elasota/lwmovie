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
#include <math.h>

#include "lwmovie_layer2_constants.hpp"

namespace lwmovie
{
	namespace layerii
	{
		const lwmUInt16 MP2_BITRATE_KBPS[EMPEGVersion_Count][NUM_BITRATE_INDEXES] =
		{
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },
			{ 0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160 },
		};

		const lwmUInt8 MP2_TABLESET[EMPEGVersion_Count][MAX_CHANNELS][NUM_BITRATE_INDEXES] =
		{
			{
				{ 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
			},
			{
				{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
			},
		};

		const lwmUInt16	MP2_SAMPLERATE[EMPEGVersion_Count][NUM_SAMPLERATE_INDEXES] =
		{
			{ 44100, 48000, 32000 },
			{ 22050, 2400,  1600  },
		};

		const lwmUInt8 MP2_SBLIMITS_MPEG1[MAX_CHANNELS][NUM_SAMPLERATE_INDEXES][NUM_BITRATE_INDEXES] =
		{
			{
				{ 8,  8,  8,  27, 27, 27, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
				{ 8,  8,  8,  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27 },
				{ 12, 12, 12, 27, 27, 27, 30, 30, 30, 30, 30, 30, 30, 30, 30 },

			},
			{
				{ 8,  8,  8,  8,  8,  8,  8,  27, 27, 27, 30, 30, 30, 30, 30 },
				{ 8,  8,  8,  8,  8,  8,  8,  27, 27, 27, 27, 27, 27, 27, 27 },
				{ 12, 12, 12, 12, 12, 12, 12, 27, 27, 27, 30, 30, 30, 30, 30 },
			},
		};

		const lwmUInt8 MP2_BITALLOC_SIZE[NUM_TABLESETS][NUM_SUBBANDS] =
		{
			{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
			{ 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
		};

		const lwmUInt8 MP2_QINDEX_TABLE_SELECT[NUM_TABLESETS][NUM_SUBBANDS] =
		{
			{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
			{ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 },
		};

		const lwmUInt8 MP2_QINDEX[NUM_QINDEX_TABLES][1 << MAX_BITALLOC] =
		{
			{ 0xff, 0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },
			{ 0xff, 0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  10, 11, 12, 13, 16 },
			{ 0xff, 0, 1, 2, 3, 4, 5, 16 },
			{ 0xff, 0, 1, 16 },

			{ 0xff, 0, 1, 3, 4, 5, 6, 7, 8, 9,  10, 11, 12, 13, 14, 15 },
			{ 0xff, 0, 1, 3, 4, 5, 6, 7 },
		};

		const lwmUInt8 MP2_QUANT_GROUP[NUM_QINDEX_TABLES][NUM_QINDEX] =
		{
			{ 3, 5, 0, 9, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		};

		MP2QuantInfo MP2_QUANT_INFO[NUM_QINDEX] =
		{
			{ 5,  3 },
			{ 7,  5 },
			{ 3,  0 },
			{ 10, 9 },
			{ 4,  0 },
			{ 5,  0 },
			{ 6,  0 },
			{ 7,  0 },
			{ 8,  0 },
			{ 9,  0 },
			{ 10, 0 },
			{ 11, 0 },
			{ 12, 0 },
			{ 13, 0 },
			{ 14, 0 },
			{ 15, 0 },
			{ 16, 0 },
		};
		
		const lwmFixedReal29 (*MP2_DEWINDOW)[NUM_SUBBANDS];

#include "lwmovie_layer2_dewindowtable.hpp"

		lwmCompressedSF MP2_SCALEFACTORS[64];

		void InitializeTables()
		{
			for(int i=0;i<64;i++)
				MP2_SCALEFACTORS[i] = LWMOVIE_CREATE_CSF(i);
			for(int i=0;i<NUM_QINDEX;i++)
			{
				if(MP2_QUANT_INFO[i].modGroup)
					MP2_QUANT_INFO[i].rcp = lwmFixedReal29(1.0 / static_cast<double>(MP2_QUANT_INFO[i].modGroup));
				else
					MP2_QUANT_INFO[i].rcp = lwmFixedReal29(1.0 / static_cast<double>((1 << MP2_QUANT_INFO[i].codeLength) - 1));
			}

			// Convert the base table in-place
			lwmFixedReal29 (*output)[NUM_SUBBANDS] = reinterpret_cast<lwmFixedReal29 (*)[NUM_SUBBANDS]>(MP2_DEWINDOW_BASE);
			MP2_DEWINDOW = output;
			for(int i=0;i<FILTER_SIZE;i++)
			{
				for(int j=0;j<NUM_SUBBANDS;j++)
				{
					lwmFixedReal29 fr;
#ifdef LWMOVIE_FIXEDPOINT
					fr = lwmFixed32<0, lwmSInt32>(MP2_DEWINDOW_BASE[i][j]).RShiftFixed<16>().IncreaseFracPrecision<13>();
#else
					fr = static_cast<lwmFloat32>(MP2_DEWINDOW_BASE[i][j]) * (1.0f / 65536.0f);
#endif
					memcpy(output[i] + j, &fr, sizeof(lwmFixedReal29));
				}
			}
		}
	}
}


