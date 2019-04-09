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
class GrRecordingContext;
class GrShaderCaps;
class GrStyle;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;
enum class GrAAType : unsigned int;

/*
 * This namespace wraps helper functions that draw ovals, rrects, and arcs (filled & stroked)
 */
class GrOvalOpFactory {
public:
    static std::unique_ptr<GrDrawOp> MakeOvalOp(GrRecordingContext*,
                                                GrPaint&&,
                                                GrAAType aaType,
                                                const SkMatrix&,
                                                const SkRect& oval,
                                                const GrStyle& style,
                                                const GrShaderCaps*);

    static std::unique_ptr<GrDrawOp> MakeRRectOp(GrRecordingContext*,
                                                 GrPaint&&,
                                                 GrAAType aaType,
                                                 const SkMatrix&,
                                                 const SkRRect&,
                                                 const SkStrokeRec&,
                                                 const GrShaderCaps*);

    static std::unique_ptr<GrDrawOp> MakeArcOp(GrRecordingContext*,
                                               GrPaint&&,
                                               GrAAType aaType,
                                               const SkMatrix&,
                                               const SkRect& oval,
                                               SkScalar startAngle,
                                               SkScalar sweepAngle,
                                               bool useCenter,
                                               const GrStyle&,
                                               const GrShaderCaps*);
};

#endif  // GrOvalOpFactory_DEFINED
