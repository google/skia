/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlurUtils_DEFINED
#define GrBlurUtils_DEFINED

class GrClip;
class GrPaint;
class GrRecordingContext;
class GrStyledShape;
class SkMatrixProvider;
class SkMaskFilter;
class SkMatrix;
class SkPaint;
namespace skgpu {
namespace ganesh {
class SurfaceDrawContext;
}
}  // namespace skgpu

/**
 *  Blur utilities.
 */
namespace GrBlurUtils {
/**
 * Draw a shape handling the mask filter if present.
 */
void drawShapeWithMaskFilter(GrRecordingContext*,
                             skgpu::ganesh::SurfaceDrawContext*,
                             const GrClip*,
                             const SkPaint&,
                             const SkMatrixProvider&,
                             const GrStyledShape&);

/**
 * Draw a shape handling the mask filter. The mask filter is not optional.
 * The GrPaint will be modified after return.
 */
void drawShapeWithMaskFilter(GrRecordingContext*,
                             skgpu::ganesh::SurfaceDrawContext*,
                             const GrClip*,
                             const GrStyledShape&,
                             GrPaint&&,
                             const SkMatrix& viewMatrix,
                             const SkMaskFilter*);
}  // namespace GrBlurUtils

#endif
