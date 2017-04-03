/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalOpFactory_DEFINED
#define GrOvalOpFactory_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrLegacyMeshDrawOp;
class GrShaderCaps;
class GrStyle;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;

/*
 * This namespace wraps helper functions that draw ovals, rrects, and arcs (filled & stroked)
 */
class GrOvalOpFactory {
public:
    static std::unique_ptr<GrLegacyMeshDrawOp> MakeOvalOp(GrColor,
                                                          const SkMatrix& viewMatrix,
                                                          const SkRect& oval,
                                                          const SkStrokeRec& stroke,
                                                          const GrShaderCaps* shaderCaps);
    static std::unique_ptr<GrLegacyMeshDrawOp> MakeRRectOp(GrColor,
                                                           bool needsDistance,
                                                           const SkMatrix& viewMatrix,
                                                           const SkRRect& rrect,
                                                           const SkStrokeRec& stroke,
                                                           const GrShaderCaps* shaderCaps);

    static std::unique_ptr<GrLegacyMeshDrawOp> MakeArcOp(GrColor,
                                                         const SkMatrix& viewMatrix,
                                                         const SkRect& oval,
                                                         SkScalar startAngle,
                                                         SkScalar sweepAngle,
                                                         bool useCenter,
                                                         const GrStyle&,
                                                         const GrShaderCaps* shaderCaps);
};

#endif  // GrOvalOpFactory_DEFINED
