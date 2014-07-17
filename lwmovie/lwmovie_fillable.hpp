#ifndef __LWMOVIE_FILLABLE_HPP__
#define __LWMOVIE_FILLABLE_HPP__

#include <string.h>
#include "lwmovie_types.hpp"

struct lwmFillableBuffer
{
	void *output;
	lwmUInt32 bytesRemaining;

	template<typename T, int Size>
	inline void Init(T (&buffer)[Size])
	{
		output = buffer;
		bytesRemaining = sizeof(T) * Size;
	}
	
	inline void Init(void *buffer, lwmUInt32 numBytes)
	{
		output = buffer;
		bytesRemaining = numBytes;
	}

	inline bool FeedData(const void **pptr, lwmUInt32 *bytesAvailable)
	{
		lwmUInt32 inBytes = (*bytesAvailable);
		if(inBytes >= bytesRemaining)
		{
			memcpy(output, *pptr, bytesRemaining);
			*pptr = reinterpret_cast<const lwmUInt8*>(*pptr) + bytesRemaining;
			*bytesAvailable = inBytes - bytesRemaining;
			bytesRemaining = 0;
			return true;
		}

		memcpy(output, *pptr, inBytes);
		*pptr = reinterpret_cast<const lwmUInt8*>(*pptr) + inBytes;
		output = reinterpret_cast<lwmUInt8*>(output) + inBytes;
		*bytesAvailable = inBytes - inBytes;
		bytesRemaining -= inBytes;
		return false;
	}
};

#endif
