/*
* Copyright (c) 2015 Eric Lasota
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
#ifndef __LWMOVIE_ADPCM_HPP__
#define __LWMOVIE_ADPCM_HPP__

#include "../common/lwmovie_coretypes.h"

namespace lwmovie
{
	namespace adpcm
	{
		struct SPredictorState
		{
			SPredictorState();
			lwmUInt8 EncodeSamples(lwmSInt16 sample1, lwmSInt16 sample2);
			void DecodeSamples(lwmUInt8 inSamples, lwmSInt16 &outSample1, lwmSInt16 &outSample2);

			lwmSInt16 predictor;
			lwmSInt8 stepIndex;

			static const unsigned int NUM_STEPS = 89;

			private:
				lwmUInt8 EncodeSample(lwmSInt16 sample);
				lwmSInt16 DecodeSample(lwmUInt8 encoded);
				static lwmSInt8 s_IndexTable[16];
				static lwmSInt16 s_StepTable[NUM_STEPS];
		};
	}
}

inline lwmovie::adpcm::SPredictorState::SPredictorState()
	: predictor(0)
	, stepIndex(0)
{
}

inline lwmUInt8 lwmovie::adpcm::SPredictorState::EncodeSamples(lwmSInt16 sample1, lwmSInt16 sample2)
{
	lwmUInt8 s1enc = EncodeSample(sample1);
	lwmUInt8 s2enc = EncodeSample(sample2);
	return static_cast<lwmUInt8>(s1enc | (s2enc << 4));
}

inline void lwmovie::adpcm::SPredictorState::DecodeSamples(lwmUInt8 inSamples, lwmSInt16 &outSample1, lwmSInt16 &outSample2)
{
	outSample1 = DecodeSample(static_cast<lwmUInt8>(inSamples & 0xf));
	outSample2 = DecodeSample(static_cast<lwmUInt8>((inSamples >> 4) & 0xf));
}

inline lwmSInt16 lwmovie::adpcm::SPredictorState::DecodeSample(lwmUInt8 encoded)
{
	// Update predictor
	lwmSInt32 newPred = this->predictor + (static_cast<lwmSInt32>(encoded) * 2 - 15) * s_StepTable[this->stepIndex] / 8;
	if(newPred > 32767)
		newPred = 32767;
	else if(newPred < -32768)
		newPred = -32768;

	this->predictor = static_cast<lwmSInt16>(newPred);

	this->stepIndex += s_IndexTable[encoded];
	if(this->stepIndex < 0) 
		this->stepIndex = 0;
	else if(this->stepIndex >= NUM_STEPS)
		this->stepIndex = NUM_STEPS - 1;

	return newPred;
}

#endif
