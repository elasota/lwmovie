#ifndef __LWMOVIE_MP2_DECODER_HPP__
#define __LWMOVIE_MP2_DECODER_HPP__

#include "../mp2dec2/lwmovie_layer2_decodestate.hpp"
#include "lwmovie_audiobuffer.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;

namespace lwmovie
{
	class lwmCMP2Decoder
	{
	public:
		explicit lwmCMP2Decoder(lwmSAllocator *alloc);
		~lwmCMP2Decoder();
		bool Init(const lwmAudioStreamInfo *audioStreamInfo);
		bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
		lwmCAudioBuffer *GetAudioBuffer();

	private:
		lwmSAllocator *m_alloc;
		lwmUInt8 m_numChannels;
		lwmovie::layerii::lwmCMP2DecodeState m_decodeState;
		lwmUInt8 m_frameData[lwmovie::layerii::MAX_FRAME_SIZE_BYTES];
		lwmCAudioBuffer m_audioBuffer;
	};
}

#endif
