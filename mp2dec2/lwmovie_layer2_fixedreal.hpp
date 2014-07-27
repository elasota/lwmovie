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


template<int FracBits>
inline lwmFixed32<FracBits>::lwmFixed32(int v)
{
	m_i = v << FracBits;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = v;
#endif
}

template<int FracBits>
inline lwmSInt32 lwmFixed32<FracBits>::RawData() const
{
	return m_i;
}

template<int FracBits>
inline lwmFixed32<FracBits>::lwmFixed32()
{
}

template<int FracBits>
inline lwmFixed32<FracBits>::lwmFixed32(const float &f)
{
	m_i = static_cast<lwmSInt32>(f * static_cast<float>(1 << FracBits));
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = f;
#endif
}

template<int FracBits>
inline lwmFixed32<FracBits>::lwmFixed32(const lwmUInt32 i)
{
	m_i = static_cast<lwmSInt32>(i << FracBits);
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = i;
#endif
}

template<int FracBits>
inline lwmFixed32<FracBits>::lwmFixed32(const double &f)
{
	m_i = static_cast<lwmSInt32>(f * static_cast<double>(1 << FracBits));
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = f;
#endif
}

template<int FracBits>
inline lwmFixed32<FracBits> & lwmFixed32<FracBits>::operator +=(const lwmFixed32<FracBits> &rs)
{
	m_i += rs.m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue += rs.debugValue;
#endif
	return *this;
}


template<int FracBits>
template<int RSFracBits, int TargetBits>
inline lwmFixed32<TargetBits> lwmFixed32<FracBits>::MulTo(const lwmFixed32<RSFracBits> &rs) const
{
	lwmSInt32 xmulHigh = static_cast<lwmSInt32>(lwmovie::xmath::EMul(m_i, rs.RawData()) >> 32);
	lwmFixed32<TargetBits> result;
	int rshift = FracBits + RSFracBits - 32 - TargetBits;
	result.m_i = static_cast<lwmSInt32>(xmulHigh >> (FracBits + RSFracBits - 32 - TargetBits));
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	result.debugValue = debugValue * rs.debugValue;
#endif
	return result;
}

template<int FracBits>
inline lwmFixed32<FracBits> & lwmFixed32<FracBits>::operator -=(const lwmFixed32<FracBits> &rs)
{
	m_i -= rs.m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue -= rs.debugValue;
#endif
	return *this;
}

template<int FracBits>
inline lwmFixed32<FracBits> lwmFixed32<FracBits>::operator +(const lwmFixed32<FracBits> &rs) const
{
	lwmFixed32<FracBits> retVal;
	retVal.m_i = m_i + rs.m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue + rs.debugValue;
#endif
	return retVal;
}

template<int FracBits>
inline lwmFixed32<FracBits> lwmFixed32<FracBits>::operator -(const lwmFixed32<FracBits> &rs) const
{
	lwmFixed32<FracBits> retVal;
	retVal.m_i = m_i - rs.m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue - rs.debugValue;
#endif
	return retVal;
}

template<int FracBits>
inline lwmFixed32<FracBits> lwmFixed32<FracBits>::operator -() const
{
	lwmFixed32<FracBits> retVal;
	retVal.m_i = -m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = -debugValue;
#endif
	return retVal;
}

template<int FracBits>
template<int RSFracBits>
inline lwmFixed32<FracBits + RSFracBits> lwmFixed32<FracBits>::operator *(const lwmFixed32<RSFracBits> &rs) const
{
	lwmFixed32<FracBits + RSFracBits> retVal;
	retVal.m_i = m_i * rs.m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue * rs.debugValue;
#endif
	return retVal;
}

template<int FracBits>
inline lwmFixed32<FracBits> lwmFixed32<FracBits>::RShift(int rs) const
{
	lwmFixed32<FracBits> retVal;
	retVal.m_i = m_i >> rs;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue / static_cast<double>(1 << rs);
#endif
	return retVal;
}

template<int FracBits>
template<int RShiftBits>
inline lwmFixed32<FracBits - RShiftBits> lwmFixed32<FracBits>::ReduceFracPrecision() const
{
	lwmFixed32<FracBits - RShiftBits> retVal;
	retVal.m_i = m_i >> RShiftBits;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue;
#endif
	return retVal;
}

template<int FracBits>
template<int LShiftBits>
inline lwmFixed32<FracBits + LShiftBits> lwmFixed32<FracBits>::IncreaseFracPrecision() const
{
	lwmFixed32<FracBits + LShiftBits> retVal;
	retVal.m_i = m_i << LShiftBits;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue;
#endif
	return retVal;
}

template<int FracBits>
template<int RShiftBits>
inline lwmFixed32<FracBits + RShiftBits> lwmFixed32<FracBits>::RShiftFixed() const
{
	lwmFixed32<FracBits + RShiftBits> retVal;
	retVal.m_i = m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	retVal.debugValue = debugValue / static_cast<double>(1 << RShiftBits);
#endif
	return retVal;
}

template<int FracBits>
inline int lwmFixed32<FracBits>::LShiftAndRound(int rs) const
{
	if(rs < FracBits)
	{
		int diff = FracBits - rs;
		lwmSInt32 rounded = m_i + (1 << (diff - 1));
		return rounded >> diff;
	}
	return m_i << (FracBits - rs);
}

template<int FracBits>
inline int lwmFixed32<FracBits>::Round() const
{
	return (m_i + (1 << (FracBits - 1))) >> FracBits;
}

typedef lwmFixed32<24> lwmFixedReal;
typedef lwmFixed32<31> lwmFixedReal31;
typedef lwmFixed32<30> lwmFixedReal30;
typedef lwmFixed32<29> lwmFixedReal29;
typedef lwmFixed32<27> lwmFixedReal27;
typedef lwmFixed32<24> lwmFixedReal24;
typedef lwmFixed32<22> lwmFixedReal22;
typedef lwmFixed32<18> lwmFixedReal18;
typedef lwmFixed32<17> lwmFixedReal17;
typedef lwmFixed32<14> lwmFixedReal14;
#define LWMOVIE_FIXEDREAL_LSAR(n, v) (n).LShiftAndRound(v)
#define LWMOVIE_FIXEDREAL_MAR(n, v) (n).MulAndRound(v)
#define LWMOVIE_FIXEDREAL_RS(n, v) (n).RShift(v)
#define LWMOVIE_FIXEDREAL_MULTO(rsBits, targetBits) .MulTo<rsBits, targetBits>

#else           //!LWMOVIE_FIXEDPOINT

#pragma warning(disable:4305)   // Truncation from double to float

typedef float lwmFixedReal;
typedef float lwmFixedReal31;
typedef float lwmFixedReal30;
typedef float lwmFixedReal29;
typedef float lwmFixedReal24;
typedef float lwmFixedReal18;
typedef float lwmFixedReal17;
typedef float lwmFixedReal14;

#define LWMOVIE_FIXEDREAL_MULTO(rsBits, targetBits)	*

#define LWMOVIE_FIXEDREAL_LSAR(n, v) static_cast<lwmSInt32>((n) * static_cast<float>(1 << (v)))
#define LWMOVIE_FIXEDREAL_MAR(n, v) static_cast<lwmSInt32>((n) * static_cast<float>(v))
#define LWMOVIE_FIXEDREAL_RS(n, v) ((n) * (1.f / static_cast<float>(1 << (v))))

#endif

#endif