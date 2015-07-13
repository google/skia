/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalRenderer_DEFINED
#define GrOvalRenderer_DEFINED

#include "GrPaint.h"

class GrDrawTarget;
class GrPipelineBuilder;
struct SkRect;
class SkStrokeRec;

/*
 * This class wraps helper functions that draw ovals and roundrects (filled & stroked)
 */
class GrOvalRenderer {
public:
    static bool DrawOval(GrDrawTarget*,
                         const GrPipelineBuilder&,
                         GrColor,
                         const SkMatrix& viewMatrix,
                         bool useAA,
                         const SkRect& oval,
                         const SkStrokeRec& stroke);
    static bool DrawRRect(GrDrawTarget*,
                          const GrPipelineBuilder&,
                          GrColor,
                          const SkMatrix& viewMatrix,
                          bool useAA,
                          const SkRRect& rrect,
                          const SkStrokeRec& stroke);
    static bool DrawDRRect(GrDrawTarget* target,
                           const GrPipelineBuilder&,
                           GrColor,
                           const SkMatrix& viewMatrix,
                           bool useAA,
                           const SkRRect& outer,
                           const SkRRect& inner);

private:
    GrOvalRenderer();

    static bool DrawEllipse(GrDrawTarget* target,
                            const GrPipelineBuilder&,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            bool useCoverageAA,
                            const SkRect& ellipse,
                            const SkStrokeRec& stroke);
    static bool DrawDIEllipse(GrDrawTarget* target,
                              const GrPipelineBuilder&,
                              GrColor,
                              const SkMatrix& viewMatrix,
                              bool useCoverageAA,
                              const SkRect& ellipse,
                              const SkStrokeRec& stroke);
    static void DrawCircle(GrDrawTarget* target,
                           const GrPipelineBuilder&,
                           GrColor,
                           const SkMatrix& viewMatrix,
                           bool useCoverageAA,
                           const SkRect& circle,
                           const SkStrokeRec& stroke);
};

#endif // GrOvalRenderer_DEFINED
