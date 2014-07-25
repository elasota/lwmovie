/*
** Copyright (C) 2004 Eric Lasota/Orbiter Productions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __SB3_YUV_H__
#define __SB3_YUV_H__

#include "inline.h"

// CCIR 601-1 YCbCr colorspace.

#define R2Y(n)  (n * 19595)					// 0.299
#define R2CB(n) (n * (-11056))				// -0.1687
#define R2CR(n) (n<<15)						// 0.5

#define G2Y(n)  (n * 38470)					// 0.587
#define G2CB(n) (n * (-21712))				// -0.3313
#define G2CR(n) (n * (-27440))				// -0.4187

#define B2Y(n)  (n * 7471)					// 0.114
#define B2CB(n) (n<<15)						// 0.5
#define B2CR(n) (n * (-5328))				// -0.0813

#define CB2G(n) ( ((int)n-128)*(-22554) )	// -0.34414
#define CB2B(n) ( ((int)n-128)*112853   )	// 1.722

#define CR2R(n) ( ((int)n-128)*91881    )	// 1.402
#define CR2G(n) ( ((int)n-128)*(-46802) )	// -0.71414


rc_inline void rgb2yuv(unsigned char r, unsigned char g, unsigned char b,
		  unsigned char *yout, unsigned char *uout, unsigned char *vout)
{

	int y;
	int cb;
	int cr;

	y = (R2Y(r) + G2Y(g) + B2Y(b)) >> 16;
	y = (y < 0) ? 0 : ((y > 255) ? 255 : y);

	cb = ((R2CB(r) + G2CB(g) + B2CB(b)) >> 16) + 128;
	cb = (cb < 0) ? 0 : ((cb > 255) ? 255 : cb);

	cr = ((R2CR(r) + G2CR(g) + B2CR(b)) >> 16) + 128;
	cr = (cr < 0) ? 0 : ((cr > 255) ? 255 : cr);

	*yout = (unsigned char)y;
	*uout = (unsigned char)cb;
	*vout = (unsigned char)cr;
}


rc_inline void yuv2rgb(unsigned char y, unsigned char u, unsigned char v,
			  unsigned char *rout, unsigned char *gout, unsigned char *bout)
{
	int r;
	int g;
	int b;

	r = y + (CR2R(v)>>16);
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);

	g = y + ((CB2G(u) + CR2G(v))>>16);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);

	b = y + (CB2B(u)>>16);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	*rout = (unsigned char)r;
	*gout = (unsigned char)g;
	*bout = (unsigned char)b;
}

#endif
