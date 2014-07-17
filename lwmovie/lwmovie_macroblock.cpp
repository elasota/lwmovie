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
#include <stdlib.h>
#include "lwmovie_videotypes.hpp"
#include "lwmovie_bits.hpp"
#include "lwmovie_vlc.hpp"
#include "lwmovie_recon_m1v.hpp"

namespace lwmovie
{
	void WriteDctRowToPlane( const lwmSInt16 *row, lwmUInt8 *px );
	void WriteDctBlockToPlane( const lwmDCTBLOCK *block, lwmUInt8 *px, lwmLargeUInt pitch );
	void AccumDctRowToPlane( const lwmSInt16 *row, lwmUInt8 *dest, const lwmUInt8 *src );
	void AccumDctBlockToPlane( const lwmDCTBLOCK *block, lwmUInt8 *dest, const lwmUInt8 *src, lwmLargeUInt pitch );
}

inline void lwmovie::WriteDctRowToPlane( const lwmSInt16 *row, lwmUInt8 *px )
{
	for(lwmLargeUInt i=0;i<8;i++)
		px[i] = lwmovie::bits::saturate8(row[i]);
}

inline void lwmovie::WriteDctBlockToPlane( const lwmDCTBLOCK *block, lwmUInt8 *px, lwmLargeUInt pitch )
{
	for(lwmLargeUInt i=0;i<8;i++)
		WriteDctRowToPlane(block->data + i*8, px + i*pitch);
}

inline void lwmovie::AccumDctRowToPlane( const lwmSInt16 *row, lwmUInt8 *dest, const lwmUInt8 *src )
{
	for(lwmLargeUInt i=0;i<8;i++)
		dest[i] = lwmovie::bits::saturate8(src[i] + row[i]);
}

inline void lwmovie::AccumDctBlockToPlane( const lwmDCTBLOCK *block, lwmUInt8 *dest, const lwmUInt8 *src, lwmLargeUInt pitch )
{
	for(lwmLargeUInt i=0;i<8;i++)
		AccumDctRowToPlane(block->data + i*8, dest + i*pitch, src + i*pitch);
}

#if 0
void lwmovie::lwmVidStream::ReconIMBlock( int bnum )
{
	lwmUInt8 *destPlane;

	/* Calculate macroblock row and column from address. */
	lwmUInt32 mb_row = static_cast<lwmUInt32>(m_mblock.mb_address) / m_mb_width;
	lwmUInt32 mb_col = static_cast<lwmUInt32>(m_mblock.mb_address) % m_mb_width;
	lwmLargeUInt pitch;
	lwmUInt32 row, col;

	/* If block is luminance block... */
	if (bnum < 4)
	{
		/* Calculate row and col values for upper left pixel of block. */
		row = mb_row * 16;
		col = mb_col * 16;
		if (bnum > 1)
			row += 8;
		if (bnum % 2 != 0)
			col += 8;

		/* Set dest to luminance plane of current pict image. */
		destPlane = m_current->GetLuminancePlane();

		/* Establish row size. */
		pitch = m_mb_width * 16;
	}
	/* Otherwise if block is Cr block... */
	/* Cr first because of the earlier mixup */
	else if (bnum == 5)
	{
		/* Set dest to Cr plane of current pict image. */
		destPlane = m_current->GetCrPlane();

		/* Establish row size. */
		pitch = m_mb_width * 8;

		/* Calculate row,col for upper left pixel of block. */
		row = mb_row * 8;
		col = mb_col * 8;
	}
	/* Otherwise block is Cb block, and ... */
	else
	{
		/* Set dest to Cb plane of current pict image. */
		destPlane = m_current->GetCbPlane();

		/* Establish row size. */
		pitch = m_mb_width * 8;

		/* Calculate row,col for upper left pixel value of block. */
		row = mb_row * 8;
		col = mb_col * 8;
	}

	/*
	 * For each pixel in block, set to cropped reconstructed value from inverse
	 * dct.
	 */
	WriteDctBlockToPlane(&m_block.dct_recon, destPlane + (row * pitch + col), pitch);
}
#endif
