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
#ifndef __LWMOVIE_LAYER2_XMATH_HPP__
#define __LWMOVIE_LAYER2_XMATH_HPP__

#include "lwmovie_layer2.hpp"

class lwmSimdSInt32;

namespace lwmovie
{
	namespace xmath
	{
		lwmSInt64 EMul(lwmSInt32 ls, lwmSInt32 rs);
		lwmSInt32 EMulHigh(lwmSInt32 ls, lwmSInt32 rs);
		lwmSimdSInt32 EMulHigh(const lwmSimdSInt32 &ls, const lwmSimdSInt32 &rs);
	}
}

#ifdef _WIN32
#include <intrin.h>

#pragma intrinsic(__emul)
#pragma warning(disable:4293)   // Negative shifts

inline lwmSInt32 lwmovie::xmath::EMulHigh(lwmSInt32 ls, lwmSInt32 rs)
{
	return static_cast<lwmSInt32>(__emul(ls, rs) >> 32);
}

inline lwmSInt64 lwmovie::xmath::EMul(lwmSInt32 ls, lwmSInt32 rs)
{
	return __emul(ls, rs);
}

#endif



#endif
