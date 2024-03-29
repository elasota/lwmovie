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
#include <stdlib.h>
#include <stdio.h>

#include "lwmux_riff.hpp"
#include "lwmux_wav.hpp"
#include "lwmux_osfile.hpp"
#include "lwmux_escape.hpp"
#include "lwmux_planio.hpp"
#include "../lwmovie/lwmovie_external_types.h"
#include "../lwmovie/lwmovie_package.hpp"
#include "../lwcelt/celt/celt.h"

using namespace lwmovie::riff;

static const unsigned int FRAME_SIZE = 1024;
static const unsigned int ENCODE_DELAY = 128;

static void *myRealloc(lwmSAllocator *alloc, void *ptr, lwmLargeUInt sz)
{
	return realloc(ptr, sz);
}

void ConvertWAV_CELT(lwmOSFile *inFile, lwmOSFile *outFile, lwmUInt32 bitsPerSecond, bool vbr, const char *metaID)
{
	CRIFFDataList *rootAtom = static_cast<CRIFFDataList*>(lwmovie::riff::ParseAtom(inFile));
	CRIFFDataChunk *fmtAtom = rootAtom->FindDataChild(SFourCC('f', 'm', 't', ' '));

	lwmSAllocator alloc;
	alloc.reallocFunc = myRealloc;

	SWAVFormat wavFormat;
	wavFormat.Read(fmtAtom, inFile);
	
	if(wavFormat.formatCode != SWAVFormat::FMT_PCM ||
		(wavFormat.numChannels != 1 && wavFormat.numChannels != 2) ||
		(wavFormat.bitsPerSample != 16 && wavFormat.bitsPerSample != 8))
	{
		fprintf(stderr, "Unsupported input format");
		return;
	}

	lwmUInt32 emittedSamples = 0;

	{
		lwmMovieHeader pkgHeader;
		pkgHeader.videoStreamType = lwmVST_None;
		pkgHeader.audioStreamType = lwmAST_OpusCustom;
		pkgHeader.numTOC = 0;
		pkgHeader.largestPacketSize = 0;
		pkgHeader.longestFrameReadahead = 0;

		lwmAudioCommonInfo aci;
		aci.numAudioStreams = 1;
		aci.sampleRate = wavFormat.sampleRate;

		lwmAudioStreamInfo asi;
		asi.speakerLayout = lwmSPEAKERLAYOUT_Unknown;
		if(wavFormat.numChannels == 1)
			asi.speakerLayout = lwmSPEAKERLAYOUT_Mono;
		if(wavFormat.numChannels == 2)
			asi.speakerLayout = lwmSPEAKERLAYOUT_Stereo_LR;

		EncodeMetaID(metaID, asi.metaID);

		lwmWritePlanToFile(pkgHeader, outFile);
		lwmWritePlanToFile(aci, outFile);
		lwmWritePlanToFile(asi, outFile);
	}

	CRIFFDataChunk *dataAtom = rootAtom->FindDataChild(SFourCC('d', 'a', 't', 'a'));

	int errorCode;
	CELTMode *mode = opus_custom_mode_create(&alloc, wavFormat.sampleRate, FRAME_SIZE, &errorCode);
	if (!mode)
	{
		fprintf(stderr, "CELT encoder failed to initialize, problem with audio parameters");
		return;
	}

	{
		CELTEncoder *encoder = opus_custom_encoder_create(&alloc, mode, wavFormat.numChannels, &errorCode);
		if(encoder)
		{
			int vbrFlag = (vbr ? 0 : 1);
			opus_custom_encoder_ctl(encoder, OPUS_SET_VBR(vbrFlag));
			opus_custom_encoder_ctl(encoder, OPUS_SET_BITRATE(bitsPerSecond));

			inFile->Seek(dataAtom->FileOffset(), lwmOSFile::SM_Start);
			lwmUInt32 numSamples = dataAtom->ChunkSize() / (wavFormat.bitsPerSample/8) / wavFormat.numChannels + ENCODE_DELAY;
			while(numSamples)
			{
				lwmSInt16 *samples = new lwmSInt16[FRAME_SIZE * wavFormat.numChannels];
				lwmUInt8 *encodedBytes = new lwmUInt8[2000];

				lwmUInt32 numUsableSamples = FRAME_SIZE;
				if(numSamples < ENCODE_DELAY)
				{
					numUsableSamples = 0;
					numSamples = 0;
				}
				else if(numUsableSamples > numSamples - ENCODE_DELAY)
				{
					numUsableSamples = numSamples - ENCODE_DELAY;
					numSamples = 0;
				}
				else
					numSamples -= FRAME_SIZE;

				inFile->ReadBytes(samples, numUsableSamples * wavFormat.numChannels * (wavFormat.bitsPerSample / 8));

				if(wavFormat.bitsPerSample == 16)
				{
					lwmSInt16 *procSample = samples;
					for(unsigned int i=0;i<numUsableSamples;i++)
					{
						for(unsigned int ch=0;ch<wavFormat.numChannels;ch++)
						{
							lwmUInt8 sb[2];
							memcpy(sb, procSample, 2);
							lwmSInt16 swappedSample = static_cast<lwmSInt16>(sb[0] | (sb[1] << 8));
							memcpy(procSample, &swappedSample, 2);
							procSample++;
						}
					}
				}
				else
				{
					const lwmSInt8 *procSmallSample = reinterpret_cast<const lwmSInt8*>(samples) + numUsableSamples * wavFormat.numChannels;
					lwmSInt16 *procSample = samples + numUsableSamples * wavFormat.numChannels;
					for(unsigned int i=0;i<numUsableSamples;i++)
					{
						for(unsigned int ch=0;ch<wavFormat.numChannels;ch++)
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
				}

				// Silence anything left over
				memset(samples + numUsableSamples*wavFormat.numChannels, 0, sizeof(lwmSInt16) * (FRAME_SIZE - numUsableSamples) * wavFormat.numChannels);

				// Encode
				int numEncoded = opus_custom_encode(encoder, samples, FRAME_SIZE, encodedBytes, 2000);
				if(numEncoded > 0)
				{
					lwmPacketHeader pktHeader;
					lwmPacketHeaderFull pktHeaderFull;
					pktHeaderFull.streamIndex = 0;
					pktHeaderFull.packetSize = static_cast<lwmUInt32>(numEncoded);
					pktHeader.packetTypeAndFlags = lwmEPT_Audio_Frame;

					lwmUInt32 packetSizeEscaped = lwmComputeEscapes(encodedBytes, pktHeaderFull.packetSize);
					lwmUInt8 *escapedPacketBytes = NULL;
					if(packetSizeEscaped != pktHeaderFull.packetSize)
					{
						escapedPacketBytes = new lwmUInt8[packetSizeEscaped];
						lwmGenerateEscapedBytes(escapedPacketBytes, encodedBytes, pktHeaderFull.packetSize);
						pktHeader.packetTypeAndFlags |= lwmPacketHeader::EFlag_Escaped;
						pktHeaderFull.packetSize = packetSizeEscaped;
					}

					lwmWritePlanToFile(pktHeader, outFile);
					lwmWritePlanToFile(pktHeaderFull, outFile);

					if(escapedPacketBytes)
					{
						outFile->WriteBytes(escapedPacketBytes, pktHeaderFull.packetSize);
						delete[] escapedPacketBytes;
					}
					else
						outFile->WriteBytes(encodedBytes, pktHeaderFull.packetSize);

					// Write synchronization
					emittedSamples += FRAME_SIZE;

					{
						lwmAudioSynchronizationPoint syncPoint;
						syncPoint.audioPeriod = emittedSamples - ENCODE_DELAY;

						lwmPacketHeader syncPacket;
						lwmPacketHeaderFull syncPacketFull;
						syncPacketFull.streamIndex = 0;
						syncPacketFull.packetSize = lwmPlanHandler<lwmAudioSynchronizationPoint>::SIZE;
						syncPacket.packetTypeAndFlags = lwmEPT_Audio_Synchronization;

						lwmWritePlanToFile(syncPacket, outFile);
						lwmWritePlanToFile(syncPacketFull, outFile);
						lwmWritePlanToFile(syncPoint, outFile);
					}
				}

				delete[] samples;
				delete[] encodedBytes;
			}
			opus_custom_encoder_destroy(encoder);
		}
		opus_custom_mode_destroy(mode);
	}
}
