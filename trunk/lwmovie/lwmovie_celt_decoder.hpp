#ifndef __LWMOVIE_CELT_DECODER_HPP__
#define __LWMOVIE_CELT_DECODER_HPP__

#include "lwmovie_audiobuffer.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;
struct CELTMode;
struct CELTDecoder;

namespace lwmovie
{
	class lwmCCELTDecoder
	{
	public:
		explicit lwmCCELTDecoder(lwmSAllocator *alloc);
		~lwmCCELTDecoder();
		bool Init(const lwmAudioStreamInfo *audioStreamInfo);
		bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
		lwmCAudioBuffer *GetAudioBuffer();

	private:
		lwmSAllocator *m_alloc;
		lwmUInt8 m_numChannels;
		CELTMode *m_celtMode;
		CELTDecoder *m_celtDecoder;
		lwmCAudioBuffer m_audioBuffer;
	};
}

#endif
