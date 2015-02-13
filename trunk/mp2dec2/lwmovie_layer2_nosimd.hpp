#ifndef __LWMOVIE_LAYER2_NOSIMD_HPP__
#define __LWMOVIE_LAYER2_NOSIMD_HPP__

#include "lwmovie_layer2.hpp"

#ifdef LWMOVIE_NOSIMD

class lwmSimdRealFloat
{
public:
	lwmSimdRealFloat();
	explicit lwmSimdRealFloat(float flt);
	explicit lwmSimdRealFloat(double flt);

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
	float RawFloat() const;

private:
	float m_float;
};

class lwmSimdSInt32
{
public:
	lwmSimdSInt32();
	lwmSimdSInt32(const lwmSimdSInt32 &other);
	explicit lwmSimdSInt32(lwmSInt32 v);

	static lwmSimdSInt32 Load(const lwmSInt32 *source);
	void Store(lwmSInt32 *dest) const;
	lwmSimdSInt32 &operator +=(const lwmSimdSInt32 &rs);
	lwmSimdSInt32 operator +(const lwmSimdSInt32 &rs) const;
	lwmSimdSInt32 operator >>(int bits) const;
	lwmSimdSInt32 operator <<(int bits) const;

	lwmSInt32 RawInt() const;

private:
	lwmSInt32 m_int32;
};

template<>
class lwmSimdInt16<lwmSimdRealFloat>
{
public:
	lwmSimdInt16(const lwmSimdRealFloat &a, const lwmSimdRealFloat &b);
	lwmSInt16 GetSimdSub(int index) const;
private:
	lwmSInt16 m_data[2];
};

template<>
class lwmSimdInt16<lwmSimdSInt32>
{
public:
	lwmSimdInt16(const lwmSimdSInt32 &a, const lwmSimdSInt32 &b);
	lwmSInt16 GetSimdSub(int index) const;

private:
	lwmSInt32 m_data[2];
};


inline lwmSimdRealFloat::lwmSimdRealFloat()
{
}

inline lwmSimdRealFloat::lwmSimdRealFloat(float flt)
	: m_float(flt)
{
}

inline lwmSimdRealFloat::lwmSimdRealFloat(double flt)
	: m_float(static_cast<float>(flt))
{
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator +(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(m_float + rs.m_float);
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator -(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(m_float - rs.m_float);
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator *(const lwmSimdRealFloat &rs) const
{
	return lwmSimdRealFloat(m_float * rs.m_float);
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator *(float rs) const
{
	return lwmSimdRealFloat(m_float * rs);
}

inline lwmSimdRealFloat &lwmSimdRealFloat::operator +=(const lwmSimdRealFloat &rs)
{
	m_float += rs.m_float;
	return *this;
}

inline lwmSimdRealFloat &lwmSimdRealFloat::operator -=(const lwmSimdRealFloat &rs)
{
	m_float -= rs.m_float;
	return *this;
}

inline lwmSimdRealFloat lwmSimdRealFloat::operator-() const
{
	return lwmSimdRealFloat(-m_float);
}

inline lwmSimdRealFloat lwmSimdRealFloat::Load(const float *in)
{
	return lwmSimdRealFloat(*in);
}

inline void lwmSimdRealFloat::Store(float *out) const
{
	*out = m_float;
}

inline lwmSimdInt16<lwmSimdRealFloat> lwmSimdRealFloat::MulClamp(const lwmSimdRealFloat &append, float mul) const
{
	return lwmSimdInt16<lwmSimdRealFloat>(lwmSimdRealFloat(m_float * mul), lwmSimdRealFloat(append.m_float * mul));
}

inline float lwmSimdRealFloat::RawFloat() const
{
	return m_float;
}

////////////////////////////////////////////////////////////////////////////////
inline lwmSimdSInt32::lwmSimdSInt32()
{
}

inline lwmSimdSInt32::lwmSimdSInt32(const lwmSimdSInt32 &other)
	: m_int32(other.m_int32)
{
}

inline lwmSimdSInt32::lwmSimdSInt32(lwmSInt32 v)
	: m_int32(v)
{
}

inline lwmSimdSInt32 lwmSimdSInt32::Load(const lwmSInt32 *source)
{
	return lwmSimdSInt32(*source);
}

inline void lwmSimdSInt32::Store(lwmSInt32 *dest) const
{
	*dest = m_int32;
}

inline lwmSimdSInt32 &lwmSimdSInt32::operator +=(const lwmSimdSInt32 &rs)
{
	m_int32 += rs.m_int32;
	return *this;
}

inline lwmSimdSInt32 lwmSimdSInt32::operator +(const lwmSimdSInt32 &rs) const
{
	return lwmSimdSInt32(m_int32 + rs.m_int32);
}

inline lwmSimdSInt32 lwmSimdSInt32::operator >>(int bits) const
{
	return lwmSimdSInt32(m_int32 >> bits);
}

inline lwmSimdSInt32 lwmSimdSInt32::operator <<(int bits) const
{
	return lwmSimdSInt32(m_int32 << bits);
}

inline lwmSInt32 lwmSimdSInt32::RawInt() const
{
	return m_int32;
}


////////////////////////////////////////////////////////////////////////////////

inline lwmSimdInt16<lwmSimdRealFloat>::lwmSimdInt16(const lwmSimdRealFloat &a, const lwmSimdRealFloat &b)
{
	float inFloats[2];
	inFloats[0] = a.RawFloat();
	inFloats[1] = b.RawFloat();
	for(int i=0;i<2;i++)
	{
		float flt = inFloats[i];
		if(flt > 32767.0f)
			m_data[i] = 32767;
		else if(flt < -32768)
			m_data[i] = -32768;
		else
			m_data[i] = static_cast<lwmSInt16>(flt);
	}
}

inline lwmSInt16 lwmSimdInt16<lwmSimdRealFloat>::GetSimdSub(int index) const
{
	return m_data[index];
}


inline lwmSimdInt16<lwmSimdSInt32>::lwmSimdInt16(const lwmSimdSInt32 &a, const lwmSimdSInt32 &b)
{
	lwmSInt32 ints[2];
	ints[0] = a.RawInt();
	ints[1] = b.RawInt();
	for(int i=0;i<2;i++)
	{
		if(ints[i] > 32767)
			m_data[i] = 32767;
		else if(ints[i] < -32768)
			m_data[i] = -32768;
		else
			m_data[i] = static_cast<lwmSInt16>(ints[i]);
	}
}

inline lwmSInt16 lwmSimdInt16<lwmSimdSInt32>::GetSimdSub(int index) const
{
	return m_data[index];
}

#endif	// LWMOVIE_NOSIMD

#endif

#ifndef __LWMOVIE_LAYER2_NOSIMD_INLINECODE_HPP__
#define __LWMOVIE_LAYER2_NOSIMD_INLINECODE_HPP__

#ifdef LWMOVIE_NOSIMD

#include "lwmovie_layer2_xmath.hpp"

inline lwmSimdSInt32 lwmovie::xmath::EMulHigh(const lwmSimdSInt32 &ls, const lwmSimdSInt32 &rs)
{
	return lwmSimdSInt32(lwmovie::xmath::EMulHigh(ls.RawInt(), rs.RawInt()));
}

#endif	// LWMOVIE_NOSIMD

#endif
