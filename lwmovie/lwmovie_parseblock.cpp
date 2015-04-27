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
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "lwmovie_videotypes.hpp"
#include "lwmovie_vlc.hpp"
#include "lwmovie_bits.hpp"
#include "lwmovie_recon_m1v.hpp"
#include "lwmovie_profile.hpp"

bool lwmovie::m1v::CDeslicerJob::ParseReconBlock(CBitstream *bitstream, IM1VBlockCursor *blockCursor, lwmSInt32 n, IM1VReconstructor *recon, lwmCProfileTagSet *profileTags)
{
#ifdef LWMOVIE_DEEP_PROFILE
	lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ParseCoeffs);
#endif
	int coeffCount = 0;
	lwmFastSInt16 firstCoeff = 0;
	lwmFastUInt8 firstCoeffPos = 0;

	idct::DCTBLOCK *recondata = blockCursor->StartReconBlock(n);

	if(m_mblock.mb_intra)
	{
		//lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ParseCoeffsIntra);
		lwmFastSInt16 coeff;
		if (n < 4)
		{
			/*
			 * Get the luminance bits.  This code has been hand optimized to
			 * get by the normal bit parsing routines.  We get some speedup
			 * by grabbing the next 16 bits and parsing things locally.
			 * Thus, calls are translated as:
			 *
			 *    show_bitsX  <-->   next16bits >> (16-X)
			 *    get_bitsX   <-->   val = next16bits >> (16-flushed-X);
			 *               flushed += X;
			 *               next16bits &= bitMask[flushed];
			 *    flush_bitsX <-->   flushed += X;
			 *               next16bits &= bitMask[flushed];
			 *
			 * I've streamlined the code a lot, so that we don't have to mask
			 * out the low order bits and a few of the extra adds are removed.
			 *    bsmith
			 */
			lwmFastUInt16 next16bits, index;
			lwmFastUInt8 flushed;
			lwmFastUInt8 size;
			lwmFastSInt16 diff;

			next16bits = bitstream->show_bits16();
			index = next16bits >> (16-5);
			if(index < 31)
			{
				size = vlc::dct_dc_size_luminance[index].value;
				flushed = vlc::dct_dc_size_luminance[index].num_bits;
			}
			else
			{
				index = next16bits >> (16-9);
				index -= 0x1f0;
				size = vlc::dct_dc_size_luminance1[index].value;
				flushed = vlc::dct_dc_size_luminance1[index].num_bits;
			}

			if (size == lwmovie::m1v::vlc::UERROR8)
			{
				bitstream->flush_bits(flushed);
				return false;
			}

			next16bits &= bits::bitMask(16+flushed);

			if(size != 0)
			{
				flushed += size;	// Maxes out in the 60s, so 8-bit is OK
				diff = (next16bits >> (16-flushed));
				if((diff & bits::bitTest(32-size)) == 0)
					diff = (static_cast<lwmSInt32>(bits::rBitMask(size)) | (diff + 1));
				diff <<= 3;
			}
			else
				diff = 0;
			bitstream->flush_bits(flushed);

			if( (n == 0) && ((m_mblock.mb_address - m_mblock.past_intra_addr) > 1) )
				coeff = diff + 1024;
			else
				coeff = diff + m_block.dct_dc_y_past;
			m_block.dct_dc_y_past = coeff;
		}
		else
		{
			/* n = 4 or 5 */
			/*
			 * Get the chrominance bits.  This code has been hand optimized to
			 * as described above
			 */
			lwmFastUInt16 next16bits, index;
			lwmFastUInt8 flushed, size;
			lwmFastSInt16 diff;

			next16bits = bitstream->show_bits16();
			index = next16bits >> (16-5);
			if(index < 31)
			{
				size = vlc::dct_dc_size_chrominance[index].value;
				flushed = vlc::dct_dc_size_chrominance[index].num_bits;
			}
			else
			{
				index = next16bits >> (16-10);
				index -= 0x3e0;
				size = vlc::dct_dc_size_chrominance1[index].value;
				flushed = vlc::dct_dc_size_chrominance1[index].num_bits;
			}

			if (size == lwmovie::m1v::vlc::UERROR8)
			{
				bitstream->flush_bits(flushed);
				return false;
			}

			next16bits &= bits::bitMask(16+flushed);

			if(size != 0)
			{
				flushed += size;
				diff = static_cast<lwmSInt32>(next16bits >> (16-flushed));
				if((diff & bits::bitTest(32-size)) == 0)
					diff = static_cast<lwmSInt32>(bits::rBitMask(size)) | (diff + 1);
				diff <<= 3;
			}
			else
				diff = 0;
			bitstream->flush_bits(flushed);

			/* We test 5 first; a result of the mixup of Cr and Cb */
			coeff = diff;
			if(n == 5)
			{
				if(m_mblock.mb_address - m_mblock.past_intra_addr > 1)
					coeff += 1024;
				else
					coeff += m_block.dct_dc_cr_past;
				m_block.dct_dc_cr_past = static_cast<lwmSInt16>(coeff);
			}
			else
			{
				if(m_mblock.mb_address - m_mblock.past_intra_addr > 1)
					coeff += 1024;
				else
					coeff += m_block.dct_dc_cb_past;
				m_block.dct_dc_cb_past = static_cast<lwmSInt16>(coeff);
			}
		}

		lwmUInt8 pos = 0;
		if(coeff != 0)
		{
			coeffCount = 1;
			firstCoeffPos = 0;
			firstCoeff = coeff;
		}

		if(m_picture->code_type != constants::MPEG_D_TYPE)
		{
			lwmFastUInt8 qscale = m_slice.quant_scale;
			const lwmUInt16 *iqmatrix = this->m_iqmatrix;

			lwmFastUInt8 i = 0;
			while(true)
			{
				lwmUInt8 run;
				lwmSInt16 level;
				DecodeDCTCoeffNext(bitstream, &run, &level);

				if(run >= vlc::END_OF_BLOCK_U)
					break;

				i += run + 1;

				if(i >= 64)
					return false;		/* Illegal access */
				pos = constants::ZIGZAG_DIRECT[i];

				/* quantizes and oddifies each coefficient */
				coeff = (level * static_cast<lwmFastSInt32>(iqmatrix[pos])) / 8;
				if(level < 0)
					coeff |= 1;
				else
					coeff = (coeff-1) | 1;

				coeffCount++;

				if(coeffCount == 1)
				{
					firstCoeff = coeff;
					firstCoeffPos = pos;
				}
				else if(coeffCount == 2)
				{
					recondata->FastZeroFill();
					recondata->data[firstCoeffPos] = firstCoeff;
					recondata->data[pos] = coeff;
				}
				else
					recondata->data[pos] = coeff;
			}

			bitstream->flush_bits(2);
		}
	}
	else
	{
#ifdef LWMOVIE_DEEP_PROFILE
		//lwmCAutoProfile _(profileTags, lwmEPROFILETAG_ParseCoeffsInter);
#endif

		/* non-intra-coded macroblock */
		const lwmUInt16 *niqmatrix = m_niqmatrix;
		lwmFastUInt8 qscale = m_slice.quant_scale;
		lwmUInt8 run;
		lwmSInt16 level;

		DecodeDCTCoeffFirst(bitstream, &run, &level);

		lwmFastUInt8 i = run;	// Max 63

		lwmFastUInt8 pos = constants::ZIGZAG_DIRECT[i];

		/* quantizes and oddifies each coefficient */
		lwmFastSInt32 coeff;
		static int largestqscale = 0;
		if (qscale > largestqscale)
			largestqscale = qscale;
		if (level < 0)
		{
			coeff = (((level<<1) - 1) * static_cast<lwmFastSInt32>(niqmatrix[pos])) / 16; 
			coeff |= 1;
		}
		else
		{
			coeff = (((level<<1) + 1) * static_cast<lwmFastSInt32>(niqmatrix[pos])) / 16; 
			coeff = (coeff-1) | 1; /* equivalent to: if ((coeff&1)==0) coeff = coeff - 1; */
		}

		// coeff is never zero
		firstCoeffPos = pos;
		firstCoeff = static_cast<lwmFastSInt16>(coeff);
		coeffCount = 1;

		if(m_picture->code_type != constants::MPEG_D_TYPE)
		{
			while(true)
			{
				DecodeDCTCoeffNext(bitstream, &run, &level);

				if(run >= vlc::END_OF_BLOCK_U)
					break;

				i += run + 1;
				if(i >= 64)
					return false;	/* Illegal access */

				pos = constants::ZIGZAG_DIRECT[i];

				if(level < 0)
				{
					coeff = (((level<<1) - 1) * static_cast<lwmFastSInt32>(niqmatrix[pos])) / 16; 
					coeff |= 1;
				}
				else
				{
					coeff = (((level<<1) + 1) * static_cast<lwmFastSInt32>(niqmatrix[pos])) / 16; 
					coeff = (coeff-1) | 1; /* equivalent to: if ((coeff&1)==0) coeff = coeff - 1; */
				}

				coeffCount++;
				if(coeffCount == 2)
				{
					recondata->FastZeroFill();
					recondata->data[firstCoeffPos] = firstCoeff;
				}

				recondata->data[pos] = static_cast<lwmSInt16>(coeff);
			} /* end while */
			bitstream->flush_bits(2);
		} /* end if (vid_stream->picture.code_type != 4) */
	}

#ifdef LWMOVIE_DEEP_PROFILE
	//lwmCAutoProfile _2(profileTags, lwmEPROFILETAG_ParseCoeffsCommit);
	//lwmCAutoProfile _3(profileTags, lwmEPROFILETAG_ParseCoeffsTest);
#endif

	if(coeffCount == 0)
	{
		// Sparse IDCT, as zero
		blockCursor->CommitZero();
	}
	else if(coeffCount == 1)
	{
		// Sparse IDCT
		static const lwmUInt8 zigzagOrder[] =
		{
			0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
            19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
            42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
            58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
        };

		blockCursor->CommitSparse(zigzagOrder[firstCoeffPos], firstCoeff);
	}
	else
	{
		// Full IDCT
		blockCursor->CommitFull();
	}

	return true;
}
