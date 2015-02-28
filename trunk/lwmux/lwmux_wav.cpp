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
