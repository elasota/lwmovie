/*
 * Copyright (c) 2015 Eric Lasota
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
#ifndef __LWMOVIE_CORETYPES_H__
#define __LWMOVIE_CORETYPES_H__

#include "lwmovie_config.h"

// This file is used by the C API

#include <stdint.h>

typedef int64_t lwmSInt64;
typedef int32_t lwmSInt32;
typedef int16_t lwmSInt16;
typedef int8_t lwmSInt8;
typedef uint64_t lwmUInt64;
typedef uint32_t lwmUInt32;
typedef uint16_t lwmUInt16;
typedef uint8_t lwmUInt8;
typedef size_t lwmLargeUInt;
typedef ptrdiff_t lwmLargeSInt;
typedef float lwmFloat32;

typedef int_fast32_t lwmFastSInt32;
typedef int_fast16_t lwmFastSInt16;
typedef int_fast8_t lwmFastSInt8;
typedef uint_fast32_t lwmFastUInt32;
typedef uint_fast16_t lwmFastUInt16;
typedef uint_fast8_t lwmFastUInt8;

#if defined(LWMOVIE_SSE2)
	#define LWMOVIE_FIXEDREAL_SIMD_WIDTH		4
	#define LWMOVIE_FIXEDREAL_SIMD_ALIGNMENT	16
	#define LWMOVIE_FIXEDREAL_SIMD_ALIGN_ATTRIB __declspec(align(16))
#elif defined(LWMOVIE_NOSIMD)
	#define LWMOVIE_FIXEDREAL_SIMD_WIDTH		1
	#define LWMOVIE_FIXEDREAL_SIMD_ALIGNMENT	1
	#define LWMOVIE_FIXEDREAL_SIMD_ALIGN_ATTRIB
#else
	#error "You must define an SIMD option in lwmovie_config.h"
#endif

#endif
