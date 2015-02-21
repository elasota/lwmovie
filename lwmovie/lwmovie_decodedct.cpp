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

/*
* Copyright (c) 1995 The Regents of the University of California.
* All rights reserved.    
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
* CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS
* ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/*
* Portions of this software Copyright (c) 1995 Brown University.
* All rights reserved.
* 
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement
* is hereby granted, provided that the above copyright notice and the
* following two paragraphs appear in all copies of this software.
* 
* IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
* UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.    THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
* BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
* SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "../common/lwmovie_coretypes.h"
#include "lwmovie_videotypes.hpp"
#include "lwmovie_vlc.hpp"
#include "lwmovie_bits.hpp"

void lwmovie::lwmDeslicerJob::DecodeDCTCoeff(lwmCBitstream *bitstream, const lwmUInt16 *dct_coeff_tbl, lwmUInt8 *outRun, lwmSInt16 *outLevel)
{
	/*
	 * Grab the next 32 bits and use it to improve performance of
	 * getting the bits to parse. Thus, calls are translated as:
	 *
	 *	show_bitsX  <-->   next32bits >> (32-X)
	 *	get_bitsX   <-->   val = next32bits >> (32-flushed-X);
	 *			   flushed += X;
	 *			   next32bits &= bitMask[flushed];
	 *	flush_bitsX <-->   flushed += X;
	 *			   next32bits &= bitMask[flushed];
	 *
	 */
	lwmUInt32 next32bits = bitstream->show_bits32();
	lwmUInt32 flushed = 0;

	/* show_bits8(index); */
	lwmUInt32 index = next32bits >> 24;


	if (index > 3)
	{
		lwmUInt16 value = dct_coeff_tbl[index];
		*outRun = (value & lwmovie::vlc::RUN_MASK) >> lwmovie::vlc::RUN_SHIFT;
		if (*outRun == lwmovie::vlc::END_OF_BLOCK_U)
		{
			*outLevel = lwmovie::vlc::END_OF_BLOCK_S;
		}
		else
		{
			/* num_bits = (value & NUM_MASK) + 1; */
			/* flush_bits(num_bits); */
			lwmUInt8 flushed = static_cast<lwmUInt8>((value & lwmovie::vlc::NUM_MASK) + 1);
			next32bits &= lwmovie::bits::bitMask(flushed);
			if(*outRun != lwmovie::vlc::ESCAPE_U)
			{
				*outLevel = (value & lwmovie::vlc::LEVEL_MASK) >> lwmovie::vlc::LEVEL_SHIFT;
				/* get_bits1(value); */
				/* if (value) *level = -*level; */
				if (next32bits >> (31-flushed))
					*outLevel = -*outLevel;
				flushed++;
				/* next32bits &= bitMask[flushed];  last op before update */
			}
			else
			{
				/* *run == ESCAPE */
				/* get_bits14(temp); */
				lwmUInt16 temp = next32bits >> (18-flushed);
				flushed += 14;
				next32bits &= lwmovie::bits::bitMask(flushed);
				*outRun = static_cast<lwmUInt8>(temp >> 8);
				temp &= 0xff;
				if (temp == 0)
				{
					/* get_bits8(*level); */
					*outLevel = static_cast<lwmUInt16>(next32bits >> (24-flushed));
					flushed += 8;
					/* next32bits &= bitMask[flushed];  last op before update */
				}
				else if (temp != 128)
				{
					/* Grab sign bit */
					*outLevel = static_cast<lwmSInt16>((static_cast<lwmSInt16>(temp << 8)) >> 8);
				}
				else
				{
					/* get_bits8(*level); */
					*outLevel = next32bits >> (24-flushed);
					flushed += 8;
					/* next32bits &= bitMask[flushed];  last op before update */
					(*outLevel) -= 256;
				}
			}
			/* Update bitstream... */
			bitstream->flush_bits(flushed);
		}
	}
	else
	{
		lwmUInt16 value;
		if (index == 2)
		{
			/* show_bits10(index); */
			index = next32bits >> 22;
			value = lwmovie::vlc::dct_coeff_tbl_2[index & 3];
		}
		else if (index == 3)
		{
			/* show_bits10(index); */
			index = next32bits >> 22;
			value = lwmovie::vlc::dct_coeff_tbl_3[index & 3];
		}
		else if (index)
		{
			/* index == 1 */
			/* show_bits12(index); */
			index = next32bits >> 20;
			value = lwmovie::vlc::dct_coeff_tbl_1[index & 15];
		}
		else
		{
			/* index == 0 */
			/* show_bits16(index); */
			index = next32bits >> 16;
			value = lwmovie::vlc::dct_coeff_tbl_0[index & 255];
		}
		*outRun = (value & lwmovie::vlc::RUN_MASK) >> lwmovie::vlc::RUN_SHIFT;
		*outLevel = (value & lwmovie::vlc::LEVEL_MASK) >> lwmovie::vlc::LEVEL_SHIFT;

		/*
		 * Fold these operations together to make it fast...
		 */
		/* num_bits = (value & NUM_MASK) + 1; */
		/* flush_bits(num_bits); */
		/* get_bits1(value); */
		/* if (value) *level = -*level; */

		flushed = (value & lwmovie::vlc::NUM_MASK) + 2;
		if ((next32bits >> (32-flushed)) & 0x1)
			*outLevel = -*outLevel;

		/* Update bitstream ... */
		bitstream->flush_bits(flushed);
	}
}

void lwmovie::lwmDeslicerJob::DecodeDCTCoeffFirst(lwmCBitstream *bitstream, lwmUInt8 *outRun, lwmSInt16 *outLevel)
{
	DecodeDCTCoeff(bitstream, lwmovie::vlc::dct_coeff_first, outRun, outLevel);
}

void lwmovie::lwmDeslicerJob::DecodeDCTCoeffNext(lwmCBitstream *bitstream, lwmUInt8 *outRun, lwmSInt16 *outLevel)
{
	DecodeDCTCoeff(bitstream, lwmovie::vlc::dct_coeff_next, outRun, outLevel);
}

lwmUInt8 lwmovie::lwmDeslicerJob::DecodeDCTDCSizeLum(lwmCBitstream *bitstream)
{
	lwmUInt32 index = bitstream->show_bits5();
	if (index < 31)
	{
		bitstream->flush_bits(lwmovie::vlc::dct_dc_size_luminance[index].num_bits);
		return lwmovie::vlc::dct_dc_size_luminance[index].value;
	}
	else
	{
		index = bitstream->show_bits9();
		index -= 0x1f0;
		bitstream->flush_bits(lwmovie::vlc::dct_dc_size_luminance1[index].num_bits);
		return lwmovie::vlc::dct_dc_size_luminance1[index].value;
	}
}

lwmUInt8 lwmovie::lwmDeslicerJob::DecodeDCTDCSizeChrom(lwmCBitstream *bitstream)
{
	lwmUInt32 index = bitstream->show_bits5();

	if (index < 31)
	{
		bitstream->flush_bits(lwmovie::vlc::dct_dc_size_chrominance[index].num_bits);
		return lwmovie::vlc::dct_dc_size_chrominance[index].value;
	}
	else
	{
		index = bitstream->show_bits10();
		index -= 0x3e0;
		bitstream->flush_bits(lwmovie::vlc::dct_dc_size_chrominance1[index].num_bits);
		return lwmovie::vlc::dct_dc_size_chrominance1[index].value;
	}
}
