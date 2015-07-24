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
#ifndef __LWMOVIE_VLC_HPP__
#define __LWMOVIE_VLC_HPP__

#include "../common/lwmovie_coretypes.h"

#define LWMOVIE_PREPROCESS_MPEG_1_VLC(n) ((n << 1) - 1)

namespace lwmovie
{
	namespace m1v
	{
		namespace vlc
		{
			const lwmUInt8 UERROR8 = 255;
			const lwmSInt8 ERROR8 = -128;

			//const uint DCT_ERROR = 63;

			const lwmUInt8 MACRO_BLOCK_STUFFING = 34;
			const lwmUInt8 MACRO_BLOCK_ESCAPE = 35;

			/* Two types of DCT Coefficients */
			//const int DCT_COEFF_FIRST = 0;
			//const int DCT_COEFF_NEXT = 1;

			/* Special values for DCT Coefficients */
			const lwmUInt8 DCT_ERROR_U = 63;
			const lwmSInt8 DCT_ERROR_S = 63;
			const lwmUInt8 END_OF_BLOCK_U = 62;
			const lwmSInt8 END_OF_BLOCK_S = 62;
			const lwmUInt8 ESCAPE_U = 61;
			const lwmSInt8 ESCAPE_S = 61;

			struct lwmVlcValue8
			{
				lwmUInt8 value;
				lwmUInt8 num_bits;
			};

			struct lwmVlcValueS8
			{
				lwmSInt8 value;
				lwmUInt8 num_bits;
			};

			// Homogenized run-level code
			struct lwmHRLC
			{
				lwmUInt8 run;
				lwmSInt8 level;
				lwmUInt8 num;
			};

			enum lwmEMBTypeFlags
			{
				MB_FLAG_QUANT = 1,
				MB_FLAG_MOTION_FORWARD = 2,
				MB_FLAG_MOTION_BACKWARD = 4,
				MB_FLAG_PATTERN = 8,
				MB_FLAG_INTRA = 16,
			};

			extern lwmVlcValue8 coded_block_pattern[];
			extern lwmVlcValue8 dct_dc_size_luminance[];
			extern lwmVlcValue8 dct_dc_size_luminance1[];
			extern lwmVlcValue8 dct_dc_size_chrominance[];
			extern lwmVlcValue8 dct_dc_size_chrominance1[];

			extern lwmVlcValue8 mb_type_P[];
			extern lwmVlcValue8 mb_type_B[];
			extern lwmVlcValue8 mb_addr_inc[2048];

			extern lwmVlcValueS8 motion_vectors[2048];

			extern lwmHRLC dct_coeff_tbl_0[];
			extern lwmHRLC dct_coeff_tbl_1[];
			extern lwmHRLC dct_coeff_tbl_2[];
			extern lwmHRLC dct_coeff_tbl_3[];
			extern lwmHRLC dct_coeff_next[];
			extern lwmHRLC dct_coeff_first[];

			void InitTables();
		}
	}
}

#endif
