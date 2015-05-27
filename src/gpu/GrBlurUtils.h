/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlurUtils_DEFINED
#define GrBlurUtils_DEFINED

class GrClip;
class GrContext;
class GrDrawContext;
class GrRenderTarget;
struct SkIRect;
class SkMatrix;
class SkPaint;
class SkPath;


/**
 *  Blur utilities.
 */
namespace GrBlurUtils {
    /**
     * Draw a path handling the mask filter if present.
     */
    void drawPathWithMaskFilter(GrContext* context,
                                GrDrawContext* drawContext,
                                GrRenderTarget* rt,
                                const GrClip& clip,
                                const SkPath& origSrcPath,
                                const SkPaint& paint,
                                const SkMatrix& origViewMatrix,
                                const SkMatrix* prePathMatrix,
                                const SkIRect& clipBounds,
                                bool pathIsMutable);
};

#endif
