
/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrSmallPathShapeData_DEFINED
#define GrSmallPathShapeData_DEFINED

#include "src/core/SkOpts.h"

class GrSmallPathShapeDataKey  {
public:
    GrSmallPathShapeDataKey () {}
    GrSmallPathShapeDataKey (const GrSmallPathShapeDataKey & that) { *this = that; }
    GrSmallPathShapeDataKey (const GrStyledShape& shape, uint32_t dim) { this->set(shape, dim); }
    GrSmallPathShapeDataKey (const GrStyledShape& shape, const SkMatrix& ctm) {
        this->set(shape, ctm);
    }

    GrSmallPathShapeDataKey & operator=(const GrSmallPathShapeDataKey & that) {
        fKey.reset(that.fKey.count());
        memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
        return *this;
    }

    // for SDF paths
    void set(const GrStyledShape& shape, uint32_t dim) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(1 + shapeKeySize);
        fKey[0] = dim;
        shape.writeUnstyledKey(&fKey[1]);
    }

    // for bitmap paths
    void set(const GrStyledShape& shape, const SkMatrix& ctm) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = ctm.get(SkMatrix::kMScaleX);
        SkScalar sy = ctm.get(SkMatrix::kMScaleY);
        SkScalar kx = ctm.get(SkMatrix::kMSkewX);
        SkScalar ky = ctm.get(SkMatrix::kMSkewY);
        SkScalar tx = ctm.get(SkMatrix::kMTransX);
        SkScalar ty = ctm.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        tx -= SkScalarFloorToScalar(tx);
        ty -= SkScalarFloorToScalar(ty);
        SkFixed fracX = SkScalarToFixed(tx) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(ty) & 0x0000FF00;
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(5 + shapeKeySize);
        fKey[0] = SkFloat2Bits(sx);
        fKey[1] = SkFloat2Bits(sy);
        fKey[2] = SkFloat2Bits(kx);
        fKey[3] = SkFloat2Bits(ky);
        fKey[4] = fracX | (fracY >> 8);
        shape.writeUnstyledKey(&fKey[5]);
    }

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

class GrSmallPathShapeData  {
public:
    GrSmallPathShapeData(const GrSmallPathShapeDataKey &key) : fKey(key) {}

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
