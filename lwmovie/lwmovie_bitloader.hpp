/*
* Copyright (c) 2015 Eric Lasota
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include "../common/lwmovie_attribs.h"
#include "../common/lwmovie_coretypes.h"

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64))
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
