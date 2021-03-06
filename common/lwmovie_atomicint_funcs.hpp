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
#ifndef __LWMOVIE_ATOMICINT_FUNCS_HPP__
#define __LWMOVIE_ATOMICINT_FUNCS_HPP__

#include "lwmovie_atomicint_type.hpp"

#ifdef _MSC_VER

#include <intrin.h>

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchangePointer)

__forceinline lwmAtomicInt lwmAtomicIncrement(lwmAtomicInt volatile* ptr)
{
	return _InterlockedIncrement(ptr);
}

__forceinline lwmAtomicInt lwmAtomicDecrement(lwmAtomicInt volatile* ptr)
{
	return _InterlockedDecrement(ptr);
}

__forceinline void *lwmAtomicCompareAndSwap(void *volatile* destination, void *newValue, void *comp)
{
	return _InterlockedCompareExchangePointer(destination, newValue, comp);
}

#endif	// _MSC_VER

#ifdef __GNUC__

inline __attribute__((always_inline)) lwmAtomicInt lwmAtomicIncrement(lwmAtomicInt volatile* ptr)
{
	return __sync_add_and_fetch(ptr, 1);
}

inline __attribute__((always_inline)) lwmAtomicInt lwmAtomicDecrement(lwmAtomicInt volatile* ptr)
{
	return __sync_add_and_fetch(ptr, -1);
}

inline __attribute__((always_inline)) void *lwmAtomicCompareAndSwap(void *volatile* destination, void *newValue, void *comp)
{
	return __sync_val_compare_and_swap(destination, comp, newValue);
}

#endif

#endif
