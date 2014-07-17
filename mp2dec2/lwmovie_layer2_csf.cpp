#include <math.h>

#include "lwmovie_layer2_csf.hpp"

// Scalefactors are 0.5 ^ (i/3 - 1)
lwmovie::layerii::lwmCompressedSF::lwmCompressedSF(int exponent)
{
	int step = exponent % 3;
	int rshift;
	lwmUInt32 base;
	if(step == 0)
	{
		base = 1 << (FRACTION_BITS - 1);
		rshift = (exponent / 3) - 2;
	}
	else
	{
		base = static_cast<lwmUInt32>(pow(0.5, (double)step / 3.0) * (double)(1 << FRACTION_BITS));
		rshift = (exponent / 3) - 1;
	}
	m_compressed = ((rshift - MIN_RSHIFT) << FRACTION_BITS) | base;
#ifdef LWMOVIE_DEBUG_FIXEDPOINT
	debugValue = pow(0.5, (double)exponent / 3.0 - 1.0);
#endif
}
