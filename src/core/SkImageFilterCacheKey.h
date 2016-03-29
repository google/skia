/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterCacheKey_DEFINED
#define SkImageFilterCacheKey_DEFINED

struct SkImageFilter::Cache::Key {
    Key(const uint32_t uniqueID, const SkMatrix& matrix,
        const SkIRect& clipBounds, uint32_t srcGenID, const SkIRect& srcSubset)
        : fUniqueID(uniqueID)
        , fMatrix(matrix)
        , fClipBounds(clipBounds)
        , fSrcGenID(srcGenID)
        , fSrcSubset(srcSubset) {
        // Assert that Key is tightly-packed, since it is hashed.
        static_assert(sizeof(Key) == sizeof(uint32_t) + sizeof(SkMatrix) + sizeof(SkIRect) +
                                     sizeof(uint32_t) + 4 * sizeof(int32_t),
                                     "image_filter_key_tight_packing");
        fMatrix.getType();  // force initialization of type, so hashes match
    }

    uint32_t fUniqueID;
    SkMatrix fMatrix;
    SkIRect fClipBounds;
    uint32_t fSrcGenID;
    SkIRect fSrcSubset;

    bool operator==(const Key& other) const {
        return fUniqueID == other.fUniqueID &&
               fMatrix == other.fMatrix &&
               fClipBounds == other.fClipBounds &&
               fSrcGenID == other.fSrcGenID &&
               fSrcSubset == other.fSrcSubset;
    }
};

#endif
