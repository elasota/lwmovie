/********************************************************************
*                                                                  *
* THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
* USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
* GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
* IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
*                                                                  *
* THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
* by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
*                                                                  *
********************************************************************

function:
last mod: $Id$

********************************************************************/

#include <string.h>
#include "internal.h"
#include "dct.h"

/*Performs an inverse 8 point Type-II DCT transform.
The output is scaled by a factor of 2 relative to the orthonormal version of
the transform.
_y: The buffer to store the result in.
Data will be placed in every 8th entry (e.g., in a column of an 8x8
block).
_x: The input coefficients.
The first 8 entries are used (e.g., from a row of an 8x8 block).*/
static void idct8_b2(ogg_int16_t *_y, const ogg_int16_t _x[8]) {
	int c0 = _x[0];
	int d4 = _x[1];
	int c2 = _x[2];
	int d6 = _x[3];
	int c1 = _x[4];
	int d5 = _x[5];
	int c3 = _x[6];
	int d7 = _x[7];

	// odd stage 4
	int c4 = d4;
	int c5 = d5 + d6;
	int c7 = d5 - d6;
	int c6 = d7;

	// odd stage 3
	int b4 = c4 + c5;
	int b5 = c4 - c5;
	int b6 = c6 + c7;
	int b7 = c6 - c7;

	// even stage 3
	int b0 = c0 + c1;
	int b1 = c0 - c1;
	int b2 = c2 + (c2 >> 2) + (c3 >> 1);
	int b3 = (c2 >> 1) - c3 - (c3 >> 2);

	// odd stage 2
	int a4 = (b7 >> 2) + b4 + (b4 >> 2) - (b4 >> 4);
	int a7 = (b4 >> 2) - b7 - (b7 >> 2) + (b7 >> 4);
	int a5 = b5 - b6 + (b6 >> 2) + (b6 >> 4);
	int a6 = b6 + b5 - (b5 >> 2) - (b5 >> 4);

	// even stage 2
	int a0 = b0 + b2;
	int a1 = b1 + b3;
	int a2 = b1 - b3;
	int a3 = b0 - b2;

	// stage 1
	int o0 = a0 + a4;
	int o1 = a1 + a5;
	int o2 = a2 + a6;
	int o3 = a3 + a7;
	int o4 = a3 - a7;
	int o5 = a2 - a6;
	int o6 = a1 - a5;
	int o7 = a0 - a4;

	// output
	_y[0 << 3] = o0;
	_y[1 << 3] = o1;
	_y[2 << 3] = o2;
	_y[3 << 3] = o3;
	_y[4 << 3] = o4;
	_y[5 << 3] = o5;
	_y[6 << 3] = o6;
	_y[7 << 3] = o7;
}

/*Performs an inverse 8x8 Type-II DCT transform.
The input is assumed to be scaled by a factor of 4 relative to orthonormal
version of the transform.
_y: The buffer to store the result in.
This may be the same as _x.
_x: The input coefficients.*/
static void oc_idct8x8_slow_lwmovie(ogg_int16_t _y[64], ogg_int16_t _x[64]) {
	ogg_int16_t w[64];
	int         i;
	/*Transform rows of x into columns of w.*/
	for (i = 0; i<8; i++)idct8_b2(w + i, _x + i * 8);
	/*Transform rows of w into columns of y.*/
	for (i = 0; i<8; i++)idct8_b2(_y + i, w + i * 8);
	/*Adjust for the scale factor.*/
	for (i = 0; i<64; i++)_y[i] = (ogg_int16_t)(_y[i] + 16 >> 5);
	/*Clear input data for next block.*/
	for (i = 0; i<64; i++)_x[i] = 0;
}

/*Performs an inverse 8x8 Type-II DCT transform.
The input is assumed to be scaled by a factor of 4 relative to orthonormal
version of the transform.*/
void oc_idct8x8_c(ogg_int16_t _y[64], ogg_int16_t _x[64], int _last_zzi) {
	/*_last_zzi is subtly different from an actual count of the number of
	coefficients we decoded for this block.
	It contains the value of zzi BEFORE the final token in the block was
	decoded.
	In most cases this is an EOB token (the continuation of an EOB run from a
	previous block counts), and so this is the same as the coefficient count.
	However, in the case that the last token was NOT an EOB token, but filled
	the block up with exactly 64 coefficients, _last_zzi will be less than 64.
	Provided the last token was not a pure zero run, the minimum value it can
	be is 46, and so that doesn't affect any of the cases in this routine.
	However, if the last token WAS a pure zero run of length 63, then _last_zzi
	will be 1 while the number of coefficients decoded is 64.
	Thus, we will trigger the following special case, where the real
	coefficient count would not.
	Note also that a zero run of length 64 will give _last_zzi a value of 0,
	but we still process the DC coefficient, which might have a non-zero value
	due to DC prediction.
	Although convoluted, this is arguably the correct behavior: it allows us to
	use a smaller transform when the block ends with a long zero run instead
	of a normal EOB token.
	It could be smarter... multiple separate zero runs at the end of a block
	will fool it, but an encoder that generates these really deserves what it
	gets.
	Needless to say we inherited this approach from VP3.*/
	/*Then perform the iDCT.*/
	//if (_last_zzi <= 3)oc_idct8x8_3(_y, _x);
	//else if (_last_zzi <= 10)oc_idct8x8_10(_y, _x);
	//else 
	oc_idct8x8_slow_lwmovie(_y, _x);
}
