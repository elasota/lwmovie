#include "../common/lwmovie_attribs.h"
#include "../common/lwmovie_coretypes.h"

#if defined(_MSC_VER) && (defined(_M_X86) || defined(_M_X64))
#include <intrin.h>

namespace lwmovie
{
	namespace arch
	{
		LWMOVIE_FORCEINLINE lwmUInt32 LoadBE32(const lwmUInt8 *bytes)
		{
			return _byteswap_ulong(*reinterpret_cast<const unsigned long*>(bytes));
		}
	}
}
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
namespace lwmovie
{
	namespace arch
	{
		LWMOVIE_FORCEINLINE lwmUInt32 LoadBE32(const lwmUInt8 *bytes)
		{
			return __builtin_bswap32(*reinterpret_cast<const lwmUInt32*>(bytes));
		}
	}
}
#else
namespace lwmovie
{
	namespace arch
	{
		LWMOVIE_FORCEINLINE lwmUInt32 LoadBE32(const lwmUInt8 *bytes)
		{
			return static_cast<lwmUInt32>((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
		}
	}
}
#endif
