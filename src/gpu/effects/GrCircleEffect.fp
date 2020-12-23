/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
layout(key) in GrClipEdgeType edgeType;
in float2 center;
in float radius;

float2 prevCenter;
float prevRadius = -1;
// The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
// fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
uniform float4 circle;

@make {
    static GrFPResult Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                           GrClipEdgeType edgeType, SkPoint center, float radius) {
        // A radius below half causes the implicit insetting done by this processor to become
        // inverted. We could handle this case by making the processor code more complicated.
        if (radius < .5f && GrProcessorEdgeTypeIsInverseFill(edgeType)) {
            return GrFPFailure(std::move(inputFP));
        }
        return GrFPSuccess(std::unique_ptr<GrFragmentProcessor>(
                    new GrCircleEffect(std::move(inputFP), edgeType, center, radius)));
    }
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@setData(pdman) {
    if (radius != prevRadius || center != prevCenter) {
        SkScalar effectiveRadius = radius;
        if (GrProcessorEdgeTypeIsInverseFill((GrClipEdgeType) edgeType)) {
            effectiveRadius -= 0.5f;
            // When the radius is 0.5 effectiveRadius is 0 which causes an inf * 0 in the shader.
            effectiveRadius = std::max(0.001f, effectiveRadius);
        } else {
            effectiveRadius += 0.5f;
        }
        pdman.set4f(circle, center.fX, center.fY, effectiveRadius,
                    SkScalarInvert(effectiveRadius));
        prevCenter = center;
        prevRadius = radius;
    }
}

half4 main() {
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
    half4 inputColor = sample(inputFP);
    @if (edgeType == GrClipEdgeType::kFillAA ||
         edgeType == GrClipEdgeType::kInverseFillAA) {
        return inputColor * saturate(d);
    } else {
        return d > 0.5 ? inputColor : half4(0);
    }
}

@test(testData) {
    SkPoint center;
    center.fX = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = testData->fRandom->nextRangeF(1.f, 1000.f);
    bool success;
    std::unique_ptr<GrFragmentProcessor> fp = testData->inputFP();
    do {
        GrClipEdgeType et = (GrClipEdgeType)testData->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
        std::tie(success, fp) = GrCircleEffect::Make(std::move(fp), et, center, radius);
    } while (!success);
    return fp;
}
