#ifndef __MP2DEC_INTERNAL_H__
#define __MP2DEC_INTERNAL_H__

#include "mp2dec.h"

#ifndef MP2DEC_FLOATINGPOINT
#include <intrin.h>

#pragma intrinsic(__emul)
#pragma warning(disable:4293)   // Negative shifts
#endif


#define MP2DEC_STATIC_CAST(t, n)        ((t)(n))


typedef struct mp2dec_bitstream
{
        const mp2dec_uint8	*bytes;
        mp2dec_uint			bitOffs;
        mp2dec_uint			totalDec;
} mp2dec_bitstream;


#ifndef MP2DEC_FLOATINGPOINT

template<int _FracBits>
inline int MP2Dec_Fixed32<_FracBits>::SLS(int v, int bits)
{
	if(bits == 0)
		return v;
	if(bits > 0)
		return v << bits;
	return v >> (-bits);
}

template<int _FracBits>
inline int MP2Dec_Fixed32<_FracBits>::SRS(int v, int bits)
{
	if(bits == 0)
		return v;
	if(bits > 0)
		return v >> bits;
	return v << (-bits);
}


template<int _FracBits>
inline int MP2Dec_Fixed32<_FracBits>::RawData() const
{
	return _i;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits>::MP2Dec_Fixed32()
{
}

template<int _FracBits>
template<int _RSFracBits>
inline MP2Dec_Fixed32<_FracBits>::MP2Dec_Fixed32(const MP2Dec_Fixed32<_RSFracBits> &rs)
{
	m_i = SLS(rs.RawData(), _FracBits - _RSFracBits);
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits>::MP2Dec_Fixed32(const float &f)
{
	m_i = static_cast<int>(f * static_cast<float>(1 << _FracBits));
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits>::MP2Dec_Fixed32(const unsigned int i)
{
	m_i = i << _FracBits;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits>::MP2Dec_Fixed32(const double &f)
{
	m_i = static_cast<int>(f * static_cast<double>(1 << _FracBits));
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits> & MP2Dec_Fixed32<_FracBits>::operator +=(const MP2Dec_Fixed32<_FracBits> &rs)
{
	m_i += rs.m_i;
	return *this;
}

template<int _FracBits>
template<int _RSFracBits>
inline MP2Dec_Fixed32<_FracBits> & MP2Dec_Fixed32<_FracBits>::operator *=(const MP2Dec_Fixed32<_RSFracBits> &rs)
{
	m_i = static_cast<int>(__emul(m_i, rs.RawData()) >> (_RSFracBits));
	return *this;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits> & MP2Dec_Fixed32<_FracBits>::operator -=(const MP2Dec_Fixed32<_FracBits> &rs)
{
	m_i -= rs.m_i;
	return *this;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits> MP2Dec_Fixed32<_FracBits>::operator +(const MP2Dec_Fixed32<_FracBits> &rs) const
{
	MP2Dec_Fixed32<_FracBits> retVal;
	retVal.m_i = m_i + rs.m_i;
	return retVal;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits> MP2Dec_Fixed32<_FracBits>::operator -(const MP2Dec_Fixed32<_FracBits> &rs) const
{
	MP2Dec_Fixed32<_FracBits> retVal;
	retVal.m_i = m_i - rs.m_i;
	return retVal;
}

template<int _FracBits>
inline MP2Dec_Fixed32<_FracBits> MP2Dec_Fixed32<_FracBits>::operator -() const
{
	MP2Dec_Fixed32<_FracBits> retVal;
	retVal.m_i = -m_i;
	return retVal;
}

template<int _FracBits>
template<int _RSFracBits>
inline MP2Dec_Fixed32<_FracBits> MP2Dec_Fixed32<_FracBits>::operator *(const MP2Dec_Fixed32<_RSFracBits> &rs) const
{
	MP2Dec_Fixed32<_FracBits> retVal;
	retVal.m_i = static_cast<int>(__emul(m_i, rs.RawData()) >> _RSFracBits);
	return retVal;
}

template<int _FracBits>
inline int MP2Dec_Fixed32<_FracBits>::MulAndRound(int rs) const
{
	int rv = static_cast<int>((__emul(m_i, rs) + (1 << (_FracBits - 1))) >> _FracBits);
	return rv;
}

template<int _FracBits>
inline int MP2Dec_Fixed32<_FracBits>::Round() const
{
	return (m_i + (1 << (_FracBits - 1))) >> _FracBits;
}

typedef MP2Dec_Fixed32<24> FixedReal;
typedef MP2Dec_Fixed32<17> FixedReal17;
#define MP2DEC_LSAR(n, v) (n).LShiftAndRound(scalefactor)
#else           //MP2DEC_FLOATINGPOINT

#pragma warning(disable:4305)   // Truncation from double to float

typedef float FixedReal;
typedef float FixedReal17;
#define MP2DEC_LSAR(n, v) static_cast<int>((n) * static_cast<float>(1 << (v)))
#define MP2DEC_MAR(n, v) static_cast<int>((n) * static_cast<float>(v))
#endif

void mp2dec_bitstream_init(mp2dec_bitstream *bitstream, const void *src);


#endif
