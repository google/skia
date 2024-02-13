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

    bool init(SkISize pixmapSize);

    // Draw a single shape into the bitmap (as a path) at location resultBounds
    void drawShape(const Shape& shape,
                   const Transform& transform,
                   const SkStrokeRec& strokeRec,
                   const SkIRect& resultBounds);

private:
    SkAutoPixmapStorage* fPixels;
    SkDrawBase           fDraw;
    SkRasterClip         fRasterClip;
};

skgpu::UniqueKey GeneratePathMaskKey(const Shape& shape,
                                     const Transform& transform,
                                     const SkStrokeRec& strokeRec,
                                     skvx::half2 maskSize);
}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathUtils_DEFINED
