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
#ifndef __LWMOVIE_CONSTANTS_HPP__
#define __LWMOVIE_CONSTANTS_HPP__

#include "../common/lwmovie_coretypes.h"

namespace lwmovie
{
	namespace m1v
	{
		namespace constants
		{
			enum lwmEParseState
			{
				PARSE_NO_VID_STREAM = (-1),
				PARSE_STREAM_UNDERFLOW = (-2),
				PARSE_FATAL = (-3),
				PARSE_SKIP_PICTURE = (-4),
				PARSE_SKIP_TO_START_CODE = (-5),
				PARSE_OK = 1,
				PARSE_BREAK = 2,
			};

			/* Start code search modes */
			enum lwmESearchMode
			{
				SEARCH_NONE = 0,				// Don't scan
				SEARCH_ANY = 1,					// Exit scan on any code
				SEARCH_NEXT_PICTURE = 2,		// Exit on PICTURE_START_CODE, GOP_START_CODE, or SEQ_END_CODE
				SEARCH_MACROBLOCK_FINISH = 3,	// Any, but cycle display if not within SLICE_MIN-SLICE_START and not ERROR
			};

			enum lwmEMBQuantType
			{
				MB_QUANT_TYPE_FALSE = 0,
				MB_QUANT_TYPE_TRUE,
				MB_QUANT_TYPE_ERROR,
			};

			enum lwmEQScaleType
			{
				Q_SCALE_TYPE_MPEG1,
				Q_SCALE_TYPE_MPEG2,
			};

			enum lwmEIntraVLCFormat
			{
				INTRA_VLC_FORMAT_MPEG1,
				INTRA_VLC_FORMAT_MPEG2,
			};

			enum lwmEIntraDCPrecision
			{
				INTRA_DC_PRECISION_8,
				INTRA_DC_PRECISION_9,
				INTRA_DC_PRECISION_10,
				INTRA_DC_PRECISION_11,
			};

			enum lwmEZigZagScanOrder
			{
				ZIGZAG_SCAN_ORDER_MPEG1,
				ZIGZAG_SCAN_ORDER_MPEG2_ALTERNATE,
			};

			const lwmSInt32 MPEG_I_TYPE = 1;
			const lwmSInt32 MPEG_P_TYPE = 2;
			const lwmSInt32 MPEG_B_TYPE = 3;
			const lwmSInt32 MPEG_D_TYPE = 4;

			/* Start codes. */

			const lwmUInt32 MPEG_SEQ_END_CODE = 0x000001b7;
			const lwmUInt32 MPEG_SEQ_START_CODE = 0x000001b3;
			const lwmUInt32 MPEG_GOP_START_CODE = 0x000001b8;
			const lwmUInt32 MPEG_PICTURE_START_CODE = 0x00000100;
			const lwmUInt32 MPEG_SLICE_MIN_START_CODE = 0x00000101;
			const lwmUInt32 MPEG_SLICE_MAX_START_CODE = 0x000001af;
			const lwmUInt32 MPEG_EXT_START_CODE = 0x000001b5;
			const lwmUInt32 MPEG_USER_START_CODE = 0x000001b2;
			const lwmUInt32 MPEG_SEQUENCE_ERROR_CODE = 0x000001b4;
			const lwmUInt32 LWMOVIE_PICTURE_END_CODE = 0x000001fd;

			const lwmUInt32 MPEG_EXT_TYPE_PICTURE_CODING_EXTENSION = 0x8;

			const lwmUInt32 DISPLAY_LOCK = 0x01;
			const lwmUInt32 PAST_LOCK = 0x02;
			const lwmUInt32 FUTURE_LOCK = 0x04;

			/* Number of macroblocks to process in one call to mpegVidRsrc. */
			const lwmUInt32 MB_QUANTUM = 40;

			/* Macros used with macroblock address decoding. */
			const lwmSInt32 MPEG_MB_STUFFING = 34;
			const lwmSInt32 MPEG_MB_ESCAPE = 35;

			const lwmUInt32 MPEG_EXTRA_SAFETY_LIMIT = 1000;

			extern lwmUInt8 ZIGZAG[64];
			extern lwmUInt8 DEFAULT_INTRA_MATRIX[64];
			extern lwmUInt8 DEFAULT_NON_INTRA_MATRIX[64];
			extern lwmUInt8 ZIGZAG_DIRECT[64];

			extern lwmUInt8 Q_SCALE_MPEG2[32];
		}
	}
}

#endif
