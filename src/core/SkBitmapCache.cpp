/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
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

namespace {
static unsigned gBitmapKeyNamespaceLabel;

struct BitmapKey : public SkResourceCache::Key {
public:
    BitmapKey(uint32_t genID, SkScalar sx, SkScalar sy, const SkIRect& bounds)
        : fGenID(genID)
        , fScaleX(sx)
        , fScaleY(sy)
        , fBounds(bounds)
    {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(genID),
                   sizeof(fGenID) + sizeof(fScaleX) + sizeof(fScaleY) + sizeof(fBounds));
    }

    uint32_t    fGenID;
    SkScalar    fScaleX;
    SkScalar    fScaleY;
    SkIRect     fBounds;
};

struct BitmapRec : public SkResourceCache::Rec {
    BitmapRec(uint32_t genID, SkScalar scaleX, SkScalar scaleY, const SkIRect& bounds,
              const SkBitmap& result)
        : fKey(genID, scaleX, scaleY, bounds)
        , fBitmap(result)
    {}

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fBitmap.getSize(); }

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

bool SkBitmapCache::Find(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY, SkBitmap* result,
                         SkResourceCache* localCache) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return false;
    }
    BitmapKey key(src.getGenerationID(), invScaleX, invScaleY, get_bounds_from_bitmap(src));

    return CHECK_LOCAL(localCache, find, Find, key, BitmapRec::Finder, result);
}

void SkBitmapCache::Add(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                        const SkBitmap& result, SkResourceCache* localCache) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return;
    }
    SkASSERT(result.isImmutable());
    BitmapRec* rec = SkNEW_ARGS(BitmapRec, (src.getGenerationID(), invScaleX, invScaleY,
                                            get_bounds_from_bitmap(src), result));
    CHECK_LOCAL(localCache, add, Add, rec);
    src.pixelRef()->notifyAddedToCache();
}

bool SkBitmapCache::Find(uint32_t genID, const SkIRect& subset, SkBitmap* result,
                         SkResourceCache* localCache) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, subset);

    return CHECK_LOCAL(localCache, find, Find, key, BitmapRec::Finder, result);
}

bool SkBitmapCache::Add(SkPixelRef* pr, const SkIRect& subset, const SkBitmap& result,
                        SkResourceCache* localCache) {
    SkASSERT(result.isImmutable());

    if (subset.isEmpty()
        || subset.top() < 0
        || subset.left() < 0
        || result.width() != subset.width()
        || result.height() != subset.height()) {
        return false;
    } else {
        BitmapRec* rec = SkNEW_ARGS(BitmapRec, (pr->getGenerationID(), 1, 1, subset, result));

        CHECK_LOCAL(localCache, add, Add, rec);
        pr->notifyAddedToCache();
        return true;
    }
}

bool SkBitmapCache::Find(uint32_t genID, SkBitmap* result, SkResourceCache* localCache) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeEmpty());

    return CHECK_LOCAL(localCache, find, Find, key, BitmapRec::Finder, result);
}

void SkBitmapCache::Add(uint32_t genID, const SkBitmap& result, SkResourceCache* localCache) {
    SkASSERT(result.isImmutable());

    BitmapRec* rec = SkNEW_ARGS(BitmapRec, (genID, 1, 1, SkIRect::MakeEmpty(), result));

    CHECK_LOCAL(localCache, add, Add, rec);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

namespace {
static unsigned gMipMapKeyNamespaceLabel;

struct MipMapKey : public SkResourceCache::Key {
public:
    MipMapKey(uint32_t genID, const SkIRect& bounds) : fGenID(genID), fBounds(bounds) {
        this->init(&gMipMapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(genID),
                   sizeof(fGenID) + sizeof(fBounds));
    }

    uint32_t    fGenID;
    SkIRect     fBounds;
};

struct MipMapRec : public SkResourceCache::Rec {
    MipMapRec(const SkBitmap& src, const SkMipMap* result)
        : fKey(src.getGenerationID(), get_bounds_from_bitmap(src))
        , fMipMap(result)
    {
        fMipMap->attachToCacheAndRef();
    }

    virtual ~MipMapRec() {
        fMipMap->detachFromCacheAndUnref();
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fMipMap->size(); }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextMip) {
        const MipMapRec& rec = static_cast<const MipMapRec&>(baseRec);
        const SkMipMap* mm = SkRef(rec.fMipMap);
        // the call to ref() above triggers a "lock" in the case of discardable memory,
        // which means we can now check for null (in case the lock failed).
        if (NULL == mm->data()) {
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

const SkMipMap* SkMipMapCache::FindAndRef(const SkBitmap& src, SkResourceCache* localCache) {
    MipMapKey key(src.getGenerationID(), get_bounds_from_bitmap(src));
    const SkMipMap* result;

    if (!CHECK_LOCAL(localCache, find, Find, key, MipMapRec::Finder, &result)) {
        result = NULL;
    }
    return result;
}

static SkResourceCache::DiscardableFactory get_fact(SkResourceCache* localCache) {
    return localCache ? localCache->GetDiscardableFactory()
                      : SkResourceCache::GetDiscardableFactory();
}

const SkMipMap* SkMipMapCache::AddAndRef(const SkBitmap& src, SkResourceCache* localCache) {
    SkMipMap* mipmap = SkMipMap::Build(src, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = SkNEW_ARGS(MipMapRec, (src, mipmap));
        CHECK_LOCAL(localCache, add, Add, rec);
        src.pixelRef()->notifyAddedToCache();
    }
    return mipmap;
}
