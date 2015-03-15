#ifndef __LWMOVIE_ADPCM_DECODER_HPP__
#define __LWMOVIE_ADPCM_DECODER_HPP__

#include "lwmovie_audiobuffer.hpp"
#include "lwmovie_audiocodec.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;
struct lwmMovieHeader;

namespace lwmovie
{
	namespace adpcm
	{
		struct SPredictorState;
	}
}

namespace lwmovie
{
	class lwmCADPCMDecoder : public lwmCAudioCodec
	{
	public:
		explicit lwmCADPCMDecoder(lwmSAllocator *alloc);
		~lwmCADPCMDecoder();
		bool Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *commonInfo,const lwmAudioStreamInfo *audioStreamInfo);
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
