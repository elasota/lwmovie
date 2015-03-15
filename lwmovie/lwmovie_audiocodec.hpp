#ifndef __LWMOVIE_AUDIOCODEC_HPP__
#define __LWMOVIE_AUDIOCODEC_HPP__

#include "../common/lwmovie_coretypes.h"

struct lwmMovieHeader;
struct lwmAudioStreamInfo;
struct lwmAudioCommonInfo;
class lwmCAudioBuffer;

namespace lwmovie
{
	struct lwmCAudioCodec
	{
	public:
		virtual bool Init(const lwmMovieHeader *movieHeader, const lwmAudioCommonInfo *audioCommonInfo, const lwmAudioStreamInfo *audioStreamInfo) = 0;
		virtual bool DigestDataPacket(const void *bytes, lwmUInt32 packetSize, bool &outOverrun) = 0;
		virtual lwmCAudioBuffer *GetAudioBuffer() = 0;
		virtual ~lwmCAudioCodec();
	};
}

#endif
