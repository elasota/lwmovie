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
#ifndef __LWMUX_WAV_HPP__
#define __LWMUX_WAV_HPP__

class lwmOSFile;

namespace lwmovie
{
	namespace riff
	{
		class CRIFFDataChunk;

		struct SWAVFormat
		{
			enum EFormatTag
			{
				FMT_PCM = 0x0001,
				FMT_FLOAT32 = 0x0003,
				FMT_ALAW = 0x0006,
				FMT_MULAW = 0x0007,
				FMT_EXTENSIBLE = 0xfffe,
			};

			lwmUInt16 formatCode;
			lwmUInt16 numChannels;
			lwmUInt32 sampleRate;
			lwmUInt32 bytesPerSecond;
			lwmUInt16 blockAlign;
			lwmUInt16 bitsPerSample;

			void Read(const CRIFFDataChunk *dataChunk, lwmOSFile *sourceFile);
		};

		bool ReadPaddedWAVSamples(lwmOSFile *inFile, lwmSInt16 *samples, lwmUInt32 numAvailableSamples, lwmUInt16 numChannels, lwmUInt16 bitsPerSample, lwmUInt32 numOutputSamples);
	}
}

#endif
