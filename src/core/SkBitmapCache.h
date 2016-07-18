/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "SkBitmap.h"
#include "SkMipMap.h"

class SkImage;
class SkResourceCache;

uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID);

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID);

struct SkBitmapCacheDesc {
    uint32_t    fImageID;
    int32_t     fWidth;
    int32_t     fHeight;
    SkIRect     fBounds;

    static SkBitmapCacheDesc Make(const SkBitmap&, int width, int height);
    static SkBitmapCacheDesc Make(const SkBitmap&);
    static SkBitmapCacheDesc Make(const SkImage*, int width, int height);
    static SkBitmapCacheDesc Make(const SkImage*);
};

class SkBitmapCache {
public:
    /**
     * Use this allocator for bitmaps, so they can use ashmem when available.
     * Returns nullptr if the ResourceCache has not been initialized with a DiscardableFactory.
     */
    static SkBitmap::Allocator* GetAllocator();

    /**
     *  Search based on the desc. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool FindWH(const SkBitmapCacheDesc&, SkBitmap* result,
                       SkResourceCache* localCache = nullptr);

    /*
     *  result must be marked isImmutable()
     */
    static bool AddWH(const SkBitmapCacheDesc&, const SkBitmap& result,
                      SkResourceCache* localCache = nullptr);

    /**
     *  Search based on the bitmap's genID and subset. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(uint32_t genID, const SkIRect& subset, SkBitmap* result,
                     SkResourceCache* localCache = nullptr);

    /**
     * The width and the height of the provided subset must be the same as the result bitmap ones.
     * result must be marked isImmutable()
     */
    static bool Add(SkPixelRef*, const SkIRect& subset, const SkBitmap& result,
                    SkResourceCache* localCache = nullptr);

    static bool Find(uint32_t genID, SkBitmap* result, SkResourceCache* localCache = nullptr);
    // todo: eliminate the need to specify ID, since it should == the bitmap's
    static void Add(uint32_t genID, const SkBitmap&, SkResourceCache* localCache = nullptr);
};

class SkMipMapCache {
public:
    static const SkMipMap* FindAndRef(const SkBitmapCacheDesc&, SkSourceGammaTreatment,
                                      SkResourceCache* localCache = nullptr);
    static const SkMipMap* AddAndRef(const SkBitmap& src, SkSourceGammaTreatment,
                                     SkResourceCache* localCache = nullptr);
};

#endif
