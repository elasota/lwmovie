/*
 * Copyright (c) 2014 Eric Lasota
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
