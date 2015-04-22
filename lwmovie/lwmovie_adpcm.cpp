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
#include "lwmovie_adpcm.hpp"

lwmUInt8 lwmovie::adpcm::SPredictorState::EncodeSample(lwmSInt16 sample)
{
	lwmSInt16 step = s_StepTable[this->stepIndex];
	lwmSInt32 diff = static_cast<lwmSInt32>(sample) - this->predictor;
	bool negate = (diff < 0);
	lwmSInt32 absDiff = static_cast<lwmUInt32>(negate ? -diff : diff);

	lwmSInt32 attempts[2];
	lwmUInt32 predDiffSqs[2];
	attempts[1] = (absDiff*8 + step - 1)/step/2;
	attempts[0] = attempts[1] - 1;

	for(int i=0;i<2;i++)
	{
		if(attempts[i] < -1) attempts[i] = -1; else if(attempts[i] > 7) attempts[i] = 7;
		if(negate)
			attempts[i] = (-1 - attempts[i]);

		lwmSInt32 pred = ((attempts[i] * 2 + 1) * static_cast<lwmSInt32>(step)) / 8 + this->predictor;
		if(pred > 32767) pred = 32767; else if(pred < -32768) pred = -32768;
		predDiffSqs[i] = static_cast<lwmUInt32>((pred - sample) * (pred - sample));
	}

	lwmSInt32 selectedAttempt = (predDiffSqs[0] <= predDiffSqs[1]) ? attempts[0] : attempts[1];
	lwmUInt8 encoded = static_cast<lwmUInt8>(selectedAttempt + 8);

	// Decode sample to update predictor
	this->DecodeSample(encoded);

	return encoded;
}


lwmSInt8 lwmovie::adpcm::SPredictorState::s_IndexTable[16] =
{
	 8,  6,  4,  2, -1, -1, -1, -1,
	-1, -1, -1, -1,  2,  4,  6,  8
};

lwmSInt16 lwmovie::adpcm::SPredictorState::s_StepTable[89] =
{
	7,		8,		9,		10,		11,		12,		13,		14,
	16,		17,		19,		21,		23,		25,		28,		31,
	34,		37,		41,		45,		50,		55,		60,		66,
	73,		80,		88,		97,		107,	118,	130,	143,
	157,	173,	190,	209,	230,	253,	279,	307,
	337,	371,	408,	449,	494,	544,	598,	658,
	724,	796,	876,	963,	1060,	1166,	1282,	1411,
	1552,	1707,	1878,	2066,	2272,	2499,	2749,	3024,
	3327,	3660,	4026,	4428,	4871,	5358,	5894,	6484,
	7132,	7845,	8630,	9493,	10442,	11487,	12635,	13899,
	15289,	16818,	18500,	20350,	22385,	24623,	27086,	29794,
	32767
};
