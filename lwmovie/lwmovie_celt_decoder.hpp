#ifndef __LWMOVIE_CELT_DECODER_HPP__
#define __LWMOVIE_CELT_DECODER_HPP__

#include "lwmovie_audiobuffer.hpp"
#include "lwmovie_audiocodec.hpp"

struct lwmSAllocator;
struct lwmAudioStreamInfo;
struct lwmMovieHeader;
struct OpusCustomMode;
struct OpusCustomDecoder;

namespace lwmovie
{
	namespace celt
	{
		class CDecoder : public CAudioCodec
		{
		public:
			explicit CDecoder(lwmSAllocator *alloc);
			~CDecoder();
			bool Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo);
			bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun);
			CAudioBuffer *GetAudioBuffer();

		private:
			lwmSAllocator *m_alloc;
			lwmUInt8 m_numChannels;
			OpusCustomMode *m_celtMode;
			OpusCustomDecoder *m_celtDecoder;
			CAudioBuffer m_audioBuffer;
		};
	}
}

#endif
