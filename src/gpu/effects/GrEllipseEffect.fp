/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(key) in int edgeType;
in vec2 center;
in vec2 radii;

vec2 prevCenter;
vec2 prevRadii = vec2(-1);
// The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
// The last two terms can underflow on mediump, so we use highp.
uniform highp vec4 ellipse;

bool useScale = sk_Caps.floatPrecisionVaries;
layout(when=useScale) uniform vec2 scale;

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@setData(pdman) {
    if (radii != prevRadii || center != prevCenter) {
        float invRXSqd;
        float invRYSqd;
        // If we're using a scale factor to work around precision issues, choose the larger radius
        // as the scale factor. The inv radii need to be pre-adjusted by the scale factor.
        if (scale.isValid()) {
            if (radii.fX > radii.fY) {
                invRXSqd = 1.f;
                invRYSqd = (radii.fX * radii.fX) /
                           (radii.fY * radii.fY);
                pdman.set2f(scale, radii.fX, 1.f / radii.fX);
            } else {
                invRXSqd = (radii.fY * radii.fY) /
                           (radii.fX * radii.fX);
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
    vec2 d = sk_FragCoord.xy - ellipse.xy;
    // If we're on a device with a "real" mediump then we'll do the distance computation in a space
    // that is normalized by the larger radius. The scale uniform will be scale, 1/scale. The
    // inverse squared radii uniform values are already in this normalized space. The center is
    // not.
    @if (useScale) {
        d *= scale.y;
    }
    vec2 Z = d * ellipse.zw;
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    float implicit = dot(Z, d) - 1;
    // grad_dot is the squared length of the gradient of the implicit.
    float grad_dot = 4 * dot(Z, Z);
    // Avoid calling inversesqrt on zero.
    grad_dot = max(grad_dot, 1e-4);
    float approx_dist = implicit * inversesqrt(grad_dot);
    @if (useScale) {
        approx_dist *= scale.x;
    }

    float alpha;
    @switch (edgeType) {
        case 0 /* kFillBW_GrProcessorEdgeType */:
            alpha = approx_dist > 0.0 ? 0.0 : 1.0;
            break;
        case 1 /* kFillAA_GrProcessorEdgeType */:
            alpha = clamp(0.5 - approx_dist, 0.0, 1.0);
            break;
        case 2 /* kInverseFillBW_GrProcessorEdgeType */:
            alpha = approx_dist > 0.0 ? 1.0 : 0.0;
            break;
        case 3 /* kInverseFillAA_GrProcessorEdgeType */:
            alpha = clamp(0.5 + approx_dist, 0.0, 1.0);
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
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType) testData->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return GrEllipseEffect::Make(et, center, SkPoint::Make(rx, ry));
}