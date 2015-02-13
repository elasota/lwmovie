/*
 * Copyright (c) 2014 Eric Lasota
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_dspops.hpp"
#include "lwmovie_layer2_constants.hpp"

void lwmovie::layerii::SubBandSynthesis(lwmSInt16 *output, lwmFixedReal22 accumulators[FILTER_SIZE][NUM_SUBBANDS], const lwmFixedReal14 coeffs[NUM_SUBBANDS], int currentRotator, lwmFastUInt8 stride)
{
	lwmUInt8 alignedSubbands[sizeof(lwmFixedReal14)*NUM_SUBBANDS*2 + LWMOVIE_FIXEDREAL_SIMD_ALIGNMENT];
	lwmFixedReal14 *subbands = reinterpret_cast<lwmFixedReal14*>(alignedSubbands - ((alignedSubbands  - reinterpret_cast<lwmUInt8*>(NULL)) & (LWMOVIE_FIXEDREAL_SIMD_ALIGNMENT - 1)) + LWMOVIE_FIXEDREAL_SIMD_ALIGNMENT);

	IMDCT32(subbands, coeffs);

	// Special first case, output instead of writing to accumulator
	{
		const lwmFixedReal22 *accumulator = accumulators[currentRotator % FILTER_SIZE];

		for(int samplePair=0;samplePair<NUM_SUBBANDS;samplePair+=LWMOVIE_FIXEDREAL_SIMD_WIDTH*2)
		{
			lwmSimdFixedReal22 finalSample[2];
			for(int subSample=0;subSample<2;subSample++)
			{
				int sample = samplePair + subSample*LWMOVIE_FIXEDREAL_SIMD_WIDTH;
				lwmSimdFixedReal27 highPrecSample = LWMOVIE_FIXEDREAL_INCREASEFRACPRECISION(lwmSimdFixedReal14::Load(subbands + sample), 13);
				finalSample[subSample] = lwmSimdFixedReal22::Load(accumulator + sample) + lwmSimdFixedReal29::Load(MP2_DEWINDOW[0] + sample) LWMOVIE_FIXEDREAL_MULTO(27, 22) (highPrecSample);
			}

			lwmSimdInt16<lwmSimdFixedRealRaw> outSamples = LWMOVIE_FIXEDREAL_LSAR_CLAMP(finalSample[0], finalSample[1], OUT_SCALE_SHIFT);
			for(int simdSub=0;simdSub<LWMOVIE_FIXEDREAL_SIMD_WIDTH*2;simdSub++)
			{
				*output = outSamples.GetSimdSub(simdSub);
				output += stride;
			}
		}
	}

	// Accumulate responses
	for(int hist=1;hist<FILTER_SIZE-1;hist++)
	{
		const lwmFixedReal29 *windowp = MP2_DEWINDOW[hist];
		const lwmFixedReal14 *phaseSubbands = subbands + (hist & 1)*NUM_SUBBANDS;
		lwmFixedReal22 *accumulator = accumulators[(currentRotator + hist) % FILTER_SIZE];

		for(int sample=0;sample<NUM_SUBBANDS;sample+=LWMOVIE_FIXEDREAL_SIMD_WIDTH)
		{
			lwmSimdFixedReal22 accum = lwmSimdFixedReal22::Load(accumulator + sample);
			accum += lwmSimdFixedReal29::Load(windowp + sample) LWMOVIE_FIXEDREAL_MULTO(27, 22) (LWMOVIE_FIXEDREAL_INCREASEFRACPRECISION(lwmSimdFixedReal14::Load(phaseSubbands + sample), 13));
			accum.Store(accumulator + sample);
		}
	}

	// Special last case, overwrite instead of accumulate
	{
		const lwmFixedReal29 *windowp = MP2_DEWINDOW[FILTER_SIZE-1];
		const lwmFixedReal14 *phaseSubbands = subbands + NUM_SUBBANDS;
		lwmFixedReal22 *accumulator = accumulators[(currentRotator + FILTER_SIZE - 1) % FILTER_SIZE];

		for(int sample=0;sample<NUM_SUBBANDS;sample+=LWMOVIE_FIXEDREAL_SIMD_WIDTH)
		{
			lwmSimdFixedReal22 accum = lwmSimdFixedReal22::Load(accumulator + sample);
			accum = lwmSimdFixedReal29::Load(windowp + sample) LWMOVIE_FIXEDREAL_MULTO(27, 22) (LWMOVIE_FIXEDREAL_INCREASEFRACPRECISION(lwmSimdFixedReal14::Load(phaseSubbands + sample), 13));
			accum.Store(accumulator + sample);
		}
	}
}
