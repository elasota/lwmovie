#ifndef __LWMOVIE_EXTERNAL_TYPES_HPP__
#define __LWMOVIE_EXTERNAL_TYPES_HPP__

#include "lwmovie_types.hpp"

struct lwmMovieState;

struct lwmSAllocator
{
	void *opaque;
	void *(*allocFunc)(void *opaque, lwmLargeUInt sz);
	void (*freeFunc)(void *opaque, void *ptr);
};

struct lwmSWorkNotifier
{
	void *opaque;
	void (*notifyAvailable)(void *opaque);
	void (*join)(void *opaque);
};

struct lwmIOFuncs
{
	lwmLargeUInt (*readFunc)(void *f, void *buf, lwmLargeUInt nBytes);
};

#endif