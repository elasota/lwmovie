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
#ifndef __LWMOVIE_EXTERNAL_TYPES_H__
#define __LWMOVIE_EXTERNAL_TYPES_H__

#include "../common/lwmovie_coretypes.h"

struct lwmMovieState;

struct lwmSAllocator
{
	void *(*allocFunc)(struct lwmSAllocator *alloc, lwmLargeUInt sz);
	void (*freeFunc)(struct lwmSAllocator *alloc, void *ptr);

#ifdef __cplusplus
	template<class T>
	inline T *NAlloc(lwmLargeUInt count)
	{
		lwmLargeUInt liMax = ~static_cast<lwmLargeUInt>(0);
		if(liMax / sizeof(T) < count)
			return NULL;
		return static_cast<T*>(allocFunc(this, sizeof(T) * count));
	}

	inline void Free(void *ptr)
	{
		freeFunc(this, ptr);
	}
#endif
};

struct lwmSWorkNotifier
{
	void (*notifyAvailableFunc)(struct lwmSWorkNotifier *workNotifier);
	void (*joinFunc)(struct lwmSWorkNotifier *workNotifier);
};

struct lwmIOFuncs
{
	lwmLargeUInt (*readFunc)(void *f, void *buf, lwmLargeUInt nBytes);
};

#endif
