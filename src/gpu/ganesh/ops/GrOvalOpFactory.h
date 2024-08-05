/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrOvalOpFactory_DEFINED
#define GrOvalOpFactory_DEFINED

#include "include/core/SkTypes.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "include/core/SkScalar.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class GrStyle;
class SkMatrix;
class SkRRect;
class SkStrokeRec;
struct GrShaderCaps;
struct SkRect;

/*
 * This namespace wraps helper functions that draw ovals, rrects, and arcs (filled & stroked)
 */
class GrOvalOpFactory {
public:
    static GrOp::Owner MakeCircleOp(GrRecordingContext*,
                                    GrPaint&&,
                                    const SkMatrix&,
                                    const SkRect& oval,
                                    const GrStyle& style,
                                    const GrShaderCaps*);

    static GrOp::Owner MakeOvalOp(GrRecordingContext*,
                                  GrPaint&&,
                                  const SkMatrix&,
                                  const SkRect& oval,
                                  const GrStyle& style,
                                  const GrShaderCaps*);

    static GrOp::Owner MakeCircularRRectOp(GrRecordingContext*,
                                           GrPaint&&,
                                           const SkMatrix&,
                                           const SkRRect&,
                                           const SkStrokeRec&,
                                           const GrShaderCaps*);

    static GrOp::Owner MakeRRectOp(GrRecordingContext*,
                                   GrPaint&&,
                                   const SkMatrix&,
                                   const SkRRect&,
                                   const SkStrokeRec&,
                                   const GrShaderCaps*);

    static GrOp::Owner MakeArcOp(GrRecordingContext*,
                                 GrPaint&&,
                                 const SkMatrix&,
                                 const SkRect& oval,
                                 SkScalar startAngle,
                                 SkScalar sweepAngle,
                                 bool useCenter,
                                 const GrStyle&,
                                 const GrShaderCaps*);
};

#endif  // !defined(SK_ENABLE_OPTIMIZE_SIZE)

#endif  // GrOvalOpFactory_DEFINED
