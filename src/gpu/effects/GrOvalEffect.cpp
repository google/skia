/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalEffect.h"

#include "GrCircleEffect.h"
#include "GrEllipseEffect.h"
#include "SkRect.h"

sk_sp<GrFragmentProcessor> GrOvalEffect::Make(GrPrimitiveEdgeType edgeType, const SkRect& oval) {
    if (kHairlineAA_GrProcessorEdgeType == edgeType) {
        return nullptr;
    }
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return GrCircleEffect::Make(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return GrEllipseEffect::Make(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + h),
                                     SkPoint::Make(w, h));
    }

    return nullptr;
}
