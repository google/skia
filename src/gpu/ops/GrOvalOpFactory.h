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

class GrDrawOp;
class GrPaint;
class GrShaderCaps;
class GrStyle;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;

/*
 * This namespace wraps helper functions that draw ovals, rrects, and arcs (filled & stroked)
 * The ops always use coverage even when their non-AA.
 */
class GrOvalOpFactory {
public:
    static std::unique_ptr<GrDrawOp> MakeCoverageOvalOp(GrPaint&&,
                                                        GrAA,
                                                        const SkMatrix&,
                                                        const SkRect& oval,
                                                        const SkStrokeRec&,
                                                        const GrShaderCaps*);

    static std::unique_ptr<GrDrawOp> MakeCoverageRRectOp(GrPaint&&,
                                                         GrAA,
                                                         const SkMatrix&,
                                                         const SkRRect&,
                                                         const SkStrokeRec&,
                                                         const GrShaderCaps*);

    static std::unique_ptr<GrDrawOp> MakeCoverageArcOp(GrPaint&&,
                                                       GrAA,
                                                       const SkMatrix&,
                                                       const SkRect& oval,
                                                       SkScalar startAngle,
                                                       SkScalar sweepAngle,
                                                       bool useCenter,
                                                       const GrStyle&,
                                                       const GrShaderCaps*);
};

#endif  // GrOvalOpFactory_DEFINED
