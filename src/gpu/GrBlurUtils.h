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
class GrShape;
class GrStyle;
class GrUniqueKey;
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
    void drawPathWithMaskFilter1(GrContext*,
                                GrRenderTargetContext*,
                                const GrClip&,
                                const SkPath& origSrcPath,
                                const SkPaint&,
                                const SkMatrix& origViewMatrix,
                                const SkMatrix* prePathMatrix,
                                const SkIRect& clipBounds,
                                bool pathIsMutable,
                                const GrUniqueKey&);

    /**
     * Draw a path handling the mask filter. The mask filter is not optional. The path effect is
     * optional. The GrPaint will be modified after return.
     */
    void drawPathWithMaskFilter2(GrContext*,
                                GrRenderTargetContext*,
                                const GrClip&,
                                const SkPath& path,
                                GrPaint&&,
                                GrAA,
                                const SkMatrix& viewMatrix,
                                const SkMaskFilter*,
                                const GrStyle&,
                                bool pathIsMutable,
                                const GrUniqueKey&);

    void drawShapeWithMaskFilter3(GrContext* context,
                                 GrRenderTargetContext* renderTargetContext,
                                 const GrClip& clip,
                                 const SkPaint& paint,
                                 const SkMatrix& viewMatrix,
                                 const GrShape& shape);
};

#endif
