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

#include "lwmovie_celt_decoder.hpp"
#include "lwmovie_package.hpp"
#include "../lwcelt/celt/celt.h"

static const unsigned int FRAME_SIZE = 1024;
static const unsigned int CELT_MAX_FRAME_BYTES = 1275;

lwmovie::celt::CDecoder::CDecoder(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_numChannels(0)
	, m_audioBuffer(alloc)
	, m_celtMode(NULL)
	, m_celtDecoder(NULL)
{
}

lwmovie::celt::CDecoder::~CDecoder()
{
	if(m_celtDecoder)
		opus_custom_decoder_destroy(m_celtDecoder);
	if(m_celtMode)
		opus_custom_mode_destroy(m_celtMode);
}

bool lwmovie::celt::CDecoder::Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo)
{
	if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Mono)
		m_numChannels = 1;
	else if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
		m_numChannels = 2;
	else
		return false;	// Unsupported channel layout

	if(!this->m_audioBuffer.Init(m_alloc, audioCommonInfo->audioReadAhead, m_numChannels))
		return false;

	int errorCode;
	m_celtMode = opus_custom_mode_create(m_alloc, audioCommonInfo->sampleRate, FRAME_SIZE, &errorCode);
	if(!m_celtMode)
		return false;
	m_celtDecoder = opus_custom_decoder_create(m_alloc, m_celtMode, m_numChannels, &errorCode);
	if(!m_celtDecoder)
		return false;

	return true;
}

bool lwmovie::celt::CDecoder::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun)
{
	outOverrun = false;
	if(packetSize > 1275)
		return false;

	lwmUInt32 numDroppedSamples;
	void *outputBuf = m_audioBuffer.ReserveNewContiguous(FRAME_SIZE, numDroppedSamples);
	if(!outputBuf)
		return false;
	if(opus_custom_decode(m_celtDecoder, static_cast<const unsigned char *>(bytes), static_cast<int>(packetSize), static_cast<opus_int16*>(outputBuf), FRAME_SIZE) != FRAME_SIZE)
		return false;
	if(numDroppedSamples)
		outOverrun = true;
	return true;
}

lwmovie::CAudioBuffer *lwmovie::celt::CDecoder::GetAudioBuffer()
{
	return &m_audioBuffer;
}
