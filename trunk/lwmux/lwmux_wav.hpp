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
	}
}

#endif
