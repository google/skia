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
#include "src/gpu/graphite/ClipStack.h"

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
 * This can be used for clip masks as well, by doing:
 *   helper.drawClip(...);
 *
 * Rasterized clip masks will include the inversion in the mask; rasterized path
 * masks assume that the CoverageMask shader will handle the inversion.
 */

class RasterMaskHelper : SkNoncopyable {
public:
    RasterMaskHelper(SkAutoPixmapStorage* pixels) : fPixels(pixels) {}

    bool init(SkISize pixmapSize, SkIVector transformedMaskOffset);

    void clear(uint8_t alpha, const SkIRect& drawBounds);

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    void drawShape(const Shape& shape,
                   const Transform& localToDevice,
                   const SkStrokeRec& strokeRec,
                   const SkIRect& drawBounds);

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    // Variant used for clipping.
    void drawClip(const Shape& shape,
                  const Transform& localToDevice,
                  uint8_t alpha,
                  const SkIRect& drawBounds);

private:
    SkAutoPixmapStorage* fPixels;
    SkDrawBase           fDraw;
    SkIVector            fTransformedMaskOffset = {0, 0};
    SkRasterClip         fRasterClip;
};

skgpu::UniqueKey GeneratePathMaskKey(const Shape& shape,
                                     const Transform& transform,
                                     const SkStrokeRec& strokeRec,
                                     skvx::half2 maskOrigin,
                                     skvx::half2 maskSize);

skgpu::UniqueKey GenerateClipMaskKey(uint32_t stackRecordID,
                                     const ClipStack::ElementList* elementsForMask,
                                     SkIRect maskDeviceBounds,
                                     bool includeBounds,
                                     SkIRect* keyBounds,
                                     bool* usesPathKey);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathUtils_DEFINED
