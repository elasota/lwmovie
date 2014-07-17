#ifndef __LWMOVIE_SIMD_DEFS_HPP__
#define __LWMOVIE_SIMD_DEFS_HPP__

#include "lwmovie_types.hpp"

namespace lwmovie
{
	static const lwmUInt32 SIMD_ALIGN	= 16;

	inline lwmUInt32 PadToSIMD(lwmUInt32 value)
	{
		value += SIMD_ALIGN - 1;
		value -= (value % SIMD_ALIGN);
		return value;
	}
};

#endif
