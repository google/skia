/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrColor.h"

class GrDrawOp;
class GrShaderCaps;
class GrStyle;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer {
public:
    static GrDrawOp* CreateOvalBatch(GrColor,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& oval,
                                     const SkStrokeRec& stroke,
                                     const GrShaderCaps* shaderCaps);
    static GrDrawOp* CreateRRectBatch(GrColor,
                                      bool needsDistance,
                                      const SkMatrix& viewMatrix,
                                      const SkRRect& rrect,
                                      const SkStrokeRec& stroke,
                                      const GrShaderCaps* shaderCaps);

    static GrDrawOp* CreateArcBatch(GrColor,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& oval,
                                    SkScalar startAngle,
                                    SkScalar sweepAngle,
                                    bool useCenter,
                                    const GrStyle&,
                                    const GrShaderCaps* shaderCaps);
};

#endif // GrOvalRenderer_DEFINED
