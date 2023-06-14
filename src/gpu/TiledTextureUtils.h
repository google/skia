/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_TiledTextureUtils_DEFINED
#define skgpu_TiledTextureUtils_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkTileMode.h"

class GrClip;
class GrRecordingContext;
class SkBitmap;
struct SkIRect;
struct SkISize;
class SkMatrix;
class SkMatrixProvider;
class SkPaint;
struct SkRect;
struct SkSamplingOptions;

namespace skgpu::ganesh {
    class SurfaceDrawContext;
}

namespace skgpu {

class TiledTextureUtils {
public:
    static bool ShouldTileImage(SkIRect conservativeClipBounds,
                                const SkISize& imageSize,
                                const SkMatrix& ctm,
                                const SkMatrix& srcToDst,
                                const SkRect* src,
                                int maxTileSize,
                                size_t cacheSize,
                                int* tileSize,
                                SkIRect* clippedSubset);

    static void DrawTiledBitmap(SkBaseDevice*,
                                const SkBitmap&,
                                int tileSize,
                                const SkMatrix& srcToDst,
                                const SkRect& srcRect,
                                const SkIRect& clippedSrcIRect,
                                const SkPaint& paint,
                                SkCanvas::QuadAAFlags origAAFlags,
                                const SkMatrix& localToDevice,
                                SkCanvas::SrcRectConstraint constraint,
                                SkSamplingOptions sampling,
                                SkTileMode tileMode);
};

} // namespace skgpu

#endif // skgpu_TiledTextureUtils_DEFINED
