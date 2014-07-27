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

/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: normalized modified discrete cosine transform
           power of two length transform only [64 <= n ]
 last mod: $Id: mdct.c 16227 2009-07-08 06:58:46Z xiphmont $

 Original algorithm adapted long ago from _The use of multirate filter
 banks for coding of high quality digital audio_, by T. Sporer,
 K. Brandenburg and B. Edler, collection of the European Signal
 Processing Conference (EUSIPCO), Amsterdam, June 1992, Vol.1, pp
 211-214

 The below code implements an algorithm that no longer looks much like
 that presented in the paper, but the basic structure remains if you
 dig deep enough to see it.

 This module DOES NOT INCLUDE code to generate/apply the window
 function.  Everybody has their own weird favorite including me... I
 happen to like the properties of y=sin(.5PI*sin^2(x)), but others may
 vehemently disagree.

 ********************************************************************/

/* this can also be run as an integer transform by uncommenting a
   define in mdct.h; the integerization is a first pass and although
   it's likely stable for Vorbis, the dynamic range is constrained and
   roundoff isn't done (so it's noisy).  Consider it functional, but
   only a starting point.  There's no point on a machine with an FPU */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lwmovie_layer2_dspops.hpp"

typedef lwmFixedReal14 DATA_TYPE;
typedef lwmFixedReal14 REG_TYPE;

#define FLOAT_CONV(n) lwmFixedReal14(n)
#define cPI3_8 lwmFixedReal14(.38268343236508977175F)
#define cPI2_8 lwmFixedReal14(.70710678118654752441F)
#define cPI1_8 lwmFixedReal14(.92387953251128675613F)

#define MULT_NORM(x) ((x).ReduceFracPrecision<14>())
#define HALVE(x) ((x).RShift(1))

#define M_PI (3.1415926535897932384626433832795)

struct imdct32_lookup
{
	DATA_TYPE trig[80];
	int bitrev[16];
	DATA_TYPE scale;
};

static imdct32_lookup lookup;

/* build lookups for trig functions; also pre-figure scaling and
   some window function algebra. */

static void mdct_32_init()
{
	int n = 64;
	int n2=n>>1;
	int log2n=6;
	DATA_TYPE *T=lookup.trig;

	/* trig lookups... */

	for(int i=0;i<n/4;i++)
	{
		T[i*2]=FLOAT_CONV(cos((M_PI/n)*(4*i)));
		T[i*2+1]=FLOAT_CONV(-sin((M_PI/n)*(4*i)));
		T[n2+i*2]=FLOAT_CONV(cos((M_PI/(2*n))*(2*i+1)));
		T[n2+i*2+1]=FLOAT_CONV(sin((M_PI/(2*n))*(2*i+1)));
	}
	for(int i=0;i<n/8;i++)
	{
		T[n+i*2]=FLOAT_CONV(cos((M_PI/n)*(4*i+2))*.5);
		T[n+i*2+1]=FLOAT_CONV(-sin((M_PI/n)*(4*i+2))*.5);
	}

	/* bitreverse lookup... */
	{
		int mask=(1<<(log2n-1))-1,i;
		int msb=1<<(log2n-2);
		for(i=0;i<n/8;i++)
		{
			int acc=0;
			for(int j=0;msb>>j;j++)
				if((msb>>j)&i)
					acc|=1<<j;
			lookup.bitrev[i*2]=((~acc)&mask)-1;
			lookup.bitrev[i*2+1]=acc;
		}
	}
	lookup.scale=FLOAT_CONV(4.f/n);
}

/* 8 point butterfly (in place, 4 register) */
static void mdct_butterfly_8(DATA_TYPE *x)
{
	REG_TYPE r0   = x[6] + x[2];
	REG_TYPE r1   = x[6] - x[2];
	REG_TYPE r2   = x[4] + x[0];
	REG_TYPE r3   = x[4] - x[0];

	x[6] = r0   + r2;
	x[4] = r0   - r2;

	r0   = x[5] - x[1];
	r2   = x[7] - x[3];
	x[0] = r1   + r0;
	x[2] = r1   - r0;

	r0   = x[5] + x[1];
	r1   = x[7] + x[3];
	x[3] = r2   + r3;
	x[1] = r2   - r3;
	x[7] = r1   + r0;
	x[5] = r1   - r0;
}

/* 16 point butterfly (in place, 4 register) */
static void mdct_butterfly_16(DATA_TYPE *x)
{
	REG_TYPE r0     = x[1]  - x[9];
	REG_TYPE r1     = x[0]  - x[8];

	x[8]  += x[0];
	x[9]  += x[1];
	x[0]   = MULT_NORM((r0   + r1) * cPI2_8);
	x[1]   = MULT_NORM((r0   - r1) * cPI2_8);

	r0     = x[3]  - x[11];
	r1     = x[10] - x[2];
	x[10] += x[2];
	x[11] += x[3];
	x[2]   = r0;
	x[3]   = r1;

	r0     = x[12] - x[4];
	r1     = x[13] - x[5];
	x[12] += x[4];
	x[13] += x[5];
	x[4]   = MULT_NORM((r0   - r1) * cPI2_8);
	x[5]   = MULT_NORM((r0   + r1) * cPI2_8);

	r0     = x[14] - x[6];
	r1     = x[15] - x[7];
	x[14] += x[6];
	x[15] += x[7];
	x[6]  = r0;
	x[7]  = r1;

	mdct_butterfly_8(x);
	mdct_butterfly_8(x+8);
}

/* 32 point butterfly (in place, 4 register) */
static void mdct_butterfly_32(DATA_TYPE *x)
{
	REG_TYPE r0     = x[30] - x[14];
	REG_TYPE r1     = x[31] - x[15];

	x[30] +=         x[14];
	x[31] +=         x[15];
	x[14]  =         r0;
	x[15]  =         r1;

	r0     = x[28] - x[12];
	r1     = x[29] - x[13];
	x[28] +=         x[12];
	x[29] +=         x[13];
	x[12]  = MULT_NORM( r0 * cPI1_8  -  r1 * cPI3_8 );
	x[13]  = MULT_NORM( r0 * cPI3_8  +  r1 * cPI1_8 );

	r0     = x[26] - x[10];
	r1     = x[27] - x[11];
	x[26] +=         x[10];
	x[27] +=         x[11];
	x[10]  = MULT_NORM(( r0  - r1 ) * cPI2_8);
	x[11]  = MULT_NORM(( r0  + r1 ) * cPI2_8);

	r0     = x[24] - x[8];
	r1     = x[25] - x[9];
	x[24] += x[8];
	x[25] += x[9];
	x[8]   = MULT_NORM( r0 * cPI3_8  -  r1 * cPI1_8 );
	x[9]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );

	r0     = x[22] - x[6];
	r1     = x[7]  - x[23];
	x[22] += x[6];
	x[23] += x[7];
	x[6]   = r1;
	x[7]   = r0;

	r0     = x[4]  - x[20];
	r1     = x[5]  - x[21];
	x[20] += x[4];
	x[21] += x[5];
	x[4]   = MULT_NORM( r1 * cPI1_8  +  r0 * cPI3_8 );
	x[5]   = MULT_NORM( r1 * cPI3_8  -  r0 * cPI1_8 );

	r0     = x[2]  - x[18];
	r1     = x[3]  - x[19];
	x[18] += x[2];
	x[19] += x[3];
	x[2]   = MULT_NORM(( r1  + r0 ) * cPI2_8);
	x[3]   = MULT_NORM(( r1  - r0 ) * cPI2_8);

	r0     = x[0]  - x[16];
	r1     = x[1]  - x[17];
	x[16] += x[0];
	x[17] += x[1];
	x[0]   = MULT_NORM( r1 * cPI3_8  +  r0 * cPI1_8 );
	x[1]   = MULT_NORM( r1 * cPI1_8  -  r0 * cPI3_8 );

	mdct_butterfly_16(x);
	mdct_butterfly_16(x+16);
}

static void mdct_bitreverse(DATA_TYPE *x)
{
	int        n       = 64;
	int       *bit     = lookup.bitrev;
	DATA_TYPE *w0      = x;
	DATA_TYPE *w1      = x = w0+(n>>1);
	DATA_TYPE *T       = lookup.trig+n;

	do
	{
		DATA_TYPE *x0    = x+bit[0];
		DATA_TYPE *x1    = x+bit[1];

		REG_TYPE  r0     = x0[1]  - x1[1];
		REG_TYPE  r1     = x0[0]  + x1[0];
		REG_TYPE  r2     = MULT_NORM(r1     * T[0]   + r0 * T[1]);
		REG_TYPE  r3     = MULT_NORM(r1     * T[1]   - r0 * T[0]);

		w1    -= 4;

		r0     = HALVE(x0[1] + x1[1]);
		r1     = HALVE(x0[0] - x1[0]);

		w0[0]  = r0     + r2;
		w1[2]  = r0     - r2;
		w0[1]  = r1     + r3;
		w1[3]  = r3     - r1;

		x0     = x+bit[2];
		x1     = x+bit[3];

		r0     = x0[1]  - x1[1];
		r1     = x0[0]  + x1[0];
		r2     = MULT_NORM(r1     * T[2]   + r0 * T[3]);
		r3     = MULT_NORM(r1     * T[3]   - r0 * T[2]);

		r0     = HALVE(x0[1] + x1[1]);
		r1     = HALVE(x0[0] - x1[0]);

		w0[2]  = r0     + r2;
		w1[0]  = r0     - r2;
		w0[3]  = r1     + r3;
		w1[1]  = r3     - r1;

		T     += 4;
		bit   += 4;
		w0    += 4;

	} while(w0<w1);
}

void lwmovie::layerii::IMDCT32(DATA_TYPE out[64], const DATA_TYPE in[32])
{
	int n=64;
	int n2=n>>1;
	int n4=n>>2;

	/* rotate */

	const DATA_TYPE *iX = in+n2-7;
	DATA_TYPE *oX = out+n2+n4;
	DATA_TYPE *T  = lookup.trig+n4;

	do
	{
		oX         -= 4;
		oX[0]       = MULT_NORM(-iX[2] * T[3] - iX[0]  * T[2]);
		oX[1]       = MULT_NORM (iX[0] * T[3] - iX[2]  * T[2]);
		oX[2]       = MULT_NORM(-iX[6] * T[1] - iX[4]  * T[0]);
		oX[3]       = MULT_NORM (iX[4] * T[1] - iX[6]  * T[0]);
		iX         -= 8;
		T          += 4;
	} while(iX>=in);

	iX            = in+n2-8;
	oX            = out+n2+n4;
	T             = lookup.trig+n4;

	do
	{
		T          -= 4;
		oX[0]       =  MULT_NORM (iX[4] * T[3] + iX[6] * T[2]);
		oX[1]       =  MULT_NORM (iX[4] * T[2] - iX[6] * T[3]);
		oX[2]       =  MULT_NORM (iX[0] * T[1] + iX[2] * T[0]);
		oX[3]       =  MULT_NORM (iX[0] * T[0] - iX[2] * T[1]);
		iX         -= 8;
		oX         += 4;
	} while(iX>=in);

	mdct_butterfly_32(out+n2);
	mdct_bitreverse(out);

	/* roatate + window */
	
	{
		DATA_TYPE *oX1=out+n2+n4;
		DATA_TYPE *oX2=out+n2+n4;
		DATA_TYPE *iX =out;
		T             =lookup.trig+n2;

		do
		{
			oX1-=4;

			oX1[3]  =  MULT_NORM (iX[0] * T[1] - iX[1] * T[0]);
			oX2[0]  = -MULT_NORM (iX[0] * T[0] + iX[1] * T[1]);

			oX1[2]  =  MULT_NORM (iX[2] * T[3] - iX[3] * T[2]);
			oX2[1]  = -MULT_NORM (iX[2] * T[2] + iX[3] * T[3]);

			oX1[1]  =  MULT_NORM (iX[4] * T[5] - iX[5] * T[4]);
			oX2[2]  = -MULT_NORM (iX[4] * T[4] + iX[5] * T[5]);

			oX1[0]  =  MULT_NORM (iX[6] * T[7] - iX[7] * T[6]);
			oX2[3]  = -MULT_NORM (iX[6] * T[6] + iX[7] * T[7]);

			oX2+=4;
			iX    +=   8;
			T     +=   8;
		} while(iX<oX1);

		iX=out+n2+n4;
		oX1=out+n4;
		oX2=oX1;

		do
		{
			oX1-=4;
			iX-=4;

			oX2[0] = -(oX1[3] = iX[3]);
			oX2[1] = -(oX1[2] = iX[2]);
			oX2[2] = -(oX1[1] = iX[1]);
			oX2[3] = -(oX1[0] = iX[0]);

			oX2+=4;
		} while(oX2<iX);

		iX=out+n2+n4;
		oX1=out+n2+n4;
		oX2=out+n2;
		do
		{
			oX1-=4;
			oX1[0]= iX[3];
			oX1[1]= iX[2];
			oX1[2]= iX[1];
			oX1[3]= iX[0];
			iX+=4;
		} while(oX1>oX2);
	}
}


namespace lwmovie
{
	struct lwmIMDCTStaticInitializer
	{
		lwmIMDCTStaticInitializer()
		{
			mdct_32_init();
		}

		static lwmIMDCTStaticInitializer instance;
	};
	lwmIMDCTStaticInitializer lwmIMDCTStaticInitializer::instance;
}
