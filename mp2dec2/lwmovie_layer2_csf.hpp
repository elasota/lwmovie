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
#ifndef __LWMOVIE_LAYER2_CSF_HPP__
#define __LWMOVIE_LAYER2_CSF_HPP__

#include "lwmovie_layer2_fixedpoint.hpp"
#include "lwmovie_layer2_fixedreal.hpp"
#include "lwmovie_layer2_xmath.hpp"

namespace lwmovie
{
	namespace layerii
	{
#ifdef LWMOVIE_FIXEDPOINT
		class lwmCompressedSF
		{
			static const int RSHIFT_BITS = 5;
			static const int MIN_RSHIFT = -2;
			static const int FRACTION_BITS = 32 - RSHIFT_BITS;	//27
			static const lwmUInt32 FRACTION_BITS_MASK = 0x7ffffff;
			static const lwmUInt32 RSHIFT_BITS_MASK = 0x1f;

			lwmUInt32 m_compressed;

		public:
			lwmCompressedSF();
			lwmFixedReal14 Mul(const lwmFixedReal29 &rs) const;
			static lwmCompressedSF FromPower(int exponent);
		};

#define LWMOVIE_CREATE_CSF(exp)	lwmCompressedSF::FromPower(exp)
#else
		typedef float lwmCompressedSF;
#define LWMOVIE_CREATE_CSF(exp)	(static_cast<lwmCompressedSF>(pow(0.5, static_cast<double>(exp) / 3.0 - 1.0)))
#endif
	}
}

#ifdef LWMOVIE_FIXEDPOINT

inline lwmovie::layerii::lwmCompressedSF::lwmCompressedSF()
{
}

inline lwmFixedReal14 lwmovie::layerii::lwmCompressedSF::Mul(const lwmFixedReal29 &rs) const
{
	lwmSInt32 frac = static_cast<lwmSInt32>(m_compressed & FRACTION_BITS_MASK);
	int shiftBase = static_cast<int>((m_compressed >> FRACTION_BITS) & RSHIFT_BITS_MASK);
	lwmSInt32 xmulHigh = static_cast<lwmSInt32>(lwmovie::xmath::EMul(rs.RawData(), frac) >> 32);	// So we can discard the lower 32 entirely
	lwmFixedReal14 result;
	int rshiftAmount = rs.FRACTION_BITS + FRACTION_BITS - 32 - result.FRACTION_BITS + MIN_RSHIFT + shiftBase;
	lwmSInt32 shifted = xmulHigh >> rshiftAmount;

	return lwmFixed32<0, lwmSInt32>(shifted).RShiftFixed<14>();
}

#endif

#endif
