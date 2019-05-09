/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(key) in GrClipEdgeType edgeType;
in float2 center;
in float radius;

float2 prevCenter;
float prevRadius = -1;
// The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
// fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
uniform float4 circle;

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType edgeType, SkPoint center,
                                                     float radius) {
        // A radius below half causes the implicit insetting done by this processor to become
        // inverted. We could handle this case by making the processor code more complicated.
        if (radius < .5f && GrProcessorEdgeTypeIsInverseFill(edgeType)) {
            return nullptr;
        }
        return std::unique_ptr<GrFragmentProcessor>(new GrCircleEffect(edgeType, center, radius));
    }
}

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@setData(pdman) {
    if (radius != prevRadius || center != prevCenter) {
        SkScalar effectiveRadius = radius;
        if (GrProcessorEdgeTypeIsInverseFill((GrClipEdgeType) edgeType)) {
            effectiveRadius -= 0.5f;
            // When the radius is 0.5 effectiveRadius is 0 which causes an inf * 0 in the shader.
            effectiveRadius = SkTMax(0.001f, effectiveRadius);
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
    // TODO: Right now the distance to circle calculation is performed in a space normalized to the
    // radius and then denormalized. This is to mitigate overflow on devices that don't have full
    // float.
    half d;
    @if (edgeType == GrClipEdgeType::kInverseFillBW ||
         edgeType == GrClipEdgeType::kInverseFillAA) {
        d = half((length((circle.xy - sk_FragCoord.xy) * circle.w) - 1.0) * circle.z);
    } else {
        d = half((1.0 - length((circle.xy - sk_FragCoord.xy) *  circle.w)) * circle.z);
    }
    @if (edgeType == GrClipEdgeType::kFillAA ||
         edgeType == GrClipEdgeType::kInverseFillAA ||
         edgeType == GrClipEdgeType::kHairlineAA) {
        sk_OutColor = sk_InColor * saturate(d);
    } else {
        sk_OutColor = d > 0.5 ? sk_InColor : half4(0);
    }
}

@test(testData) {
    SkPoint center;
    center.fX = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = testData->fRandom->nextRangeF(1.f, 1000.f);
    GrClipEdgeType et;
    do {
        et = (GrClipEdgeType) testData->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
    } while (GrClipEdgeType::kHairlineAA == et);
    return GrCircleEffect::Make(et, center, radius);
}