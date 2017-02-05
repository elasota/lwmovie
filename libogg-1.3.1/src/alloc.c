#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ogg/ogg.h>

void *ogghack_calloc(const ogg_allocator *alloc, size_t count, size_t sz)
{
	void *buf;

	if (count == 0 || sz == 0)
		return alloc->reallocfunc(alloc->ctx, NULL, 0);

	if (SIZE_MAX / sz < count)
		return NULL;

	buf = alloc->reallocfunc(alloc->ctx, NULL, sz * count);

	if (buf != NULL)
		memset(buf, 0, sz * count);

	return buf;
}

void *ogghack_malloc(const ogg_allocator *alloc, size_t sz)
{
	return alloc->reallocfunc(alloc->ctx, NULL, sz);
}

void *ogghack_realloc(const ogg_allocator *alloc, void *ptr, size_t sz)
{
	return alloc->reallocfunc(alloc->ctx, ptr, sz);
}

void ogghack_free(const ogg_allocator *alloc, void *ptr)
{
	alloc->reallocfunc(alloc->ctx, ptr, 0);
}
