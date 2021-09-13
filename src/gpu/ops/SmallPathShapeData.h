/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SmallPathShapeData_DEFINED
#define SmallPathShapeData_DEFINED

#include "src/core/SkOpts.h"
#include "src/gpu/GrDrawOpAtlas.h"

class GrStyledShape;

namespace skgpu::v1 {

class SmallPathShapeDataKey {
public:
    // TODO: add a move variant
    SmallPathShapeDataKey(const SmallPathShapeDataKey& that) {
        fKey.reset(that.fKey.count());
        memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
    }

    SmallPathShapeDataKey& operator=(const SmallPathShapeDataKey&) = delete;

    // for SDF paths
    SmallPathShapeDataKey(const GrStyledShape&, uint32_t dim);

    // for bitmap paths
    SmallPathShapeDataKey(const GrStyledShape&, const SkMatrix& ctm);

    bool operator==(const SmallPathShapeDataKey & that) const {
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

class SmallPathShapeData {
public:
    SmallPathShapeData(const SmallPathShapeDataKey& key) : fKey(key) {}

    const SmallPathShapeDataKey fKey;
    SkRect                      fBounds;
    GrDrawOpAtlas::AtlasLocator fAtlasLocator;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(SmallPathShapeData);

    static inline const SmallPathShapeDataKey& GetKey(const SmallPathShapeData& data) {
        return data.fKey;
    }

    static inline uint32_t Hash(const SmallPathShapeDataKey& key) {
        return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
    }
};

} // namespace skgpu::v1

#endif // SmallPathShapeData_DEFINED
