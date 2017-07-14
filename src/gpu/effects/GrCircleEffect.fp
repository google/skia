/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(key) in int edgeType;
in vec2 center;
in float radius;

vec2 prevCenter;
float prevRadius = -1;
// The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
// fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
uniform vec4 circle;

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@setData(pdman) {
    if (radius != prevRadius || center != prevCenter) {
        SkScalar effectiveRadius = radius;
        if (GrProcessorEdgeTypeIsInverseFill((GrPrimitiveEdgeType) edgeType)) {
            effectiveRadius -= 0.5f;
        } else {
            effectiveRadius += 0.5f;
        }
        pdman.set4f(circle, center.fX, center.fY, effectiveRadius,
                    SkScalarInvert(effectiveRadius));
        prevCenter = center;
        prevRadius = radius;
    }
}

void main() {
    // TODO: Right now the distance to circle caclulation is performed in a space normalized to the
    // radius and then denormalized. This is to prevent overflow on devices that have a "real"
    // mediump. It'd be nice to only do this on mediump devices.
    float d;
    @if (edgeType == 2 /* kInverseFillBW_GrProcessorEdgeType */ ||
         edgeType == 3 /* kInverseFillAA_GrProcessorEdgeType */) {
        d = (length((circle.xy - sk_FragCoord.xy) * circle.w) - 1.0) * circle.z;
    } else {
        d = (1.0 - length((circle.xy - sk_FragCoord.xy) *  circle.w)) * circle.z;
    }
    @if (edgeType == 1 /* kFillAA_GrProcessorEdgeType */ ||
         edgeType == 3 /* kInverseFillAA_GrProcessorEdgeType */ ||
         edgeType == 4 /* kHairlineAA_GrProcessorEdgeType */) {
        d = clamp(d, 0.0, 1.0);
    } else {
        d = d > 0.5 ? 1.0 : 0.0;
    }

    sk_OutColor = sk_InColor * d;
}

@test(testData) {
    SkPoint center;
    center.fX = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = testData->fRandom->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType) testData->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return GrCircleEffect::Make(et, center, radius);
}