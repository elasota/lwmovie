/*
Copyright (c) 2012 Eric Lasota

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lwmovie_layer2.hpp"
#include "lwmovie_layer2_decodestate.hpp"

#include <stdlib.h>

using namespace lwmovie;
using namespace lwmovie::layerii;

// Simple MP2 to raw sample converter
int main(int argc, const char **argv)
{
	unsigned char headerBytes[HEADER_SIZE_BYTES];
	unsigned char frameBytes[MAX_FRAME_SIZE_BYTES];
	clock_t startTime;

	lwmCMP2DecodeState decodeState;

	lwmSInt16 mixed[2*FRAME_NUM_SAMPLES];

	printf("Init\n");
	printf("Start\n");
	startTime = clock();

	FILE *f = fopen(argv[1], "rb");
	FILE *fout = fopen(argv[2], "wb");
	while(!feof(f))
	{
		fread(headerBytes, HEADER_SIZE_BYTES, 1, f);

		decodeState.ParseHeader(headerBytes);

		lwmUInt16 frameSizeBytes = decodeState.GetFrameSizeBytes();
		fread(frameBytes, frameSizeBytes, 1, f);

		decodeState.DecodeFrame(frameBytes, mixed);

		fwrite(mixed, FRAME_NUM_SAMPLES*2*2, 1, fout);
	}
	fclose(f);
	fclose(fout);

	printf("Time: %i\n", clock() - startTime);
	return 0;
}
