#ifndef __LWMOVIE_LAYER2_MATHOPS_HPP__
#define __LWMOVIE_LAYER2_MATHOPS_HPP__

#include "lwmovie_layer2.hpp"

#ifdef LWMOVIE_FIXEDPOINT

struct lwmFixed32Base
{
	lwmSInt32 m_i;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	double debugValue;
#endif
};

#ifdef __cplusplus

template<int FracBits>
class lwmFixed32 : public lwmFixed32Base
{
public:
	static const int FRACTION_BITS = FracBits;

	int RawData() const;

	lwmFixed32();
	explicit lwmFixed32(lwmSInt32 v);

	lwmFixed32(const float &f);
	lwmFixed32(const lwmUInt32 i);
	lwmFixed32(const double &f);

	lwmFixed32<FracBits> & operator +=(const lwmFixed32<FracBits> &rs);

	lwmFixed32<FracBits> & operator -=(const lwmFixed32<FracBits> &rs);

	lwmFixed32<FracBits> operator +(const lwmFixed32<FracBits> &rs) const;

	lwmFixed32<FracBits> operator -(const lwmFixed32<FracBits> &rs) const;

	lwmFixed32<FracBits> operator -() const;

	template<int RSFracBits>
	lwmFixed32<FracBits + RSFracBits> operator *(const lwmFixed32<RSFracBits> &rs) const;

	template<int RSFracBits, int TargetBits>
	lwmFixed32<TargetBits> MulTo(const lwmFixed32<RSFracBits> &rs) const;

	lwmSInt32 MulAndRound(lwmSInt32 rs) const;
	lwmSInt32 Round() const;
	lwmFixed32<FracBits> RShift(int rs) const;
	template<int RShiftBits>
	lwmFixed32<FracBits - RShiftBits> ReduceFracPrecision() const;
	template<int LShiftBits>
	lwmFixed32<FracBits + LShiftBits> IncreaseFracPrecision() const;
	template<int RShiftBits>
	lwmFixed32<FracBits + RShiftBits> RShiftFixed() const;
	lwmSInt32 LShiftAndRound(int rs) const;
};

#endif  // __cplusplus

#endif  // MP2DEC_FIXEDPOINT

#endif

