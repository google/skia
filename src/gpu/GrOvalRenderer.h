/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrPaint.h"

class GrDrawBatch;
class GrPipelineBuilder;
class GrShaderCaps;
struct SkRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer {
public:
    static GrDrawBatch* CreateOvalBatch(const GrPipelineBuilder&,
                                        GrColor,
                                        const SkMatrix& viewMatrix,
                                        bool useAA,
                                        const SkRect& oval,
                                        const SkStrokeRec& stroke,
                                        GrShaderCaps* shaderCaps);
    static GrDrawBatch* CreateRRectBatch(const GrPipelineBuilder&,
                                         GrColor,
                                         const SkMatrix& viewMatrix,
                                         bool useAA,
                                         const SkRRect& rrect,
                                         const SkStrokeRec& stroke,
                                         GrShaderCaps* shaderCaps);

private:
    GrOvalRenderer();

    static GrDrawBatch* CreateEllipseBatch(GrColor,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& ellipse,
                                           const SkStrokeRec& stroke);
    static GrDrawBatch* CreateDIEllipseBatch(GrColor,
                                             const SkMatrix& viewMatrix,
                                             const SkRect& ellipse,
                                             const SkStrokeRec& stroke);
    static GrDrawBatch* CreateCircleBatch(GrColor,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& circle,
                                          const SkStrokeRec& stroke);
};

#endif // GrOvalRenderer_DEFINED
