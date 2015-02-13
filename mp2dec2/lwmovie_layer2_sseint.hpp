#ifndef __LWMOVIE_LAYER2_SSEINT_HPP__
#define __LWMOVIE_LAYER2_SSEINT_HPP__

#include "lwmovie_layer2.hpp"

#ifdef LWMOVIE_SSE2

#include <emmintrin.h>

class lwmSimdSInt32
{
public:
	lwmSimdSInt32();
	lwmSimdSInt32(const lwmSimdSInt32 &other);
	explicit lwmSimdSInt32(const __m128i &v);
	explicit lwmSimdSInt32(lwmSInt32 fillVal);

	static lwmSimdSInt32 Load(const lwmSInt32 *source);
	void Store(lwmSInt32 *dest) const;
	lwmSimdSInt32 &operator +=(const lwmSimdSInt32 &rs);
	lwmSimdSInt32 operator +(const lwmSimdSInt32 &rs) const;
	lwmSimdSInt32 operator >>(int bits) const;
	lwmSimdSInt32 operator <<(int bits) const;

	const __m128i &RawSimd() const;

private:
	explicit lwmSimdSInt32(const lwmSInt32 *source);

	__m128i m_simdInt32;
};

template<>
class lwmSimdInt16<lwmSimdSInt32>
{
public:
	lwmSimdInt16(const lwmSimdSInt32 &a, const lwmSimdSInt32 &b);
	lwmSInt16 GetSimdSub(int index) const;

private:
	__m128i m_data;
};

#endif	// LWMOVIE_SSE2

#endif

#ifndef __LWMOVIE_LAYER2_SSEINT_INLINECODE_HPP__
#define __LWMOVIE_LAYER2_SSEINT_INLINECODE_HPP__

#ifdef LWMOVIE_SSE2

#include "lwmovie_layer2_xmath.hpp"

inline lwmSimdSInt32::lwmSimdSInt32()
{
}

inline lwmSimdSInt32::lwmSimdSInt32(const lwmSimdSInt32 &other)
	: m_simdInt32(other.m_simdInt32)
{
}

inline lwmSimdSInt32::lwmSimdSInt32(lwmSInt32 fillVal)
	: m_simdInt32(_mm_set1_epi32(fillVal))
{
}

inline lwmSimdSInt32::lwmSimdSInt32(const __m128i &v)
	: m_simdInt32(v)
{
}

inline lwmSimdSInt32 lwmSimdSInt32::Load(const lwmSInt32 *source)
{
	return lwmSimdSInt32(source);
}

inline void lwmSimdSInt32::Store(lwmSInt32 *dest) const
{
	_mm_store_si128(reinterpret_cast<__m128i*>(dest), m_simdInt32);
}


inline lwmSimdSInt32 &lwmSimdSInt32::operator +=(const lwmSimdSInt32 &rs)
{
	m_simdInt32 = _mm_add_epi32(m_simdInt32, rs.m_simdInt32);
	return *this;
}

inline lwmSimdSInt32 lwmSimdSInt32::operator +(const lwmSimdSInt32 &rs) const
{
	return lwmSimdSInt32(_mm_add_epi32(m_simdInt32, rs.m_simdInt32));
}

inline lwmSimdSInt32 lwmSimdSInt32::operator >>(int bits) const
{
	return lwmSimdSInt32(_mm_srai_epi32(m_simdInt32, bits));
}

inline lwmSimdSInt32 lwmSimdSInt32::operator <<(int bits) const
{
	return lwmSimdSInt32(_mm_slli_epi32(m_simdInt32, bits));
}


inline const __m128i &lwmSimdSInt32::RawSimd() const
{
	return m_simdInt32;
}

inline lwmSimdSInt32::lwmSimdSInt32(const lwmSInt32 *source)
	: m_simdInt32(_mm_load_si128(reinterpret_cast<const __m128i *>(source)))
{
}

inline lwmSimdInt16<lwmSimdSInt32>::lwmSimdInt16(const lwmSimdSInt32 &a, const lwmSimdSInt32 &b)
	: m_data(_mm_packs_epi32(a.RawSimd(), b.RawSimd()))
{
}

inline lwmSInt16 lwmSimdInt16<lwmSimdSInt32>::GetSimdSub(int index) const
{
	return m_data.m128i_i16[index];
}


inline lwmSimdSInt32 lwmovie::xmath::EMulHigh(const lwmSimdSInt32 &a, const lwmSimdSInt32 &b)
{
	__m128i lo_a_even = a.RawSimd();
	__m128i lo_a_odd = _mm_srli_si128(lo_a_even, 4);

	__m128i lo_b_even = b.RawSimd();
	__m128i lo_b_odd = _mm_srli_si128(lo_b_even, 4);

#ifdef LWMOVIE_SSE41
	__m128i even = _mm_mul_epi32(lo_a_even, lo_b_even);
	__m128i odd = _mm_mul_epi32(lo_a_odd, lo_b_odd);
#else
	__m128i hi_a_even = _mm_srai_epi32(lo_a_even, 31);
	__m128i hi_a_odd = _mm_srai_epi32(lo_a_odd, 31);
	__m128i hi_b_even = _mm_srai_epi32(lo_b_even, 31);
	__m128i hi_b_odd = _mm_srai_epi32(lo_b_odd, 31);

	__m128i even1 = _mm_mul_epu32(lo_a_even, lo_b_even);
	__m128i even2 = _mm_slli_epi64(_mm_mul_epu32(hi_a_even, lo_b_even), 32);
	__m128i even3 = _mm_slli_epi64(_mm_mul_epu32(lo_a_even, hi_b_even), 32);
	__m128i even = _mm_add_epi64(_mm_add_epi64(even1, even2), even3);

	__m128i odd1 = _mm_mul_epu32(lo_a_odd, lo_b_odd);
	__m128i odd2 = _mm_slli_epi64(_mm_mul_epu32(hi_a_odd, lo_b_odd), 32);
	__m128i odd3 = _mm_slli_epi64(_mm_mul_epu32(lo_a_odd, hi_b_odd), 32);
	__m128i odd = _mm_add_epi64(_mm_add_epi64(odd1, odd2), odd3);
#endif

	// Only want the high 32-bits
	__m128i evenHi = _mm_srli_epi64(even, 32);
	__m128i oddHi = _mm_slli_epi64(_mm_srli_epi64(odd, 32), 32);
	__m128i mixed = _mm_or_si128(evenHi, oddHi);

	return lwmSimdSInt32(mixed);
}

#endif	// LWMOVIE_SSE2

#endif

