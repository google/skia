/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkImage.h"
#include "SkResourceCache.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkRect.h"

/**
 *  Use this for bitmapcache and mipmapcache entries.
 */
uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID) {
    uint64_t sharedID = SkSetFourByteTag('b', 'm', 'a', 'p');
    return (sharedID << 32) | bitmapGenID;
}

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID) {
    SkResourceCache::PostPurgeSharedID(SkMakeResourceCacheSharedIDForBitmap(bitmapGenID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmap::Allocator* SkBitmapCache::GetAllocator() {
    return SkResourceCache::GetAllocator();
}

/**
 This function finds the bounds of the bitmap *within its pixelRef*.
 If the bitmap lacks a pixelRef, it will return an empty rect, since
 that doesn't make sense.  This may be a useful enough function that
 it should be somewhere else (in SkBitmap?).
 */
static SkIRect get_bounds_from_bitmap(const SkBitmap& bm) {
    if (!(bm.pixelRef())) {
        return SkIRect::MakeEmpty();
    }
    SkIPoint origin = bm.pixelRefOrigin();
    return SkIRect::MakeXYWH(origin.fX, origin.fY, bm.width(), bm.height());
}

/**
 *  This function finds the bounds of the image. Today this is just the entire bounds,
 *  but in the future we may support subsets within an image, in which case this should
 *  return that subset (see get_bounds_from_bitmap).
 */
static SkIRect get_bounds_from_image(const SkImage* image) {
    SkASSERT(image->width() > 0 && image->height() > 0);
    return SkIRect::MakeWH(image->width(), image->height());
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(uint32_t imageID, int origWidth, int origHeight) {
    SkASSERT(imageID);
    SkASSERT(origWidth > 0 && origHeight > 0);
    return { imageID, 0, 0, {0, 0, origWidth, origHeight} };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkBitmap& bm, int scaledWidth, int scaledHeight) {
    SkASSERT(bm.width() > 0 && bm.height() > 0);
    SkASSERT(scaledWidth > 0 && scaledHeight > 0);
    SkASSERT(scaledWidth != bm.width() || scaledHeight != bm.height());

    return { bm.getGenerationID(), scaledWidth, scaledHeight, get_bounds_from_bitmap(bm) };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkBitmap& bm) {
    SkASSERT(bm.width() > 0 && bm.height() > 0);
    SkASSERT(bm.pixelRefOrigin() == SkIPoint::Make(0, 0));

    return { bm.getGenerationID(), 0, 0, get_bounds_from_bitmap(bm) };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image, int scaledWidth, int scaledHeight) {
    SkASSERT(image->width() > 0 && image->height() > 0);
    SkASSERT(scaledWidth > 0 && scaledHeight > 0);

    // If the dimensions are the same, should we set them to 0,0?
    //SkASSERT(scaledWidth != image->width() || scaledHeight != image->height());

    return { image->uniqueID(), scaledWidth, scaledHeight, get_bounds_from_image(image) };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image) {
    SkASSERT(image->width() > 0 && image->height() > 0);

    return { image->uniqueID(), 0, 0, get_bounds_from_image(image) };
}

namespace {
static unsigned gBitmapKeyNamespaceLabel;

struct BitmapKey : public SkResourceCache::Key {
public:
    BitmapKey(const SkBitmapCacheDesc& desc) : fDesc(desc) {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fDesc.fImageID),
                   sizeof(fDesc));
    }

    void dump() const {
        SkDebugf("-- add [%d %d] %d [%d %d %d %d]\n",
                 fDesc.fScaledWidth, fDesc.fScaledHeight, fDesc.fImageID,
             fDesc.fSubset.x(), fDesc.fSubset.y(), fDesc.fSubset.width(), fDesc.fSubset.height());
    }

    const SkBitmapCacheDesc fDesc;
};

struct BitmapRec : public SkResourceCache::Rec {
    BitmapRec(const SkBitmapCacheDesc& desc, const SkBitmap& result)
        : fKey(desc)
        , fBitmap(result)
    {
#ifdef TRACE_NEW_BITMAP_CACHE_RECS
        fKey.dump();
#endif
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fBitmap.getSize(); }

    const char* getCategory() const override { return "bitmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fBitmap.pixelRef()->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextBitmap) {
        const BitmapRec& rec = static_cast<const BitmapRec&>(baseRec);
        SkBitmap* result = (SkBitmap*)contextBitmap;

        *result = rec.fBitmap;
        result->lockPixels();
        return SkToBool(result->getPixels());
    }

private:
    BitmapKey   fKey;
    SkBitmap    fBitmap;
};
} // namespace

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

bool SkBitmapCache::Find(const SkBitmapCacheDesc& desc, SkBitmap* result,
                         SkResourceCache* localCache) {
    desc.validate();
    return CHECK_LOCAL(localCache, find, Find, BitmapKey(desc), BitmapRec::Finder, result);
}

bool SkBitmapCache::Add(const SkBitmapCacheDesc& desc, const SkBitmap& result,
                        SkResourceCache* localCache) {
    desc.validate();
    SkASSERT(result.isImmutable());
    BitmapRec* rec = new BitmapRec(desc, result);
    CHECK_LOCAL(localCache, add, Add, rec);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

namespace {
static unsigned gMipMapKeyNamespaceLabel;

struct MipMapKey : public SkResourceCache::Key {
public:
    MipMapKey(uint32_t imageID, const SkIRect& subset, SkDestinationSurfaceColorMode colorMode)
        : fImageID(imageID)
        , fColorMode(static_cast<uint32_t>(colorMode))
        , fSubset(subset)
    {
        SkASSERT(fImageID);
        SkASSERT(!subset.isEmpty());
        this->init(&gMipMapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(fImageID),
                   sizeof(fImageID) + sizeof(fColorMode) + sizeof(fSubset));
    }

    uint32_t    fImageID;
    uint32_t    fColorMode;
    SkIRect     fSubset;
};

struct MipMapRec : public SkResourceCache::Rec {
    MipMapRec(uint32_t imageID, const SkIRect& subset, SkDestinationSurfaceColorMode colorMode,
              const SkMipMap* result)
        : fKey(imageID, subset, colorMode)
        , fMipMap(result)
    {
        fMipMap->attachToCacheAndRef();
    }

    ~MipMapRec() override {
        fMipMap->detachFromCacheAndUnref();
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fMipMap->size(); }
    const char* getCategory() const override { return "mipmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fMipMap->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextMip) {
        const MipMapRec& rec = static_cast<const MipMapRec&>(baseRec);
        const SkMipMap* mm = SkRef(rec.fMipMap);
        // the call to ref() above triggers a "lock" in the case of discardable memory,
        // which means we can now check for null (in case the lock failed).
        if (nullptr == mm->data()) {
            mm->unref();    // balance our call to ref()
            return false;
        }
        // the call must call unref() when they are done.
        *(const SkMipMap**)contextMip = mm;
        return true;
    }

private:
    MipMapKey       fKey;
    const SkMipMap* fMipMap;
};
}

const SkMipMap* SkMipMapCache::FindAndRef(const SkBitmapCacheDesc& desc,
                                          SkDestinationSurfaceColorMode colorMode,
                                          SkResourceCache* localCache) {
    SkASSERT(desc.fScaledWidth == 0);
    SkASSERT(desc.fScaledHeight == 0);
    MipMapKey key(desc.fImageID, desc.fSubset, colorMode);
    const SkMipMap* result;

    if (!CHECK_LOCAL(localCache, find, Find, key, MipMapRec::Finder, &result)) {
        result = nullptr;
    }
    return result;
}

static SkResourceCache::DiscardableFactory get_fact(SkResourceCache* localCache) {
    return localCache ? localCache->GetDiscardableFactory()
                      : SkResourceCache::GetDiscardableFactory();
}

const SkMipMap* SkMipMapCache::AddAndRef(const SkBitmap& src,
                                         SkDestinationSurfaceColorMode colorMode,
                                         SkResourceCache* localCache) {
    SkMipMap* mipmap = SkMipMap::Build(src, colorMode, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = new MipMapRec(src.getGenerationID(), get_bounds_from_bitmap(src),
                                       colorMode, mipmap);
        CHECK_LOCAL(localCache, add, Add, rec);
        src.pixelRef()->notifyAddedToCache();
    }
    return mipmap;
}
