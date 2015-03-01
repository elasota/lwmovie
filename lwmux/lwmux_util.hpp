#ifndef __LWMUX_UTIL_HPP__
#define __LWMUX_UTIL_HPP__

#include "../common/lwmovie_coretypes.h"

class lwmOSFile;

namespace lwmux
{
	namespace util
	{
		void WriteSyncedAudioPacket(lwmOSFile *osFile, const lwmUInt8 *bytes, lwmUInt32 sz, lwmUInt32 syncTime);
	}
}

#endif
