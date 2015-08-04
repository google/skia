/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "SkScalar.h"
#include "SkBitmap.h"

class SkResourceCache;
class SkMipMap;

uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID);

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID);

class SkBitmapCache {
public:
    /**
     * Use this allocator for bitmaps, so they can use ashmem when available.
     * Returns NULL if the ResourceCache has not been initialized with a DiscardableFactory.
     */
    static SkBitmap::Allocator* GetAllocator();

    /**
     *  Search based on the src bitmap and inverse scales in X and Y. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY, SkBitmap* result,
                     SkResourceCache* localCache = NULL);

    /*
     *  result must be marked isImmutable()
     */
    static void Add(const SkBitmap& src, SkScalar invScaleX, SkScalar invScaleY,
            const SkBitmap& result, SkResourceCache* localCache = NULL);

    /**
     *  Search based on the bitmap's genID and subset. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(uint32_t genID, const SkIRect& subset, SkBitmap* result,
                     SkResourceCache* localCache = NULL);

    /**
     * The width and the height of the provided subset must be the same as the result bitmap ones.
     * result must be marked isImmutable()
     */
    static bool Add(SkPixelRef*, const SkIRect& subset, const SkBitmap& result,
                    SkResourceCache* localCache = NULL);

    static bool Find(uint32_t genID, SkBitmap* result, SkResourceCache* localCache = NULL);
    // todo: eliminate the need to specify ID, since it should == the bitmap's
    static void Add(uint32_t genID, const SkBitmap&, SkResourceCache* localCache = NULL);
};

class SkMipMapCache {
public:
    static const SkMipMap* FindAndRef(const SkBitmap& src, SkResourceCache* localCache = NULL);
    static const SkMipMap* AddAndRef(const SkBitmap& src, SkResourceCache* localCache = NULL);
};

#endif
