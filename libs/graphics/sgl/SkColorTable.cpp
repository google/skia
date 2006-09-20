#include "SkBitmap.h"
#include "SkTemplates.h"

SkColorTable::SkColorTable() : fColors(nil), f16BitCache(nil), fCount(0), fFlags(0)
{
	SkDEBUGCODE(fColorLockCount = 0;)
	SkDEBUGCODE(f16BitCacheLockCount = 0;)
}

SkColorTable::~SkColorTable()
{
	SkASSERT(fColorLockCount == 0);
	SkASSERT(f16BitCacheLockCount == 0);

	sk_free(fColors);
	sk_free(f16BitCache);
}

void SkColorTable::setFlags(unsigned flags)
{
	fFlags = SkToU8(flags);
}

void SkColorTable::setColors(const SkPMColor src[], int count)
{
	SkASSERT(fColorLockCount == 0);
	SkASSERT((unsigned)count <= 256);

	if (fCount != count)
	{
		if (count == 0)
		{
			sk_free(fColors);
			fColors = nil;
		}
		else
		{
			// allocate new array before freeing old, in case the alloc fails (throws)
			SkPMColor* table = (SkPMColor*)sk_malloc_throw(count * sizeof(SkPMColor));
			sk_free(fColors);
			fColors = table;

			if (src)
				memcpy(fColors, src, count * sizeof(SkPMColor));
		}
		fCount = SkToU16(count);
	}
	else
	{
		if (src)
			memcpy(fColors, src, count * sizeof(SkPMColor));
	}

	this->inval16BitCache();
}

void SkColorTable::inval16BitCache()
{
	SkASSERT(f16BitCacheLockCount == 0);
	if (f16BitCache)
	{
		sk_free(f16BitCache);
		f16BitCache = nil;
	}
}

#include "SkColorPriv.h"

static inline void build_16bitcache(U16 dst[], const SkPMColor src[], int count)
{
	while (--count >= 0)
		*dst++ = SkPixel32ToPixel16_ToU16(*src++);
}

const U16* SkColorTable::lock16BitCache()
{
	if (fFlags & kColorsAreOpaque_Flag)
	{
		if (f16BitCache == nil)	// build the cache
		{
			f16BitCache = (U16*)sk_malloc_throw(fCount * sizeof(U16));
			build_16bitcache(f16BitCache, fColors, fCount);
		}
	}
	else	// our colors have alpha, so no cache
	{
		this->inval16BitCache();
		if (f16BitCache)
		{
			sk_free(f16BitCache);
			f16BitCache = nil;
		}
	}

	SkDEBUGCODE(f16BitCacheLockCount += 1);
	return f16BitCache;
}


