
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDashingEffect_DEFINED
#define GrDashingEffect_DEFINED

#include "GrTypesPriv.h"
#include "SkPathEffect.h"

class GrGpu;
class GrDrawTarget;
class GrPaint;
class GrStrokeInfo;

class GrGLDashingEffect;
class SkPath;

namespace GrDashingEffect {
    bool DrawDashLine(const SkPoint pts[2], const GrPaint& paint, const GrStrokeInfo& strokeInfo,
                      GrGpu* gpu, GrDrawTarget* target, const SkMatrix& vm);

    enum DashCap {
        kRound_DashCap,
        kNonRound_DashCap,
    };

    /**
     * An effect that renders a dashed line. It is intended to be used as a coverage effect.
     * The effect is meant for dashed lines that only have a single on/off interval pair.
     * Bounding geometry is rendered and the effect computes coverage based on the fragment's
     * position relative to the dashed line.
     */
    GrEffect* Create(GrEffectEdgeType edgeType, const SkPathEffect::DashInfo& info,
                     SkScalar strokeWidth, DashCap cap);
}

#endif
