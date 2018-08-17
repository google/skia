/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlurUtils_DEFINED
#define GrBlurUtils_DEFINED

#include "GrTypesPriv.h"

class GrClip;
class GrContext;
class GrPaint;
class GrRenderTarget;
class GrRenderTargetContext;
class GrStyle;
struct SkIRect;
class SkMaskFilter;
class SkMatrix;
class SkPaint;
class SkPath;
class SkPathEffect;


/**
 *  Blur utilities.
 */
namespace GrBlurUtils {
    /**
     * Draw a path handling the mask filter if present.
     */
    void drawPathWithMaskFilter(GrContext*,
                                GrRenderTargetContext*,
                                const GrClip&,
                                const SkPath& origSrcPath,
                                const SkPaint&,
                                const SkMatrix& viewMatrix,
                                bool pathIsMutable);

    /**
     * Draw a path handling the mask filter. The mask filter is not optional.
     * The GrPaint will be modified after return.
     */
    void drawPathWithMaskFilter(GrContext*,
                                GrRenderTargetContext*,
                                const GrClip&,
                                const SkPath& path,
                                GrPaint&&,
                                GrAA,
                                const SkMatrix& viewMatrix,
                                const SkMaskFilter*,
                                const GrStyle&,
                                bool pathIsMutable);
};

#endif
