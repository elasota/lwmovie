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
#include <new>

#include "lwmovie_adpcm.hpp"
#include "lwmovie_adpcm_decoder.hpp"
#include "lwmovie_package.hpp"

lwmovie::adpcm::CDecoder::CDecoder(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_numChannels(0)
	, m_audioBuffer(alloc)
	, m_predictors(NULL)
{
}

lwmovie::adpcm::CDecoder::~CDecoder()
{
	if(m_predictors)
		m_alloc->Free(m_predictors);
}

bool lwmovie::adpcm::CDecoder::Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *commonInfo, const lwmAudioStreamInfo *audioStreamInfo)
{
	if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Mono)
		m_numChannels = 1;
	else if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
		m_numChannels = 2;
	else
		return false;	// Unsupported channel layout

	if(!this->m_audioBuffer.Init(m_alloc, commonInfo->audioReadAhead, m_numChannels))
		return false;

	m_predictors = m_alloc->NAlloc<adpcm::SPredictorState>(m_numChannels);
	if(!m_predictors)
		return false;

	for(lwmFastUInt8 i=0;i<m_numChannels;i++)
		new (m_predictors+i) adpcm::SPredictorState();

	return true;
}

bool lwmovie::adpcm::CDecoder::DigestDataPacket(const void *data, lwmUInt32 packetSize, bool &outOverrun)
{
	const lwmUInt8 *bytes = static_cast<const lwmUInt8 *>(data);
	lwmUInt32 numChannelBytes = packetSize / m_numChannels;
	if(numChannelBytes < 2 || (numChannelBytes * m_numChannels) != packetSize)
		return false;

	lwmUInt32 numFrameSamples = (numChannelBytes-2) * 2;

	for(lwmFastUInt8 i=0;i<m_numChannels;i++)
	{
		lwmSInt16 encodedState = static_cast<lwmSInt16>(bytes[i*2+0] | (bytes[i*2+1] << 8));
		lwmSInt16 initialPredictor = static_cast<lwmSInt16>(encodedState & 0xff80);
		if(initialPredictor > 0)
			initialPredictor |= (initialPredictor >> 8);
		m_predictors[i].predictor = initialPredictor;

		lwmSInt8 stepIndex = static_cast<lwmSInt8>(encodedState & 0x7f);
		if(stepIndex < 0 || stepIndex >= static_cast<lwmSInt8>(adpcm::SPredictorState::NUM_STEPS))
			return false;
		m_predictors[i].stepIndex = stepIndex;
	}

	lwmUInt32 numDroppedSamples;
	void *outputBuf = m_audioBuffer.ReserveNewContiguous(numFrameSamples, numDroppedSamples);
	if(!outputBuf)
		return false;

	lwmSInt16 *outSamples = static_cast<lwmSInt16*>(outputBuf);
	lwmFastUInt8 nChannels = m_numChannels;

	const lwmUInt8 *sampleBytes = bytes + m_numChannels * 2;
	lwmSInt16 *chLimit = outSamples + m_numChannels * numFrameSamples;
	for(lwmUInt32 i=0;i<numFrameSamples;i+=2)
	{
		for(lwmFastUInt8 ch=0;ch<nChannels;ch++)
		{
			m_predictors[ch].DecodeSamples(sampleBytes[ch], outSamples[ch], outSamples[ch + nChannels]);
		}
		outSamples += nChannels * 2;
		sampleBytes += nChannels;
	}

	if(numDroppedSamples)
		outOverrun = true;
	return true;
}

lwmovie::CAudioBuffer *lwmovie::adpcm::CDecoder::GetAudioBuffer()
{
	return &m_audioBuffer;
}
