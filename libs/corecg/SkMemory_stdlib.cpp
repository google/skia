#include "SkTypes.h"
#include <stdio.h>
#include <stdlib.h>

void sk_throw()
{
#ifdef ANDROID
    fprintf(stderr, "throwing...\n");
#endif
    abort();
}

void sk_out_of_memory(void)
{
#ifdef ANDROID
    fprintf(stderr,"- out of memory in SGL -\n");
#endif
    abort();
}

void* sk_malloc_throw(size_t size)
{
	return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size)
{
	SkDEBUGCODE(if (size) size += 4;)
	SkDEBUGCODE(if (addr) addr = (char*)addr - 4;)

	void* p = realloc(addr, size);
	if (size == 0)
		return p;

	if (p == NULL)
		sk_throw();
#ifdef SK_DEBUG
	else
	{
		memcpy(p, "skia", 4);
		p = (char*)p + 4;
	}
#endif
	return p;
}

void sk_free(void* p)
{
	if (p)
	{
#ifdef SK_DEBUG
		SkDEBUGCODE(p = (char*)p - 4;)
		SkASSERT(memcmp(p, "skia", 4) == 0);
#endif
		free(p);
	}
}

void* sk_malloc_flags(size_t size, unsigned flags)
{
	SkDEBUGCODE(size += 4;)
	
	void* p = malloc(size);
	if (p == NULL)
	{
		if (flags & SK_MALLOC_THROW)
			sk_throw();
	}
#ifdef SK_DEBUG
	else
	{
		memcpy(p, "skia", 4);
		p = (char*)p + 4;
		memset(p, 0xCD, size - 4);
	}
#endif
	return p;
}

