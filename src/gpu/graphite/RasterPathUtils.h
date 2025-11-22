/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RasterPathUtils_DEFINED
#define skgpu_graphite_RasterPathUtils_DEFINED

#include "include/core/SkBitmap.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/base/SkVx.h"
#include "src/core/SkDraw.h"
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
    // Let RasterMaskHelper allocate the backing store (A8). The caller can optionally
    // provide a padding and a translation applied to each draw. The padding is added to
    // the size on all four sides but excluded from the area rendered to. That is, (0,0)
    // will refer to the top-left pixel *inside* the padding. Since each draw method
    // takes a Transform, the translation is just a convenience to save repeated
    // concatenations when the caller already has prebuilt Transforms. The entire
    // allocation (including the padding) is initialized to initialAlpha.
    static std::tuple<SkBitmap, RasterMaskHelper> Allocate(SkISize size,
                                                           SkIVector translation = {0, 0},
                                                           int padding = 0,
                                                           SkAlpha initialAlpha = 0);
    // The caller provides the storage, which must be A8.
    explicit RasterMaskHelper(SkPixmap pixmap, SkIVector translation = {0, 0});

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    void drawShape(const Shape& shape,
                   const Transform& localToDevice,
                   const SkStrokeRec& strokeRec);

    // Draw a single shape into the bitmap (as a path) at location resultBounds.
    // Variant used for clipping.
    void drawClip(const Shape& shape, const Transform& localToDevice, uint8_t alpha);

private:
    SkPixmap     fPixels;
    SkRasterClip fRasterClip;
    SkVector     fTranslate;
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
