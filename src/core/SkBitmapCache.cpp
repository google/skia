/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkScaledImageCache.h"
#include "SkMipMap.h"
#include "SkRect.h"

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

struct BitmapKey : public SkScaledImageCache::Key {
public:
    BitmapKey(uint32_t genID, SkScalar scaleX, SkScalar scaleY, const SkIRect& bounds)
    : fGenID(genID)
    , fScaleX(scaleX)
    , fScaleY(scaleY)
    , fBounds(bounds)
    {
        this->init(sizeof(fGenID) + sizeof(fScaleX) + sizeof(fScaleY) + sizeof(fBounds));
    }

    uint32_t    fGenID;
    SkScalar    fScaleX;
    SkScalar    fScaleY;
    SkIRect     fBounds;
};

//////////////////////////////////////////////////////////////////////////////////////////

struct BitmapRec : public SkScaledImageCache::Rec {
    BitmapRec(uint32_t genID, SkScalar scaleX, SkScalar scaleY, const SkIRect& bounds,
              const SkBitmap& result)
        : fKey(genID, scaleX, scaleY, bounds)
        , fBitmap(result)
    {}

    BitmapKey   fKey;
    SkBitmap    fBitmap;
    
    virtual const Key& getKey() const SK_OVERRIDE { return fKey; }
    virtual size_t bytesUsed() const SK_OVERRIDE { return sizeof(fKey) + fBitmap.getSize(); }
};

static bool find_and_return(const BitmapKey& key, SkBitmap* result) {
    const BitmapRec* rec = (BitmapRec*)SkScaledImageCache::FindAndLock(key);
    if (rec) {
        *result = rec->fBitmap;
        SkScaledImageCache::Unlock(rec);

        result->lockPixels();
        if (result->getPixels()) {
            return true;
        }
        // todo: we should explicitly purge rec from the cache at this point, since
        //       it is effectively purged already (has no memory behind it)
        result->reset();
        // fall-through to false
    }
    return false;
}

bool SkBitmapCache::Find(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                         SkBitmap* result) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return false;
    }
    BitmapKey key(src.getGenerationID(), invScaleX, invScaleY, get_bounds_from_bitmap(src));
    return find_and_return(key, result);
}

void SkBitmapCache::Add(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
                        const SkBitmap& result) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return;
    }
    SkScaledImageCache::Add(SkNEW_ARGS(BitmapRec,
                                       (src.getGenerationID(), invScaleX, invScaleY,
                                        get_bounds_from_bitmap(src), result)));
}

bool SkBitmapCache::Find(uint32_t genID, int width, int height, SkBitmap* result) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeWH(width, height));
    return find_and_return(key, result);
}

void SkBitmapCache::Add(uint32_t genID, int width, int height, const SkBitmap& result) {
    SkScaledImageCache::Add(SkNEW_ARGS(BitmapRec, (genID, SK_Scalar1, SK_Scalar1,
                                                   SkIRect::MakeWH(width, height), result)));
}

//////////////////////////////////////////////////////////////////////////////////////////

struct MipMapRec : public SkScaledImageCache::Rec {
    MipMapRec(const SkBitmap& src, const SkMipMap* result)
        : fKey(src.getGenerationID(), 0, 0, get_bounds_from_bitmap(src))
        , fMipMap(SkRef(result))
    {}

    virtual ~MipMapRec() {
        fMipMap->unref();
    }

    BitmapKey       fKey;
    const SkMipMap* fMipMap;
    
    virtual const Key& getKey() const SK_OVERRIDE { return fKey; }
    virtual size_t bytesUsed() const SK_OVERRIDE { return sizeof(fKey) + fMipMap->getSize(); }
};


const SkMipMap* SkMipMapCache::FindAndRef(const SkBitmap& src) {
    BitmapKey key(src.getGenerationID(), 0, 0, get_bounds_from_bitmap(src));
    const MipMapRec* rec = (MipMapRec*)SkScaledImageCache::FindAndLock(key);
    const SkMipMap* result = NULL;
    if (rec) {
        result = SkRef(rec->fMipMap);
        SkScaledImageCache::Unlock(rec);
    }
    return result;
}

void SkMipMapCache::Add(const SkBitmap& src, const SkMipMap* result) {
    if (result) {
        SkScaledImageCache::Add(SkNEW_ARGS(MipMapRec, (src, result)));
    }
}

