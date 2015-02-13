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
#ifndef __LWMOVIE_LAYER2_FIXEDREAL_HPP__
#define __LWMOVIE_LAYER2_FIXEDREAL_HPP__

#include "lwmovie_layer2_fixedpoint.hpp"

#ifdef LWMOVIE_FIXEDPOINT

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage>::lwmFixed32(const TStorage &v)
{
	m_i = v << FracBits;
}

template<int FracBits, class TStorage>
inline TStorage lwmFixed32<FracBits, TStorage>::RawData() const
{
	return m_i;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage>::lwmFixed32()
{
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage>::lwmFixed32(const float &f)
{
	m_i = static_cast<lwmSInt32>(f * static_cast<float>(1 << FracBits));
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage>::lwmFixed32(const lwmUInt32 i)
{
	m_i = static_cast<lwmSInt32>(i << FracBits);
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage>::lwmFixed32(const double &f)
{
	m_i = static_cast<lwmSInt32>(f * static_cast<double>(1 << FracBits));
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> & lwmFixed32<FracBits, TStorage>::operator +=(const lwmFixed32<FracBits, TStorage> &rs)
{
	m_i += rs.m_i;
	return *this;
}


template<int FracBits, class TStorage>
template<int RSFracBits, int TargetBits>
inline lwmFixed32<TargetBits, TStorage> lwmFixed32<FracBits, TStorage>::MulTo(const lwmFixed32<RSFracBits, TStorage> &rs) const
{
	TStorage xmulHigh = lwmovie::xmath::EMulHigh(m_i, rs.RawData());
	lwmFixed32<TargetBits, TStorage> result;
	int rshift = FracBits + RSFracBits - 32 - TargetBits;
	result.m_i = xmulHigh >> (FracBits + RSFracBits - 32 - TargetBits);
	return result;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> & lwmFixed32<FracBits, TStorage>::operator -=(const lwmFixed32<FracBits, TStorage> &rs)
{
	m_i -= rs.m_i;
	return *this;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> lwmFixed32<FracBits, TStorage>::operator +(const lwmFixed32<FracBits, TStorage> &rs) const
{
	lwmFixed32<FracBits, TStorage> retVal;
	retVal.m_i = m_i + rs.m_i;
	return retVal;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> lwmFixed32<FracBits, TStorage>::operator -(const lwmFixed32<FracBits, TStorage> &rs) const
{
	lwmFixed32<FracBits, TStorage> retVal;
	retVal.m_i = m_i - rs.m_i;
	return retVal;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> lwmFixed32<FracBits, TStorage>::operator -() const
{
	lwmFixed32<FracBits, TStorage> retVal;
	retVal.m_i = -m_i;
	return retVal;
}

template<int FracBits, class TStorage>
template<int RSFracBits>
inline lwmFixed32<FracBits + RSFracBits, TStorage> lwmFixed32<FracBits, TStorage>::operator *(const lwmFixed32<RSFracBits, TStorage> &rs) const
{
	lwmFixed32<FracBits + RSFracBits, TStorage> retVal;
	retVal.m_i = m_i * rs.m_i;
	return retVal;
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> lwmFixed32<FracBits, TStorage>::RShift(int rs) const
{
	lwmFixed32<FracBits, TStorage> retVal;
	retVal.m_i = m_i >> rs;
	return retVal;
}

template<int FracBits, class TStorage>
template<int RShiftBits>
inline lwmFixed32<FracBits - RShiftBits, TStorage> lwmFixed32<FracBits, TStorage>::ReduceFracPrecision() const
{
	lwmFixed32<FracBits - RShiftBits, TStorage> retVal;
	retVal.m_i = m_i >> RShiftBits;
	return retVal;
}

template<int FracBits, class TStorage>
template<int LShiftBits>
inline lwmFixed32<FracBits + LShiftBits, TStorage> lwmFixed32<FracBits, TStorage>::IncreaseFracPrecision() const
{
	lwmFixed32<FracBits + LShiftBits, TStorage> retVal;
	retVal.m_i = m_i << LShiftBits;
	return retVal;
}

template<int FracBits, class TStorage>
template<int RShiftBits>
inline lwmFixed32<FracBits + RShiftBits, TStorage> lwmFixed32<FracBits, TStorage>::RShiftFixed() const
{
	lwmFixed32<FracBits + RShiftBits, TStorage> retVal;
	retVal.m_i = m_i;
	return retVal;
}

template<int FracBits, class TStorage>
inline lwmSimdInt16<TStorage> lwmFixed32<FracBits, TStorage>::LShiftRoundClamp(const lwmFixed32<FracBits, TStorage> &append, int rs) const
{
	TStorage firstHalf, secondHalf;
	if(rs < FracBits)
	{
		int diff = FracBits - rs;
		firstHalf = (m_i + TStorage(1 << (diff - 1))) >> diff;
		secondHalf = (append.m_i + TStorage(1 << (diff - 1))) >> diff;
	}
	else
	{
		firstHalf = m_i << (FracBits - rs);
		secondHalf = append.m_i << (FracBits - rs);
	}
	return lwmSimdInt16<TStorage>(firstHalf, secondHalf);
}

template<int FracBits, class TStorage>
inline lwmFixed32<FracBits, TStorage> lwmFixed32<FracBits, TStorage>::Load(const lwmFixed32<FracBits, lwmSInt32> *source)
{
	lwmFixed32 result;
	result.m_i = TStorage::Load(reinterpret_cast<const lwmSInt32*>(source));
	return result;
}

template<int FracBits, class TStorage>
inline void lwmFixed32<FracBits, TStorage>::Store(lwmFixed32<FracBits, lwmSInt32> *dest) const
{
	m_i.Store(reinterpret_cast<lwmSInt32*>(dest));
}

template<int FracBits, class TStorage>
inline int lwmFixed32<FracBits, TStorage>::Round() const
{
	return (m_i + (1 << (FracBits - 1))) >> FracBits;
}

typedef lwmFixed32<24, lwmSInt32> lwmFixedReal;
typedef lwmFixed32<0, lwmSInt32> lwmFixedReal0;
typedef lwmFixed32<31, lwmSInt32> lwmFixedReal31;
typedef lwmFixed32<30, lwmSInt32> lwmFixedReal30;
typedef lwmFixed32<29, lwmSInt32> lwmFixedReal29;
typedef lwmFixed32<27, lwmSInt32> lwmFixedReal27;
typedef lwmFixed32<24, lwmSInt32> lwmFixedReal24;
typedef lwmFixed32<22, lwmSInt32> lwmFixedReal22;
typedef lwmFixed32<18, lwmSInt32> lwmFixedReal18;
typedef lwmFixed32<17, lwmSInt32> lwmFixedReal17;
typedef lwmFixed32<14, lwmSInt32> lwmFixedReal14;

typedef lwmFixed32<14, lwmSimdSInt32> lwmSimdFixedReal14;
typedef lwmFixed32<22, lwmSimdSInt32> lwmSimdFixedReal22;
typedef lwmFixed32<27, lwmSimdSInt32> lwmSimdFixedReal27;
typedef lwmFixed32<29, lwmSimdSInt32> lwmSimdFixedReal29;

typedef lwmSimdSInt32 lwmSimdFixedRealRaw;

#define LWMOVIE_FIXEDREAL_LSAR_CLAMP(n0, n1, v) (n0).LShiftRoundClamp((n1), (v))
#define LWMOVIE_FIXEDREAL_MAR(n, v) (n).MulAndRound(v)
#define LWMOVIE_FIXEDREAL_RS(n, v) (n).RShift(v)
#define LWMOVIE_FIXEDREAL_MULTO(rsBits, targetBits) .MulTo<rsBits, targetBits>
#define LWMOVIE_FIXEDREAL_INCREASEFRACPRECISION(ls, bits) ((ls).IncreaseFracPrecision<bits>())
#define LWMOVIE_FIXEDREAL_REDUCEFRACPRECISION(ls, bits) ((ls).ReduceFracPrecision<bits>())
#define LWMOVIE_FIXEDREAL_CSF_MUL .Mul

#else           //!LWMOVIE_FIXEDPOINT

#pragma warning(disable:4305)   // Truncation from double to float

typedef float lwmFixedReal;
typedef float lwmFixedReal0;
typedef float lwmFixedReal31;
typedef float lwmFixedReal30;
typedef float lwmFixedReal29;
typedef float lwmFixedReal22;
typedef float lwmFixedReal24;
typedef float lwmFixedReal27;
typedef float lwmFixedReal18;
typedef float lwmFixedReal17;
typedef float lwmFixedReal14;

#define LWMOVIE_FIXEDREAL_LSAR_CLAMP(n0, n1, v) (n0).MulClamp((n1), static_cast<float>(1 << (v)))
#define LWMOVIE_FIXEDREAL_MAR(n, v) static_cast<lwmSInt32>((n) * static_cast<float>(v))
#define LWMOVIE_FIXEDREAL_RS(n, v) ((n) * (1.f / static_cast<float>(1 << (v))))
#define LWMOVIE_FIXEDREAL_MULTO(rsBits, targetBits) *
#define LWMOVIE_FIXEDREAL_INCREASEFRACPRECISION(n, v) (n)
#define LWMOVIE_FIXEDREAL_REDUCEFRACPRECISION(n, v) (n)
#define LWMOVIE_FIXEDREAL_CSF_MUL *

typedef lwmSimdRealFloat lwmSimdFixedReal14;
typedef lwmSimdRealFloat lwmSimdFixedReal29;
typedef lwmSimdRealFloat lwmSimdFixedReal27;
typedef lwmSimdRealFloat lwmSimdFixedReal22;
typedef lwmSimdRealFloat lwmSimdFixedRealRaw;

#endif

#endif