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
#ifndef __LWMOVIE_LAYER2_FIXEDPOINT_HPP__
#define __LWMOVIE_LAYER2_FIXEDPOINT_HPP__

#include "lwmovie_layer2.hpp"
#include "lwmovie_simdint.hpp"

#ifdef LWMOVIE_FIXEDPOINT

template<class TStorage>
struct lwmFixed32Base
{
	TStorage m_i;
};

#ifdef __cplusplus

template<int FracBits, class TStorage>
class lwmFixed32 : public lwmFixed32Base<TStorage>
{
public:
	static const int FRACTION_BITS = FracBits;

	TStorage RawData() const;

	lwmFixed32();
	explicit lwmFixed32(const TStorage &v);

	lwmFixed32(const float &f);
	lwmFixed32(const lwmUInt32 i);
	lwmFixed32(const double &f);

	lwmFixed32<FracBits, TStorage> & operator +=(const lwmFixed32<FracBits, TStorage> &rs);

	lwmFixed32<FracBits, TStorage> & operator -=(const lwmFixed32<FracBits, TStorage> &rs);

	lwmFixed32<FracBits, TStorage> operator +(const lwmFixed32<FracBits, TStorage> &rs) const;

	lwmFixed32<FracBits, TStorage> operator -(const lwmFixed32<FracBits, TStorage> &rs) const;

	lwmFixed32<FracBits, TStorage> operator -() const;

	template<int RSFracBits>
	lwmFixed32<FracBits + RSFracBits, TStorage> operator *(const lwmFixed32<RSFracBits, TStorage> &rs) const;

	template<int RSFracBits, int TargetBits>
	lwmFixed32<TargetBits, TStorage> MulTo(const lwmFixed32<RSFracBits, TStorage> &rs) const;

	lwmSInt32 MulAndRound(lwmSInt32 rs) const;
	lwmSInt32 Round() const;
	lwmFixed32<FracBits, TStorage> RShift(int rs) const;
	template<int RShiftBits>
	lwmFixed32<FracBits - RShiftBits, TStorage> ReduceFracPrecision() const;
	template<int LShiftBits>
	lwmFixed32<FracBits + LShiftBits, TStorage> IncreaseFracPrecision() const;
	template<int RShiftBits>
	lwmFixed32<FracBits + RShiftBits, TStorage> RShiftFixed() const;
	lwmSimdInt16<TStorage> LShiftRoundClamp(const lwmFixed32<FracBits, TStorage> &append, int rs) const;

	static lwmFixed32<FracBits, TStorage> Load(const lwmFixed32<FracBits, lwmSInt32> *source);
	void Store(lwmFixed32<FracBits, lwmSInt32> *dest) const;
};


#endif  // __cplusplus

#endif  // MP2DEC_FIXEDPOINT

#include "lwmovie_layer2_sseint.hpp"
#include "lwmovie_layer2_ssefloat.hpp"
#include "lwmovie_layer2_nosimd.hpp"

#endif

