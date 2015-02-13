#ifndef __LWMOVIE_LAYER2_SSEFLOAT_HPP__
#define __LWMOVIE_LAYER2_SSEFLOAT_HPP__

#include "lwmovie_layer2.hpp"

#ifdef LWMOVIE_SSE2

#include <xmmintrin.h>

class lwmSimdRealFloat
{
public:
	lwmSimdRealFloat();
	explicit lwmSimdRealFloat(float flt);
	explicit lwmSimdRealFloat(double flt);
	explicit lwmSimdRealFloat(__m128 simd);

	lwmSimdRealFloat operator +(const lwmSimdRealFloat &rs) const;
	lwmSimdRealFloat operator -(const lwmSimdRealFloat &rs) const;
	lwmSimdRealFloat operator *(const lwmSimdRealFloat &rs) const;
	lwmSimdRealFloat operator *(float rs) const;
	lwmSimdRealFloat &operator +=(const lwmSimdRealFloat &rs);
	lwmSimdRealFloat &operator -=(const lwmSimdRealFloat &rs);
	lwmSimdRealFloat operator-() const;

	static lwmSimdRealFloat Load(const float *in);
	void Store(float *out) const;

	lwmSimdInt16<lwmSimdRealFloat> MulClamp(const lwmSimdRealFloat &append, float mul) const;
	const __m128 &RawSimd() const;

private:
	__m128 m_simdFloats;
};


template<>
class lwmSimdInt16<lwmSimdRealFloat>
{
public:
	lwmSimdInt16(const lwmSimdRealFloat &a, const lwmSimdRealFloat &b);
	lwmSInt16 GetSimdSub(int index) const;
private:
	__m128i m_data;
};

inline lwmSimdInt16<lwmSimdRealFloat>::lwmSimdInt16(const lwmSimdRealFloat &a, const lwmSimdRealFloat &b)
	: m_data(_mm_packs_epi32(_mm_cvttps_epi32(a.RawSimd()), _mm_cvttps_epi32(b.RawSimd())))
{
}

inline lwmSInt16 lwmSimdInt16<lwmSimdRealFloat>::GetSimdSub(int index) const
{
	return m_data.m128i_i16[index];
}

inline lwmSimdRealFloat::lwmSimdRealFloat()
{
}

inline lwmSimdRealFloat::lwmSimdRealFloat(float flt)
	: m_simdFloats(_mm_set_ps1(flt))
{
}

inline lwmSimdRealFloat::lwmSimdRealFloat(double flt)
	: m_simdFloats(_mm_set_ps1(static_cast<float>(flt)))
{
}

inline lwmSimdRealFloat::lwmSimdRealFloat(__m128 simd)
	: m_simdFloats(simd)
{
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator +(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(_mm_add_ps(m_simdFloats, rs.m_simdFloats));
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator -(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(_mm_sub_ps(m_simdFloats, rs.m_simdFloats));
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator *(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(_mm_mul_ps(m_simdFloats, rs.m_simdFloats));
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator *(float rs) const
{
	return lwmSimdRealFloat(_mm_mul_ps(m_simdFloats, _mm_set_ps1(rs)));
}

inline lwmSimdRealFloat &lwmSimdRealFloat::operator +=(const lwmSimdRealFloat &rs)
{
	m_simdFloats = _mm_add_ps(m_simdFloats, rs.m_simdFloats);
	return *this;
}

inline lwmSimdRealFloat &lwmSimdRealFloat::operator -=(const lwmSimdRealFloat &rs)
{
	m_simdFloats = _mm_sub_ps(m_simdFloats, rs.m_simdFloats);
	return *this;
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator-() const
{
	return lwmSimdRealFloat(_mm_sub_ps(_mm_setzero_ps(), m_simdFloats));
}

inline lwmSimdRealFloat lwmSimdRealFloat::Load(const float *in)
{
	return lwmSimdRealFloat(_mm_load_ps(in));
}

inline void lwmSimdRealFloat::Store(float *out) const
{
	_mm_store_ps(out, m_simdFloats);
}

inline lwmSimdInt16<lwmSimdRealFloat> lwmSimdRealFloat::MulClamp(const lwmSimdRealFloat &append, float mul) const
{
	__m128 simdMul = _mm_set_ps1(mul);
	return lwmSimdInt16<lwmSimdRealFloat>(lwmSimdRealFloat(_mm_mul_ps(m_simdFloats, simdMul)), lwmSimdRealFloat(_mm_mul_ps(append.m_simdFloats, simdMul)));
}

inline const __m128 &lwmSimdRealFloat::RawSimd() const
{
	return m_simdFloats;
}

#endif

#endif
