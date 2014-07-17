#ifndef __LWMOVIE_LAYER2_CSF_HPP__
#define __LWMOVIE_LAYER2_CSF_HPP__

#include "lwmovie_layer2_fixedpoint.hpp"
#include "lwmovie_layer2_fixedreal.hpp"
#include "lwmovie_layer2_xmath.hpp"

namespace lwmovie
{
	namespace layerii
	{
		class lwmCompressedSF
		{
			static const int RSHIFT_BITS = 5;
			static const int MIN_RSHIFT = -2;
			static const int FRACTION_BITS = 32 - RSHIFT_BITS;	//27
			static const lwmUInt32 FRACTION_BITS_MASK = 0x7ffffff;
			static const lwmUInt32 RSHIFT_BITS_MASK = 0x1f;

			lwmUInt32 m_compressed;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
			double debugValue;
#endif

		public:
			lwmCompressedSF();
			explicit lwmCompressedSF(int exponent);
			lwmFixedReal14 Mul(const lwmFixedReal29 &rs) const;
		};
	}
}

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

	result = lwmFixed32<0>(shifted).RShiftFixed<14>();
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	result.debugValue = debugValue * rs.debugValue;
#endif
	return result;
}

#endif
