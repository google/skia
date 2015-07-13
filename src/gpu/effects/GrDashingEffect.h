
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDashingEffect_DEFINED
#define GrDashingEffect_DEFINED

#include "GrColor.h"
#include "GrTypesPriv.h"
#include "SkPathEffect.h"

class GrClip;
class GrDrawTarget;
class GrPaint;
class GrPipelineBuilder;
class GrStrokeInfo;

namespace GrDashingEffect {
    bool DrawDashLine(GrDrawTarget*, const GrPipelineBuilder&, GrColor,
                      const SkMatrix& viewMatrix, const SkPoint pts[2], bool useAA,
                      const GrStrokeInfo& strokeInfo);
    bool CanDrawDashLine(const SkPoint pts[2], const GrStrokeInfo& strokeInfo,
                         const SkMatrix& viewMatrix);
}

#endif
