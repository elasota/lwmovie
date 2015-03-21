#include "lwmovie_mp2_decoder.hpp"
#include "lwmovie_package.hpp"
#include <string.h>

lwmovie::lwmCMP2Decoder::lwmCMP2Decoder(lwmSAllocator *alloc)
	: m_alloc(alloc)
	, m_numChannels(0)
	, m_audioBuffer(alloc)
{
	memset(m_frameData, 0, sizeof(m_frameData));
}

lwmovie::lwmCMP2Decoder::~lwmCMP2Decoder()
{
}

bool lwmovie::lwmCMP2Decoder::Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *commonInfo, const lwmAudioStreamInfo *audioStreamInfo)
{
	if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Mono)
		m_numChannels = 1;
	else if(audioStreamInfo->speakerLayout == lwmSPEAKERLAYOUT_Stereo_LR)
		m_numChannels = 2;
	else
		return false;	// Unsupported channel layout

	if(!this->m_audioBuffer.Init(m_alloc, commonInfo->audioReadAhead, m_numChannels))
		return false;

	return true;
}

bool lwmovie::lwmCMP2Decoder::DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun)
{
	outOverrun = false;
	if(packetSize > lwmovie::layerii::MAX_FRAME_SIZE_BYTES || packetSize < lwmovie::layerii::HEADER_SIZE_BYTES)
		return false;
	// TODO: If unsafe decodes are permitted, fast decode could skip this copy
	memcpy(m_frameData, bytes, packetSize);
	if(!m_decodeState.ParseHeader(m_frameData))
		return false;
	if(packetSize - lwmovie::layerii::HEADER_SIZE_BYTES != m_decodeState.GetFrameSizeBytes())
		return false;
	if(m_decodeState.GetChannelCount() != m_numChannels)
		return false;	// Real-time switch between mono/stereo not supported
	lwmUInt32 numDroppedSamples;
	void *outputBuf = m_audioBuffer.ReserveNewContiguous(lwmovie::layerii::FRAME_NUM_SAMPLES, numDroppedSamples);
	if(!outputBuf)
		return false;
	if(!m_decodeState.DecodeFrame(m_frameData + lwmovie::layerii::HEADER_SIZE_BYTES, static_cast<lwmSInt16*>(outputBuf)))
		return false;
	if(numDroppedSamples)
		outOverrun = true;
	return true;
}

lwmCAudioBuffer *lwmovie::lwmCMP2Decoder::GetAudioBuffer()
{
	return &m_audioBuffer;
}
