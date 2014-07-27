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
#include <math.h>

#include "lwmovie_layer2_csf.hpp"

// Scalefactors are 0.5 ^ (i/3 - 1)
lwmovie::layerii::lwmCompressedSF::lwmCompressedSF(int exponent)
{
	int step = exponent % 3;
	int rshift;
	lwmUInt32 base;
	if(step == 0)
	{
		base = 1 << (FRACTION_BITS - 1);
		rshift = (exponent / 3) - 2;
	}
	else
	{
		base = static_cast<lwmUInt32>(pow(0.5, (double)step / 3.0) * (double)(1 << FRACTION_BITS));
		rshift = (exponent / 3) - 1;
	}
	m_compressed = ((rshift - MIN_RSHIFT) << FRACTION_BITS) | base;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = pow(0.5, (double)exponent / 3.0 - 1.0);
#endif
}
