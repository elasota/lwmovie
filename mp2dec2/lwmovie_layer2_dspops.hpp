#ifndef __LWMOVIE_LAYER2_DSPOPS_HPP__
#define __LWMOVIE_LAYER2_DSPOPS_HPP__

#include "lwmovie_layer2_xmath.hpp"
#include "lwmovie_layer2_constants.hpp"
#include "lwmovie_layer2_fixedreal.hpp"

namespace lwmovie
{
	namespace layerii
	{
		void SubBandSynthesis(lwmSInt16 *output, lwmFixedReal22 accumulators[lwmovie::layerii::FILTER_SIZE][lwmovie::layerii::NUM_SUBBANDS], const lwmFixedReal14 coeffs[lwmovie::layerii::NUM_SUBBANDS], int currentRotator, lwmFastUInt8 stride);
		void IMDCT32(lwmFixedReal14 out[64], const lwmFixedReal14 in[32]);
	}
}



#endif
