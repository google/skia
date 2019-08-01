/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "include/core/SkRect.h"
#include <memory>

class SkBitmap;
class SkImage;
class SkImage_Base;
struct SkImageInfo;
class SkMipMap;
class SkPixmap;
class SkResourceCache;

uint64_t SkMakeResourceCacheSharedIDForBitmap(uint32_t bitmapGenID);

void SkNotifyBitmapGenIDIsStale(uint32_t bitmapGenID);

struct SkBitmapCacheDesc {
    uint32_t    fImageID;       // != 0
    SkIRect     fSubset;        // always set to a valid rect (entire or subset)

    void validate() const {
        SkASSERT(fImageID);
        SkASSERT(fSubset.fLeft >= 0 && fSubset.fTop >= 0);
        SkASSERT(fSubset.width() > 0 && fSubset.height() > 0);
    }

    static SkBitmapCacheDesc Make(const SkImage*);
    static SkBitmapCacheDesc Make(uint32_t genID, const SkIRect& subset);
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
    static const SkMipMap* FindAndRef(const SkBitmapCacheDesc&,
                                      SkResourceCache* localCache = nullptr);
    static const SkMipMap* AddAndRef(const SkImage_Base*,
                                     SkResourceCache* localCache = nullptr);
};

#endif
