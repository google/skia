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

#include <tuple>

class GrClip;
class GrRecordingContext;
class SkBitmap;
struct SkIRect;
struct SkISize;
class SkMatrix;
class SkPaint;
struct SkRect;
struct SkSamplingOptions;

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

    enum class ImageDrawMode {
        // Src and dst have been restricted to the image content. May need to clamp, no need to
        // decal.
        kOptimized,
        // Src and dst are their original sizes, requires use of a decal instead of plain clamping.
        // This is used when a dst clip is provided and extends outside of the optimized dst rect.
        kDecal,
        // Src or dst are empty, or do not intersect the image content so don't draw anything.
        kSkip
    };

    static ImageDrawMode OptimizeSampleArea(const SkISize& imageSize,
                                            const SkRect& origSrcRect,
                                            const SkRect& origDstRect,
                                            const SkPoint dstClip[4],
                                            SkRect* outSrcRect,
                                            SkRect* outDstRect,
                                            SkMatrix* outSrcToDst);

    static bool CanDisableMipmap(const SkMatrix& viewM, const SkMatrix& localM);

    static void ClampedOutsetWithOffset(SkIRect* iRect, int outset, SkPoint* offset,
                                        const SkIRect& clamp);

    static std::tuple<bool, size_t> DrawAsTiledImageRect(SkCanvas*,
                                                         const SkImage*,
                                                         const SkRect& srcRect,
                                                         const SkRect& dstRect,
                                                         SkCanvas::QuadAAFlags,
                                                         const SkSamplingOptions&,
                                                         const SkPaint*,
                                                         SkCanvas::SrcRectConstraint,
                                                         size_t cacheSize,
                                                         size_t maxTextureSize);
};

} // namespace skgpu

#endif // skgpu_TiledTextureUtils_DEFINED
