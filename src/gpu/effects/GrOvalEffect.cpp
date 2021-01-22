/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrOvalEffect.h"

#include "include/core/SkRect.h"
#include "src/gpu/effects/generated/GrCircleEffect.h"
#include "src/gpu/effects/generated/GrEllipseEffect.h"

GrFPResult GrOvalEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP, GrClipEdgeType edgeType,
                              const SkRect& oval, const GrShaderCaps& caps) {
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return GrCircleEffect::Make(std::move(inputFP), edgeType,
                                    SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return GrEllipseEffect::Make(std::move(inputFP), edgeType,
                                     SkPoint::Make(oval.fLeft + w, oval.fTop + h),
                                     SkPoint::Make(w, h), caps);
    }
    SkUNREACHABLE;
}
