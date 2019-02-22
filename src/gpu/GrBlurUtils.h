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
class GrRecordingContext;
class GrRenderTarget;
class GrRenderTargetContext;
class GrShape;
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
     * Draw a shape handling the mask filter if present.
     */
    void drawShapeWithMaskFilter(GrRecordingContext*,
                                 GrRenderTargetContext*,
                                 const GrClip&,
                                 const SkPaint&,
                                 const SkMatrix& viewMatrix,
                                 const GrShape&);

    /**
     * Draw a shape handling the mask filter. The mask filter is not optional.
     * The GrPaint will be modified after return.
     */
    void drawShapeWithMaskFilter(GrRecordingContext*,
                                 GrRenderTargetContext*,
                                 const GrClip&,
                                 const GrShape&,
                                 GrPaint&&,
                                 const SkMatrix& viewMatrix,
                                 const SkMaskFilter*);
};

#endif
