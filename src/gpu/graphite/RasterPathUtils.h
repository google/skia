/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RasterPathUtils_DEFINED
#define skgpu_graphite_RasterPathUtils_DEFINED

#include "include/private/base/SkNoncopyable.h"
#include "src/base/SkVx.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ClipStack_graphite.h"

namespace skgpu::graphite {

class Shape;
class Transform;

/**
 * The RasterMaskHelper helps generate masks using the software rendering
 * path. It is intended to be used as:
 *
 *   RasterMaskHelper helper(pixmapstorage);
 *   helper.init(...);
 *   helper.drawShape(...);
 *
 * The result of this process will be the mask rendered in the Pixmap,
 * at the upper left hand corner of the bounds.
 *
 * TODO: this could be extended to support clip masks, similar to GrSWMaskHelper.
 */

class RasterMaskHelper : SkNoncopyable {
public:
    RasterMaskHelper(SkAutoPixmapStorage* pixels) : fPixels(pixels) {}

    bool init(SkISize pixmapSize, skvx::float2 transformedMaskOffset);

    void clear(uint8_t alpha, const SkIRect& resultBounds) {
        SkPaint paint;
        paint.setColor(SkColorSetARGB(alpha, 0xFF, 0xFF, 0xFF));
        fDraw.drawRect(SkRect::Make(resultBounds), paint);
    }

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    void drawShape(const Shape& shape,
                   const Transform& localToDevice,
                   const SkStrokeRec& strokeRec,
                   const SkIRect& resultBounds);

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    // Variant used for clipping.
    void drawClip(const Shape& shape,
                  const Transform& transform,
                  uint8_t alpha,
                  const SkIRect& resultBounds);

private:
    SkAutoPixmapStorage* fPixels;
    SkDrawBase           fDraw;
    skvx::float2         fTransformedMaskOffset = {0};
    SkRasterClip         fRasterClip;
};

skgpu::UniqueKey GeneratePathMaskKey(const Shape& shape,
                                     const Transform& transform,
                                     const SkStrokeRec& strokeRec,
                                     skvx::half2 maskOrigin,
                                     skvx::half2 maskSize);

skgpu::UniqueKey GenerateClipMaskKey(uint32_t stackRecordID,
                                     const ClipStack::ElementList* elementsForMask);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathUtils_DEFINED
