#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_dspops.hpp"
#include "lwmovie_layer2_constants.hpp"

void lwmovie::layerii::SubBandSynthesis(lwmSInt16 *output, lwmFixedReal22 accumulators[FILTER_SIZE][NUM_SUBBANDS], const lwmFixedReal14 coeffs[NUM_SUBBANDS], int currentRotator, lwmFastUInt8 stride)
{
	lwmFixedReal14 subbands[NUM_SUBBANDS*2];
	IMDCT32(subbands, coeffs);

	// Special first case, output instead of writing to accumulator
	{
		const lwmFixedReal22 *accumulator = accumulators[currentRotator % FILTER_SIZE];

		for(int sample=0;sample<NUM_SUBBANDS;sample++)
		{
			lwmFixedReal22 finalSample = accumulator[sample] + MP2_DEWINDOW[0][sample].MulTo<27, 22>(subbands[sample].IncreaseFracPrecision<13>());

			lwmSInt32 raw = finalSample.LShiftAndRound(OUT_SCALE_SHIFT);
			if(raw>OUT_SCALE_MAX)
				raw=OUT_SCALE_MAX;
			else if(raw<OUT_SCALE_MIN)
				raw=OUT_SCALE_MIN;
			*output = static_cast<lwmSInt16>(raw);
			output += stride;
		}
	}

	// Accumulate responses
	for(int hist=1;hist<FILTER_SIZE-1;hist++)
	{
		const lwmFixedReal29 *windowp = MP2_DEWINDOW[hist];
		const lwmFixedReal14 *phaseSubbands = subbands + (hist & 1)*NUM_SUBBANDS;
		lwmFixedReal22 *accumulator = accumulators[(currentRotator + hist) % FILTER_SIZE];

		for(int sample=0;sample<NUM_SUBBANDS;sample++)
			accumulator[sample] += windowp[sample].MulTo<27, 22>(phaseSubbands[sample].IncreaseFracPrecision<13>());
	}

	// Special last case, overwrite instead of accumulate
	{
		const lwmFixedReal29 *windowp = MP2_DEWINDOW[FILTER_SIZE-1];
		const lwmFixedReal14 *phaseSubbands = subbands + NUM_SUBBANDS;
		lwmFixedReal22 *accumulator = accumulators[(currentRotator + FILTER_SIZE - 1) % FILTER_SIZE];

		for(int sample=0;sample<NUM_SUBBANDS;sample++)
			accumulator[sample] = windowp[sample].MulTo<27, 22>(phaseSubbands[sample].IncreaseFracPrecision<13>());
	}
}
