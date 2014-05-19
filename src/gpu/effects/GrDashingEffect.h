
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

class GrContext;

class GrGLDashingEffect;
class SkPath;

namespace GrDashingEffect {
    bool DrawDashLine(const SkPoint pnts[2], const SkPaint& paint, GrContext* context);

    /**
     * An effect that renders a dashed line. It is intended to be used as a coverage effect.
     * The effect is meant for dashed lines that only have a single on/off interval pair.
     * Bounding geometry is rendered and the effect computes coverage based on the fragment's
     * position relative to the dashed line.
     */
    GrEffectRef* Create(GrEffectEdgeType edgeType, const SkPathEffect::DashInfo& info,
                        const SkMatrix& matrix, SkScalar strokeWidth);
}

#endif
