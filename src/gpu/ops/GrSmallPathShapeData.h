/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathShapeData_DEFINED
#define GrSmallPathShapeData_DEFINED

#include "src/core/SkOpts.h"
#include "src/gpu/GrDrawOpAtlas.h"

class GrStyledShape;

class GrSmallPathShapeDataKey {
public:
    // TODO: add a move variant
    GrSmallPathShapeDataKey(const GrSmallPathShapeDataKey& that) {
        fKey.reset(that.fKey.count());
        memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
    }

    GrSmallPathShapeDataKey& operator=(const GrSmallPathShapeDataKey&) = delete;

    // for SDF paths
    GrSmallPathShapeDataKey(const GrStyledShape&, uint32_t dim);

    // for bitmap paths
    GrSmallPathShapeDataKey(const GrStyledShape&, const SkMatrix& ctm);

    bool operator==(const GrSmallPathShapeDataKey & that) const {
        return fKey.count() == that.fKey.count() &&
                0 == memcmp(fKey.get(), that.fKey.get(), sizeof(uint32_t) * fKey.count());
    }

    int count32() const { return fKey.count(); }
    const uint32_t* data() const { return fKey.get(); }

private:
    // The key is composed of the GrStyledShape's key, and either the dimensions of the DF
    // generated for the path (32x32 max, 64x64 max, 128x128 max) if an SDF image or
    // the matrix for the path with only fractional translation.
    SkAutoSTArray<24, uint32_t> fKey;
};

class GrSmallPathShapeData {
public:
    GrSmallPathShapeData(const GrSmallPathShapeDataKey& key) : fKey(key) {}

    const GrSmallPathShapeDataKey fKey;
    SkRect                        fBounds;
    GrDrawOpAtlas::AtlasLocator   fAtlasLocator;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrSmallPathShapeData);

    static inline const GrSmallPathShapeDataKey& GetKey(const GrSmallPathShapeData& data) {
        return data.fKey;
    }

    static inline uint32_t Hash(const GrSmallPathShapeDataKey& key) {
        return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
    }
};

#endif
