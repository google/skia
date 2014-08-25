/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
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

SkScaledImageCache::ID* SkBitmapCache::FindAndLock(const SkBitmap& src,
                                                   SkScalar invScaleX, SkScalar invScaleY,
                                                   SkBitmap* result) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return NULL;
    }
    BitmapKey key(src.getGenerationID(), invScaleX, invScaleY, get_bounds_from_bitmap(src));
    return SkScaledImageCache::FindAndLock(key, result);
}

SkScaledImageCache::ID* SkBitmapCache::AddAndLock(const SkBitmap& src,
                                                  SkScalar invScaleX, SkScalar invScaleY,
                                                  const SkBitmap& result) {
    if (0 == invScaleX || 0 == invScaleY) {
        // degenerate, and the key we use for mipmaps
        return NULL;
    }
    BitmapKey key(src.getGenerationID(), invScaleX, invScaleY, get_bounds_from_bitmap(src));
    return SkScaledImageCache::AddAndLock(key, result);
}

////

SkScaledImageCache::ID* SkBitmapCache::FindAndLock(uint32_t genID, int width, int height,
                                                   SkBitmap* result) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeWH(width, height));
    return SkScaledImageCache::FindAndLock(key, result);
}

SkScaledImageCache::ID* SkBitmapCache::AddAndLock(uint32_t genID, int width, int height,
                                                     const SkBitmap& result) {
    BitmapKey key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeWH(width, height));
    return SkScaledImageCache::AddAndLock(key, result);
}

////

SkScaledImageCache::ID* SkMipMapCache::FindAndLock(const SkBitmap& src, const SkMipMap** result) {
    BitmapKey key(src.getGenerationID(), SK_Scalar1, SK_Scalar1, get_bounds_from_bitmap(src));
    return SkScaledImageCache::FindAndLock(key, result);
}

SkScaledImageCache::ID* SkMipMapCache::AddAndLock(const SkBitmap& src, const SkMipMap* result) {
    BitmapKey key(src.getGenerationID(), SK_Scalar1, SK_Scalar1, get_bounds_from_bitmap(src));
    return SkScaledImageCache::AddAndLock(key, result);
}

