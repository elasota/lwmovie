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
#include "lwmux_riff.hpp"
#include "lwmux_wav.hpp"
#include "lwmux_osfile.hpp"

void lwmovie::riff::SWAVFormat::Read(const CRIFFDataChunk *dataChunk, lwmOSFile *sourceFile)
{
	sourceFile->Seek(dataChunk->FileOffset(), lwmOSFile::SM_Start);

	formatCode = ReadUInt16(sourceFile);
	numChannels = ReadUInt16(sourceFile);
	sampleRate = ReadUInt32(sourceFile);
	bytesPerSecond = ReadUInt32(sourceFile);
	blockAlign = ReadUInt16(sourceFile);
	bitsPerSample = ReadUInt16(sourceFile);
}

bool lwmovie::riff::ReadPaddedWAVSamples(lwmOSFile *inFile, lwmSInt16 *samples, lwmUInt32 numAvailableSamples, lwmUInt16 numChannels, lwmUInt16 bitsPerSample, lwmUInt32 numOutputSamples)
{
	if(numAvailableSamples > numOutputSamples)
		numAvailableSamples = numOutputSamples;

	inFile->ReadBytes(samples, numAvailableSamples * numChannels * (bitsPerSample / 8));

	if(bitsPerSample == 16)
	{
		lwmSInt16 *procSample = samples;
		for(unsigned int i=0;i<numAvailableSamples * numChannels;i++)
		{
			lwmUInt8 sb[2];
			memcpy(sb, procSample, 2);
			lwmSInt16 swappedSample = static_cast<lwmSInt16>(sb[0] | (sb[1] << 8));
			memcpy(procSample, &swappedSample, 2);
			procSample++;
		}
	}
	else
	{
		const lwmSInt8 *procSmallSample = reinterpret_cast<const lwmSInt8*>(samples) + numAvailableSamples * numChannels;
		lwmSInt16 *procSample = samples + numAvailableSamples * numChannels;
		for(unsigned int i=0;i<numAvailableSamples * numChannels;i++)
		{
			procSample--;
			procSmallSample--;

			lwmSInt8 smallSample = *procSmallSample;
			lwmSInt16 largeSample;
			if(smallSample > 0)
				largeSample = (smallSample << 8) | (smallSample << 1) | (smallSample >> 6);
			else
				largeSample = smallSample << 8;

			memcpy(procSample, &largeSample, 2);
		}
	}

	// Silence the rest
	memset(samples + numAvailableSamples * numChannels, 0, sizeof(lwmSInt16) * (numOutputSamples - numAvailableSamples) * numChannels);

	return true;
}