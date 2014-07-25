#ifndef __LWMOVIE_ATOMICINT_HPP__
#define __LWMOVIE_ATOMICINT_HPP__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "lwmovie_types.hpp"

inline lwmAtomicInt lwmAtomicIncrement(lwmAtomicInt volatile* ptr)
{
	return InterlockedIncrement(ptr);
}

inline lwmAtomicInt lwmAtomicDecrement(lwmAtomicInt volatile* ptr)
{
	return InterlockedDecrement(ptr);
}

inline void *lwmAtomicCompareAndSwap(void *volatile* destination, void *newValue, void *comp)
{
	return InterlockedCompareExchangePointer(destination, newValue, comp);
}

#endif
