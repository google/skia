/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
    #include "src/gpu/GrShaderCaps.h"
}

layout(key) in GrClipEdgeType edgeType;
in float2 center;
in float2 radii;

float2 prevCenter;
float2 prevRadii = float2(-1);
// The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
// The last two terms can underflow when float != fp32, so we also provide a workaround.
uniform float4 ellipse;

bool medPrecision = !sk_Caps.floatIs32Bits;
layout(when=medPrecision) uniform float2 scale;

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType edgeType, SkPoint center,
                                                     SkPoint radii, const GrShaderCaps& caps) {
        // Small radii produce bad results on devices without full float.
        if (!caps.floatIs32Bits() && (radii.fX < 0.5f || radii.fY < 0.5f)) {
            return nullptr;
        }
        // Very narrow ellipses produce bad results on devices without full float
        if (!caps.floatIs32Bits() && (radii.fX > 255*radii.fY || radii.fY > 255*radii.fX)) {
            return nullptr;
        }
        // Very large ellipses produce bad results on devices without full float
        if (!caps.floatIs32Bits() && (radii.fX > 16384 || radii.fY > 16384)) {
            return nullptr;
        }
        return std::unique_ptr<GrFragmentProcessor>(new GrEllipseEffect(edgeType, center, radii));
    }
}

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@setData(pdman) {
    if (radii != prevRadii || center != prevCenter) {
        float invRXSqd;
        float invRYSqd;
        // If we're using a scale factor to work around precision issues, choose the larger
        // radius as the scale factor. The inv radii need to be pre-adjusted by the scale
        // factor.
        if (scale.isValid()) {
            if (radii.fX > radii.fY) {
                invRXSqd = 1.f;
                invRYSqd = (radii.fX * radii.fX) / (radii.fY * radii.fY);
                pdman.set2f(scale, radii.fX, 1.f / radii.fX);
            } else {
                invRXSqd = (radii.fY * radii.fY) / (radii.fX * radii.fX);
                invRYSqd = 1.f;
                pdman.set2f(scale, radii.fY, 1.f / radii.fY);
            }
        } else {
            invRXSqd = 1.f / (radii.fX * radii.fX);
            invRYSqd = 1.f / (radii.fY * radii.fY);
        }
        pdman.set4f(ellipse, center.fX, center.fY, invRXSqd, invRYSqd);
        prevCenter = center;
        prevRadii = radii;
    }
}

void main() {
    // d is the offset to the ellipse center
    float2 d = sk_FragCoord.xy - ellipse.xy;
    // If we're on a device with a "real" mediump then we'll do the distance computation in a space
    // that is normalized by the larger radius or 128, whichever is smaller. The scale uniform will
    // be scale, 1/scale. The inverse squared radii uniform values are already in this normalized space.
    // The center is not.
    @if (medPrecision) {
        d *= scale.y;
    }
    float2 Z = d * ellipse.zw;
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    float implicit = dot(Z, d) - 1;
    // grad_dot is the squared length of the gradient of the implicit.
    float grad_dot = 4 * dot(Z, Z);
    // Avoid calling inversesqrt on zero.
    @if (medPrecision) {
        grad_dot = max(grad_dot, 6.1036e-5);
    } else {
        grad_dot = max(grad_dot, 1.1755e-38);
    }
    float approx_dist = implicit * inversesqrt(grad_dot);
    @if (medPrecision) {
        approx_dist *= scale.x;
    }

    half alpha;
    @switch (edgeType) {
        case GrClipEdgeType::kFillBW:
            alpha = approx_dist > 0.0 ? 0.0 : 1.0;
            break;
        case GrClipEdgeType::kFillAA:
            alpha = saturate(0.5 - half(approx_dist));
            break;
        case GrClipEdgeType::kInverseFillBW:
            alpha = approx_dist > 0.0 ? 1.0 : 0.0;
            break;
        case GrClipEdgeType::kInverseFillAA:
            alpha = saturate(0.5 + half(approx_dist));
            break;
        default:
            // hairline not supported
            discard;
    }
    sk_OutColor = sk_InColor * alpha;
}

@test(testData) {
    SkPoint center;
    center.fX = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = testData->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = testData->fRandom->nextRangeF(0.f, 1000.f);
    SkScalar ry = testData->fRandom->nextRangeF(0.f, 1000.f);
    GrClipEdgeType et;
    do {
        et = (GrClipEdgeType) testData->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
    } while (GrClipEdgeType::kHairlineAA == et);
    return GrEllipseEffect::Make(et, center, SkPoint::Make(rx, ry),
                                 *testData->caps()->shaderCaps());
}
