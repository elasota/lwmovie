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
#ifndef __LWMOVIE_LAYER2_DSPOPS_HPP__
#define __LWMOVIE_LAYER2_DSPOPS_HPP__

#include "lwmovie_layer2_xmath.hpp"
#include "lwmovie_layer2_constants.hpp"
#include "lwmovie_layer2_fixedreal.hpp"

namespace lwmovie
{
	namespace layerii
	{
		void SubBandSynthesis(lwmSInt16 *output, lwmFixedReal22 accumulators[lwmovie::layerii::FILTER_SIZE][lwmovie::layerii::NUM_SUBBANDS], const lwmFixedReal14 coeffs[lwmovie::layerii::NUM_SUBBANDS], int currentRotator, lwmFastUInt8 stride);
		void IMDCT32(lwmFixedReal14 out[64], const lwmFixedReal14 in[32]);
	}
}



#endif
