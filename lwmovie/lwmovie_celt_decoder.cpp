#include <stdlib.h>

#include "lwmovie_celt_decoder.hpp"
#include "lwmovie_package.hpp"
#include "../lwcelt/celt/celt.h"

static const unsigned int FRAME_SIZE = 1024;
static const unsigned int CELT_MAX_FRAME_BYTES = 1275;

lwmovie::lwmCCELTDecoder::lwmCCELTDecoder(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_numChannels(0)
	, m_audioBuffer(alloc)
	, m_celtMode(NULL)
	, m_celtDecoder(NULL)
{
}

lwmovie::lwmCCELTDecoder::~lwmCCELTDecoder()
{
	if(m_celtDecoder)
		opus_custom_decoder_destroy(m_celtDecoder);
	if(m_celtMode)
		opus_custom_mode_destroy(m_celtMode);
}

bool lwmovie::lwmCCELTDecoder::Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo)
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

bool lwmovie::lwmCCELTDecoder::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun)
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

lwmCAudioBuffer *lwmovie::lwmCCELTDecoder::GetAudioBuffer()
{
	return &m_audioBuffer;
}
