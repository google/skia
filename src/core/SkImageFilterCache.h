/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterCache_DEFINED
#define SkImageFilterCache_DEFINED

#include "SkMatrix.h"
#include "SkRefCnt.h"

struct SkIPoint;
class SkImageFilter;
class SkSpecialImage;

struct SkImageFilterCacheKey {
    SkImageFilterCacheKey(const uint32_t uniqueID, const SkMatrix& matrix,
        const SkIRect& clipBounds, uint32_t srcGenID, const SkIRect& srcSubset)
        : fUniqueID(uniqueID)
        , fMatrix(matrix)
        , fClipBounds(clipBounds)
        , fSrcGenID(srcGenID)
        , fSrcSubset(srcSubset) {
        // Assert that Key is tightly-packed, since it is hashed.
        static_assert(sizeof(SkImageFilterCacheKey) == sizeof(uint32_t) + sizeof(SkMatrix) +
                                     sizeof(SkIRect) + sizeof(uint32_t) + 4 * sizeof(int32_t),
                                     "image_filter_key_tight_packing");
        fMatrix.getType();  // force initialization of type, so hashes match
    }

    uint32_t fUniqueID;
    SkMatrix fMatrix;
    SkIRect fClipBounds;
    uint32_t fSrcGenID;
    SkIRect fSrcSubset;

    bool operator==(const SkImageFilterCacheKey& other) const {
        return fUniqueID == other.fUniqueID &&
               fMatrix == other.fMatrix &&
               fClipBounds == other.fClipBounds &&
               fSrcGenID == other.fSrcGenID &&
               fSrcSubset == other.fSrcSubset;
    }
};

// This cache maps from (filter's unique ID + CTM + clipBounds + src bitmap generation ID) to
// (result, offset).
class SkImageFilterCache : public SkRefCnt {
public:
    enum { kDefaultTransientSize = 32 * 1024 * 1024 };

    virtual ~SkImageFilterCache() {}
    static SkImageFilterCache* Create(size_t maxBytes);
    static SkImageFilterCache* Get();
    virtual sk_sp<SkSpecialImage> get(const SkImageFilterCacheKey& key, SkIPoint* offset) const = 0;
    virtual void set(const SkImageFilterCacheKey& key, SkSpecialImage* image,
                     const SkIPoint& offset, const SkImageFilter* filter) = 0;
    virtual void purge() = 0;
    virtual void purgeByKeys(const SkImageFilterCacheKey[], int) = 0;
    SkDEBUGCODE(virtual int count() const = 0;)
};

#endif
