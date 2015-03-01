#ifndef __LWMOVIE_ADPCM_DECODER_HPP__
#define __LWMOVIE_ADPCM_DECODER_HPP__

#include "lwmovie_audiobuffer.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;

namespace lwmovie
{
	namespace adpcm
	{
		struct SPredictorState;
	}
}

namespace lwmovie
{
	class lwmCADPCMDecoder
	{
	public:
		explicit lwmCADPCMDecoder(lwmSAllocator *alloc);
		~lwmCADPCMDecoder();
		bool Init(const lwmAudioStreamInfo *audioStreamInfo);
		bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
		lwmCAudioBuffer *GetAudioBuffer();

	private:
		lwmSAllocator *m_alloc;
		lwmUInt8 m_numChannels;

		adpcm::SPredictorState *m_predictors;

		lwmCAudioBuffer m_audioBuffer;
	};
}

#endif
