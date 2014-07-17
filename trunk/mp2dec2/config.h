/*
Copyright (c) 2012 Eric Lasota


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __MP2DEC_CONFIG_H__
#define __MP2DEC_CONFIG_H__

// Define this to use floating point calculations instead of integer
#define MP2DEC_FLOATINGPOINT    1
//#define MP2DEC_ENABLE_SSE2              1
#define MP2DEC_SIMD_SIZE 16
#define MP2DEC_SIMD_ALIGN __declspec(align(16))

typedef size_t                  mp2dec_uint;                    // Natural size unsigned int
typedef ptrdiff_t               mp2dec_int;                             // Natural size signed int

typedef unsigned int    mp2dec_uint32;
typedef unsigned short  mp2dec_uint16;
typedef int                             mp2dec_int32;
typedef unsigned char   mp2dec_uint8;
typedef unsigned char   mp2dec_bool;
typedef short                   mp2dec_sint16;

// Change these to your linkage
#define MP2DEC_CPP_LINKAGE extern "C"
#define MP2DEC_C_LINKAGE


#define LWMPEG_ALIGN(n) __declspec(align(n))

#endif
