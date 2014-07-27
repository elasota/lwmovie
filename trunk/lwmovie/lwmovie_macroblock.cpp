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
