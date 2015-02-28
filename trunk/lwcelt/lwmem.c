#include "../lwmovie/lwmovie_external_types.h"
#include "os_support.h"

void *celt_alloc (struct lwmSAllocator *alloc, int size)
{
	lwmLargeUInt sz = (lwmLargeUInt)size;
	void *ptr = alloc->allocFunc(alloc, sz);
	memset(ptr, 0, sz);
	return ptr;
}

void *celt_alloc_scratch (struct lwmSAllocator *alloc, int size)
{
	return alloc->allocFunc(alloc, (lwmLargeUInt)size);
}

void celt_free (struct lwmSAllocator *alloc, void *ptr)
{
	alloc->freeFunc(alloc, ptr);
}

void celt_free_scratch (struct lwmSAllocator *alloc, void *ptr)
{
	alloc->freeFunc(alloc, ptr);
}
