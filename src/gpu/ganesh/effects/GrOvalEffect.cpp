/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrOvalEffect.h"

#include <utility>

enum class GrClipEdgeType;

GrFPResult GrOvalEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP, GrClipEdgeType edgeType,
                              const SkRect& oval, const GrShaderCaps& caps) {
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return GrFragmentProcessor::Circle(std::move(inputFP), edgeType,
                                           SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return GrFragmentProcessor::Ellipse(std::move(inputFP), edgeType,
                                            SkPoint::Make(oval.fLeft + w, oval.fTop + h),
                                            SkPoint::Make(w, h), caps);
    }
    SkUNREACHABLE;
}
