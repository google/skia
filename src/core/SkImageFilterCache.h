/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterCache_DEFINED
#define SkImageFilterCache_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkImageFilterTypes.h"

struct SkIPoint;
class SkImageFilter;

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
        SkASSERT(fMatrix.isFinite());   // otherwise we can't rely on == self when comparing keys
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

// This cache maps from (filter's unique ID + CTM + clipBounds + src bitmap generation ID) to result
// NOTE: this is the _specific_ unique ID of the image filter, so refiltering the same image with a
// copy of the image filter (with exactly the same parameters) will not yield a cache hit.
class SkImageFilterCache : public SkRefCnt {
public:
    enum { kDefaultTransientSize = 32 * 1024 * 1024 };

    ~SkImageFilterCache() override {}
    static SkImageFilterCache* Create(size_t maxBytes);
    static SkImageFilterCache* Get();

    // Returns true on cache hit and updates 'result' to be the cached result. Returns false when
    // not in the cache, in which case 'result' is not modified.
    virtual bool get(const SkImageFilterCacheKey& key,
                     skif::FilterResult* result) const = 0;
    // 'filter' is included in the caching to allow the purging of all of an image filter's cached
    // results when it is destroyed.
    virtual void set(const SkImageFilterCacheKey& key, const SkImageFilter* filter,
                     const skif::FilterResult& result) = 0;
    virtual void purge() = 0;
    virtual void purgeByImageFilter(const SkImageFilter*) = 0;
    SkDEBUGCODE(virtual int count() const = 0;)
};

#endif
