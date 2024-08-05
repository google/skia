/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/SmallPathShapeData.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkFixed.h"
#include "src/base/SkFloatBits.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

namespace skgpu::ganesh {

SmallPathShapeDataKey::SmallPathShapeDataKey(const GrStyledShape& shape, uint32_t dim) {
    // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
    // relevant styling information.
    SkASSERT(shape.style().isSimpleFill());
    SkASSERT(shape.hasUnstyledKey());
    int shapeKeySize = shape.unstyledKeySize();
    fKey.reset(1 + shapeKeySize);
    fKey[0] = dim;
    shape.writeUnstyledKey(&fKey[1]);
}

SmallPathShapeDataKey::SmallPathShapeDataKey(const GrStyledShape& shape, const SkMatrix& ctm) {
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

}  // namespace skgpu::ganesh

#endif // SK_ENABLE_OPTIMIZE_SIZE
