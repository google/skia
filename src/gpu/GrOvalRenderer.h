/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrColor.h"

class GrDrawBatch;
class GrShaderCaps;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer {
public:
    static GrDrawBatch* CreateOvalBatch(GrColor,
                                        const SkMatrix& viewMatrix,
                                        const SkRect& oval,
                                        const SkStrokeRec& stroke,
                                        GrShaderCaps* shaderCaps);
    static GrDrawBatch* CreateRRectBatch(GrColor,
                                         const SkMatrix& viewMatrix,
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
