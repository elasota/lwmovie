#include <stdio.h>

#include "lwmux_riff.hpp"
#include "lwmux_wav.hpp"
#include "lwmux_osfile.hpp"
#include "lwmux_escape.hpp"
#include "lwmux_util.hpp"
#include "lwmux_planio.hpp"
#include "../lwmovie/lwmovie_adpcm.hpp"
#include "../lwmovie/lwmovie_external_types.h"
#include "../lwmovie/lwmovie_package.hpp"

using namespace lwmovie::riff;

static const unsigned int FRAME_SIZE = 1024;

static void EncodeADPCM(lwmUInt8 *output, const lwmSInt16 *samples, lwmUInt32 numSamples, lwmovie::adpcm::SPredictorState *predictorStates, lwmSInt32 stride, lwmUInt32 numChannels)
{
	while(numSamples)
	{
		for(lwmUInt32 i=0;i<numChannels;i++)
		{
			lwmovie::adpcm::SPredictorState predState = predictorStates[i];
			output[i] = predictorStates[i].EncodeSamples(samples[i], samples[i + numChannels]);
		}
		numSamples -= 2;
		samples += stride * 2;
		output += numChannels;
	}
}

void ConvertWAV_ADPCM(lwmOSFile *inFile, lwmOSFile *outFile)
{
	CRIFFDataList *rootAtom = static_cast<CRIFFDataList*>(lwmovie::riff::ParseAtom(inFile));
	CRIFFDataChunk *fmtAtom = rootAtom->FindDataChild(SFourCC('f', 'm', 't', ' '));

	SWAVFormat wavFormat;
	wavFormat.Read(fmtAtom, inFile);
	
	if(wavFormat.formatCode != SWAVFormat::FMT_PCM ||
		(wavFormat.numChannels != 1 && wavFormat.numChannels != 2) ||
		(wavFormat.bitsPerSample != 16 && wavFormat.bitsPerSample != 8))
	{
		fprintf(stderr, "Unsupported input format");
		return;
	}

	lwmovie::adpcm::SPredictorState *predictorStates = new lwmovie::adpcm::SPredictorState[wavFormat.numChannels];

	lwmUInt32 emittedSamples = 0;

	{
		lwmMovieHeader pkgHeader;
		pkgHeader.videoStreamType = lwmVST_None;
		pkgHeader.audioStreamType = lwmAST_ADPCM;
		pkgHeader.numTOC = 0;
		pkgHeader.largestPacketSize = 0;
		pkgHeader.longestFrameReadahead = 0;

		lwmAudioCommonInfo aci;
		aci.sampleRate = wavFormat.sampleRate;
		aci.numAudioStreams = 1;

		lwmAudioStreamInfo asi;
		asi.speakerLayout = lwmSPEAKERLAYOUT_Unknown;
		if(wavFormat.numChannels == 1)
			asi.speakerLayout = lwmSPEAKERLAYOUT_Mono;
		if(wavFormat.numChannels == 2)
			asi.speakerLayout = lwmSPEAKERLAYOUT_Stereo_LR;

		lwmWritePlanToFile(pkgHeader, outFile);
		lwmWritePlanToFile(aci, outFile);
		lwmWritePlanToFile(asi, outFile);
	}

	CRIFFDataChunk *dataAtom = rootAtom->FindDataChild(SFourCC('d', 'a', 't', 'a'));


	// This employs a kind of fun trick: The first frame is encoded backwards to prime the predictor.  This ensures
	bool firstFrame = false;

	inFile->Seek(dataAtom->FileOffset(), lwmOSFile::SM_Start);
	lwmUInt32 numSamples = dataAtom->ChunkSize() / (wavFormat.bitsPerSample/8) / wavFormat.numChannels;
	lwmUInt32 writtenSamples = 0;
	while(numSamples)
	{
		lwmUInt32 numUsableSamples = FRAME_SIZE;
		if(numUsableSamples > numSamples)
			numUsableSamples = numSamples;

		numSamples -= numUsableSamples;

		lwmUInt32 thisFrameSizeSamples = (numUsableSamples + 1) / 2 * 2;
		lwmUInt32 headerSize = 2*wavFormat.numChannels;
		lwmUInt32 samplesDataSize = thisFrameSizeSamples * wavFormat.numChannels / 2;
			
		lwmSInt16 *samples = new lwmSInt16[thisFrameSizeSamples * wavFormat.numChannels];
		lwmUInt8 *encodedBytes = new lwmUInt8[headerSize + samplesDataSize];

		lwmovie::riff::ReadPaddedWAVSamples(inFile, samples, numUsableSamples, wavFormat.numChannels, wavFormat.bitsPerSample, thisFrameSizeSamples);

		if(firstFrame)
		{
			// In the first frame, prime the predictor stepping by encoding backwards up to the first sample.
			firstFrame = false;
			EncodeADPCM(encodedBytes, samples + thisFrameSizeSamples - 2, thisFrameSizeSamples, predictorStates, -static_cast<lwmSInt16>(wavFormat.numChannels), wavFormat.numChannels);
		}

		// Write predictor states
		for(lwmUInt16 i=0;i<wavFormat.numChannels;i++)
		{
			lwmSInt32 adjustedPredictor = predictorStates[i].predictor;
			adjustedPredictor = ((((adjustedPredictor + 32768) * 0x1ff + 32767) / 65535) - 0x100) << 7;
			if(adjustedPredictor > 0)
				adjustedPredictor |= (adjustedPredictor >> 8);
			predictorStates[i].predictor = static_cast<lwmSInt16>(adjustedPredictor);

			encodedBytes[i*2 + 0] = static_cast<lwmUInt8>((adjustedPredictor & 0x80) | predictorStates[i].stepIndex);
			encodedBytes[i*2 + 1] = static_cast<lwmUInt8>((adjustedPredictor >> 8) & 0xff);
		}

		// Encode Samples
		EncodeADPCM(encodedBytes + headerSize, samples, thisFrameSizeSamples, predictorStates, static_cast<lwmSInt16>(wavFormat.numChannels), wavFormat.numChannels);

		// Write out
		writtenSamples += thisFrameSizeSamples;
		lwmux::util::WriteSyncedAudioPacket(outFile, encodedBytes, headerSize + samplesDataSize, writtenSamples);

		delete[] samples;
		delete[] encodedBytes;
	}

	delete[] predictorStates;
}
