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

bool ShouldTileImage(GrRecordingContext* context,
                     SkIRect conservativeClipBounds,
                     uint32_t /* imageID */,
                     const SkISize& imageSize,
                     const SkMatrix& ctm,
                     const SkMatrix& srcToDst,
                     const SkRect* src,
                     int maxTileSize,
                     int* tileSize,
                     SkIRect* clippedSubset);

typedef void (*DrawImageProc)(GrRecordingContext* rContext,
                              skgpu::ganesh::SurfaceDrawContext* sdc,
                              const GrClip* clip,
                              const SkMatrixProvider& matrixProvider,
                              const SkPaint& paint,
                              const SkImage* image,
                              const SkRect& src,
                              const SkRect& dst,
                               const SkPoint dstClip[4],
                              const SkMatrix& srcToDst,
                              SkCanvas::QuadAAFlags aaFlags,
                              SkCanvas::SrcRectConstraint constraint,
                              SkSamplingOptions sampling,
                              SkTileMode tm);

void DrawTiledBitmap(GrRecordingContext* rContext,
                     skgpu::ganesh::SurfaceDrawContext* sdc,
                     const GrClip* clip,
                     const SkBitmap& bitmap,
                     int tileSize,
                     const SkMatrixProvider& matrixProvider,
                     const SkMatrix& srcToDst,
                     const SkRect& srcRect,
                     const SkIRect& clippedSrcIRect,
                     const SkPaint& paint,
                     SkCanvas::QuadAAFlags origAAFlags,
                     SkCanvas::SrcRectConstraint constraint,
                     SkSamplingOptions sampling,
                     SkTileMode tileMode,
                     DrawImageProc drawImageFn);

} // namespace skgpu

#endif // skgpu_TiledTextureUtils_DEFINED
