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

		class CDecoder : public CAudioCodec
		{
		public:
			explicit CDecoder(lwmSAllocator *alloc);
			~CDecoder();
			bool Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *commonInfo, const lwmAudioStreamInfo *audioStreamInfo);
			bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
			CAudioBuffer *GetAudioBuffer();

		private:
			lwmSAllocator *m_alloc;
			lwmUInt8 m_numChannels;

			SPredictorState *m_predictors;

			CAudioBuffer m_audioBuffer;
		};
	}
}

#endif
