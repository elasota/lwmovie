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
#ifndef __LWMOVIE_SIMD_DEFS_HPP__
#define __LWMOVIE_SIMD_DEFS_HPP__

#include "../common/lwmovie_coretypes.h"

namespace lwmovie
{
#ifdef LWMOVIE_SSE2
	static const lwmUInt32 SIMD_ALIGN		= 16;
	static const lwmUInt32 SIMD_BLOCK_SIZE	= 16;
#elif defined(LWMOVIE_NOSIMD)
	static const lwmUInt32 SIMD_ALIGN		= 1;
	static const lwmUInt32 SIMD_BLOCK_SIZE	= 1;
#endif

	inline lwmUInt32 PadToSIMD(lwmUInt32 value)
	{
		value += SIMD_ALIGN - 1;
		value -= (value % SIMD_ALIGN);
		return value;
	}
};

#endif
