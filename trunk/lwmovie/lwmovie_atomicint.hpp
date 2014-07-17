#ifndef __LWMOVIE_ATOMICINT_HPP__
#define __LWMOVIE_ATOMICINT_HPP__

#include <intrin.h>
#include "lwmovie_types.hpp"

inline lwmAtomicInt lwmAtomicIncrement(lwmAtomicInt volatile* ptr)
{
	return _InterlockedIncrement(ptr);
}

inline lwmAtomicInt lwmAtomicDecrement(lwmAtomicInt volatile* ptr)
{
	return _InterlockedDecrement(ptr);
}

#endif
