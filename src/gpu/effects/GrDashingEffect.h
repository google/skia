
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
class GrGpu;
class GrPaint;
class GrPipelineBuilder;
class GrStrokeInfo;

namespace GrDashingEffect {
    bool DrawDashLine(GrGpu*, GrDrawTarget*, GrPipelineBuilder*, GrColor,
                      const SkMatrix& viewMatrix, const SkPoint pts[2], const GrPaint& paint,
                      const GrStrokeInfo& strokeInfo);
}

#endif
