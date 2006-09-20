#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkTemplates.h"

//////////////////////////////////////////////////////////////////////

#define kMinGlphAlloc		(sizeof(SkGlyph) * 64)
#define kMinImageAlloc		(24 * 64)	// this guy should be pointsize-dependent IMHO

/*  We use this local function instead of the static class method to work around
    a bug in gcc98
*/
void SkDescriptor_Free(SkDescriptor* desc);
void SkDescriptor_Free(SkDescriptor* desc)
{
    SkDescriptor::Free(desc);
}

SkGlyphCache::SkGlyphCache(const SkDescriptor* desc)
	: fGlyphAlloc(kMinGlphAlloc), fImageAlloc(kMinImageAlloc)
{
	fNext = NULL;
	fDesc = desc->copy();
	SkAutoTCallProc<SkDescriptor, SkDescriptor_Free> autoFree(fDesc);
	fScalerContext = SkScalerContext::Create(desc);
	memset(fHash, 0, sizeof(fHash));

	fScalerContext->getLineHeight(&fAbove, &fBelow);
	(void)autoFree.detach();
}

SkGlyphCache::~SkGlyphCache()
{
	SkGlyph**	gptr = fGlyphArray.begin();
	SkGlyph**	stop = fGlyphArray.end();
	while (gptr < stop)
	{
		SkPath* path = (*gptr)->fPath;
		if (path)
			SkDELETE(path);
		gptr += 1;
	}
	SkDescriptor::Free(fDesc);
	SkDELETE(fScalerContext);
}

const SkGlyph& SkGlyphCache::lookupMetrics(SkUnichar charCode)
{
	SkGlyph* glyph;

	int		hi = 0;
	int		count = fGlyphArray.count();

	if (count)
	{
		SkGlyph**	gptr = fGlyphArray.begin();
		int		lo = 0;

		hi = count - 1;
		while (lo < hi)
		{
			int mid = (hi + lo) >> 1;
			if (gptr[mid]->fCharCode < charCode)
				lo = mid + 1;
			else
				hi = mid;
		}
		glyph = gptr[hi];
		if (glyph->fCharCode == charCode)
			goto DONE;

		// check if we need to bump hi before falling though to the allocator
		if (glyph->fCharCode < charCode)
			hi += 1;
	}

	// not found, but hi tells us where to inser the new glyph

	glyph = (SkGlyph*)fGlyphAlloc.alloc(sizeof(SkGlyph), SkChunkAlloc::kThrow_AllocFailType);
	glyph->fCharCode = SkToU16(charCode);
	glyph->fImage = NULL;
	glyph->fPath = NULL;
	fScalerContext->getMetrics(glyph);
	*fGlyphArray.insert(hi) = glyph;

DONE:
	fHash[charCode & kHashMask] = glyph;
	return *glyph;
}

const void* SkGlyphCache::findImage(SkUnichar uni)
{
	// cast away the constness, so we can update fImage if needed
	SkGlyph* glyph = (SkGlyph*)&this->getMetrics(uni);

	if (glyph->fWidth)
	{
		if (glyph->fImage == NULL)
		{
			size_t	size = glyph->computeImageSize();
			glyph->fImage = fImageAlloc.alloc(size, SkChunkAlloc::kReturnNil_AllocFailType);
			fScalerContext->getImage(*glyph);
		}
	}
	return glyph->fImage;
}

const SkPath* SkGlyphCache::findPath(SkUnichar uni)
{
	// cast away the constness, so we can update fImage if needed
	SkGlyph* glyph = (SkGlyph*)&this->getMetrics(uni);

	if (glyph->fWidth)
	{
		if (glyph->fPath == NULL)
		{
			glyph->fPath = SkNEW(SkPath);
			fScalerContext->getPath(*glyph, glyph->fPath);
		}
	}
	return glyph->fPath;
}

void SkGlyphCache::getLineHeight(SkPoint* above, SkPoint* below)
{
	if (above)
		*above = fAbove;
	if (below)
		*below = fBelow;
}

/////////////////////////////////////////////////////////////////

SkGlyphCache* SkGlyphCache::DetachCache(const SkPaint& paint, const SkMatrix* matrix)
{
	return paint.detachCache(matrix);
}

#include "SkGlobals.h"
#include "SkThread.h"

#define SkGlyphCache_GlobalsTag		SkSetFourByteTag('g', 'l', 'f', 'c')

class SkGlyphCache_Globals : public SkGlobals::Rec {
public:
	SkMutex			fMutex;
	SkGlyphCache*	fHead;
};

#ifdef SK_USE_RUNTIME_GLOBALS
	static SkGlobals::Rec* create_globals()
	{
		SkGlyphCache_Globals* rec = SkNEW(SkGlyphCache_Globals);
		rec->fHead = NULL;
		return rec;
	}

	#define FIND_GC_GLOBALS()	*(SkGlyphCache_Globals*)SkGlobals::Find(SkGlyphCache_GlobalsTag, create_globals)
	#define GET_GC_GLOBALS()	*(SkGlyphCache_Globals*)SkGlobals::Get(SkGlyphCache_GlobalsTag)
#else
	static SkGlyphCache_Globals	gGCGlobals;
	#define FIND_GC_GLOBALS()	gGCGlobals
	#define GET_GC_GLOBALS()	gGCGlobals
#endif

SkGlyphCache* SkGlyphCache::DetachCache(const SkDescriptor* desc)
{
	SkASSERT(desc);

	SkGlyphCache_Globals& globals = FIND_GC_GLOBALS();

	globals.fMutex.acquire();
	SkGlyphCache*	cache = globals.fHead;
	SkGlyphCache*	prev = NULL;

	while (cache)
	{
		SkGlyphCache* next = cache->fNext;

		if (*cache->fDesc == *desc)
		{
			if (prev)
				prev->fNext = next;
			else
				globals.fHead = next;
			cache->fNext = NULL;
			break;
		}
		prev = cache;
		cache = next;
	}
	globals.fMutex.release();

	if (cache == NULL)
		cache = SkNEW_ARGS(SkGlyphCache, (desc));
	return cache;
}

void SkGlyphCache::AttachCache(SkGlyphCache* cache)
{
	SkASSERT(cache);
	SkASSERT(cache->fNext == NULL);

	SkGlyphCache_Globals& globals = GET_GC_GLOBALS();

	globals.fMutex.acquire();

	cache->fNext = globals.fHead;
	globals.fHead = cache;

	globals.fMutex.release();
}

bool SkGlyphCache::FreeCache(size_t bytesNeeded)
{
	SkGlyphCache_Globals& globals = FIND_GC_GLOBALS();

	globals.fMutex.acquire();
	SkGlyphCache*	cache = globals.fHead;
	bool			didSomething = (cache != NULL);

	while (cache)
	{
		SkGlyphCache* next = cache->fNext;
		SkDELETE(cache);
		cache = next;
	}

	globals.fHead = NULL;
	globals.fMutex.release();

	return didSomething;
}

