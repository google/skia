#ifndef SkGlyphCache_DEFINED
#define SkGlyphCache_DEFINED

#include "SkBitmap.h"
#include "SkChunkAlloc.h"
#include "SkDescriptor.h"
#include "SkScalerContext.h"
#include "SkTemplates.h"

class SkPaint;

class SkGlyphCache {
public:
	const SkGlyph& getMetrics(SkUnichar charCode)
	{
		int		 hash = charCode & kHashMask;
		SkGlyph* glyph = fHash[hash];

		if (glyph != nil && glyph->fCharCode == charCode)
			return *glyph;
		return this->lookupMetrics(charCode);
	}


	const void*		findImage(SkUnichar);
	const SkPath*	findPath(SkUnichar);
	void			getLineHeight(SkPoint* above, SkPoint* below);

	static SkGlyphCache* DetachCache(const SkPaint&, const SkMatrix* matrix);
	static SkGlyphCache* DetachCache(const SkDescriptor*);
	static void			 AttachCache(SkGlyphCache*);
	static bool			 FreeCache(size_t bytesNeeded);

private:
	SkGlyphCache(const SkDescriptor*);
	~SkGlyphCache();

	const SkGlyph& lookupMetrics(SkUnichar charCode);

	SkGlyphCache*		fNext;
	SkDescriptor*		fDesc;
	SkScalerContext*	fScalerContext;

	enum {
		kHashBits	= 6,
		kHashCount	= 1 << kHashBits,
		kHashMask	= kHashCount - 1
	};
	SkGlyph*			fHash[kHashCount];
	SkTDArray<SkGlyph*>	fGlyphArray;
	SkChunkAlloc		fGlyphAlloc;
	SkChunkAlloc		fImageAlloc;

	SkPoint	fAbove, fBelow;
};

class SkAutoGlyphCache {
public:
	SkAutoGlyphCache(SkGlyphCache* cache) : fCache(cache) {}
	SkAutoGlyphCache(const SkDescriptor* desc)
	{
		fCache = SkGlyphCache::DetachCache(desc);
	}
	SkAutoGlyphCache(const SkPaint& paint, const SkMatrix* matrix)
	{
		fCache = SkGlyphCache::DetachCache(paint, matrix);
	}
	~SkAutoGlyphCache()
	{
		if (fCache)
			SkGlyphCache::AttachCache(fCache);
	}

	SkGlyphCache*	getCache() const { return fCache; }

	void release()
	{
		if (fCache)
		{
			SkGlyphCache::AttachCache(fCache);
			fCache = nil;
		}
	}
private:
	SkGlyphCache*	fCache;
};

#endif

