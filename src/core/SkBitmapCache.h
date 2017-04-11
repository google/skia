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
    uint32_t    fImageID;       // != 0
    int32_t     fScaledWidth;   // 0 for unscaled
    int32_t     fScaledHeight;  // 0 for unscaled
    SkIRect     fSubset;        // always set to a valid rect (entire or subset)

    void validate() const {
        SkASSERT(fImageID);
        if (fScaledWidth || fScaledHeight) {
            SkASSERT(fScaledWidth && fScaledHeight);
        }
        SkASSERT(fSubset.fLeft >= 0 && fSubset.fTop >= 0);
        SkASSERT(fSubset.width() > 0 && fSubset.height() > 0);
    }

    static SkBitmapCacheDesc Make(const SkBitmap&, int scaledWidth, int scaledHeight);
    static SkBitmapCacheDesc Make(const SkBitmap&);
    static SkBitmapCacheDesc Make(const SkImage*, int scaledWidth, int scaledHeight);
    static SkBitmapCacheDesc Make(const SkImage*);

    // Use with care -- width/height must match the original bitmap/image
    static SkBitmapCacheDesc Make(uint32_t genID, int origWidth, int origHeight);
};

class SkBitmapCache {
public:
    /**
     *  Search based on the desc. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(const SkBitmapCacheDesc&, SkBitmap* result);

    class Rec;
    struct RecDeleter { void operator()(Rec* r) { PrivateDeleteRec(r); } };
    typedef std::unique_ptr<Rec, RecDeleter> RecPtr;

    static RecPtr Alloc(const SkBitmapCacheDesc&, const SkImageInfo&, SkPixmap*);
    static void Add(RecPtr, SkBitmap*);

private:
    static void PrivateDeleteRec(Rec*);
};

class SkMipMapCache {
public:
    // Note: the scaled width/height in desc must be 0, as any other value would not make sense.
    static const SkMipMap* FindAndRef(const SkBitmapCacheDesc&, SkDestinationSurfaceColorMode,
                                      SkResourceCache* localCache = nullptr);
    static const SkMipMap* AddAndRef(const SkBitmap& src, SkDestinationSurfaceColorMode,
                                     SkResourceCache* localCache = nullptr);
};

#endif
