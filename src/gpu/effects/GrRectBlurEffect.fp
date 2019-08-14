/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
    #include "include/core/SkScalar.h"
    #include "src/core/SkBlurMask.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrShaderCaps.h"
}

in float4 rect;

layout(key) bool highp = abs(rect.x) > 16000 || abs(rect.y) > 16000 ||
                         abs(rect.z) > 16000 || abs(rect.w) > 16000;

layout(when= highp) uniform float4 rectF;
layout(when=!highp) uniform half4  rectH;

in uniform half sigma;

@make {
     static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider* proxyProvider,
                                                      const GrShaderCaps& caps,
                                                      const SkRect& rect, float sigma) {
         float doubleProfileSize = (12 * sigma);
         if (!caps.floatIs32Bits()) {
             // We promote the math that gets us into the Gaussian space to full float when the rect
             // coords are large. If we don't have full float then fail. We could probably clip the
             // rect to an outset device bounds instead.
             if (SkScalarAbs(rect.fLeft)  > 16000.f || SkScalarAbs(rect.fTop)    > 16000.f ||
                 SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f) {
                    return nullptr;
             }
         }
         // Sigma is always a half.
         SkASSERT(sigma > 0);
         if (sigma > 16000.f) {
             return nullptr;
         }

         if (doubleProfileSize >= (float) rect.width() ||
             doubleProfileSize >= (float) rect.height()) {
             // if the blur sigma is too large so the gaussian overlaps the whole
             // rect in either direction, fall back to CPU path for now.
             return nullptr;
         }

         return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(rect, sigma));
     }
}

void main() {
    half invr = 1.0 / (2.0 * sigma);

    // Get the smaller of the signed distance from the frag coord to the left and right edges.
    half x;
    @if (highp) {
        float lDiff = rectF.x - sk_FragCoord.x;
        float rDiff = sk_FragCoord.x - rectF.z;
        x = half(max(lDiff, rDiff) * invr);
    } else {
        half lDiff = half(rectH.x - sk_FragCoord.x);
        half rDiff = half(sk_FragCoord.x - rectH.z);
        x = max(lDiff, rDiff) * invr;
    }
    // This is lifted from the implementation of SkBlurMask::ComputeBlurProfile. It approximates
    // a Gaussian as three box filters, and then computes the integral of this approximation from
    // -inf to x.
    // TODO: Make this a function when supported in .fp files as we duplicate it for y below.
    half xCoverage;
    if (x > 1.5) {
        xCoverage = 0.0;
    } else if (x < -1.5) {
        xCoverage = 1.0;
    } else {
        half x2 = x * x;
        half x3 = x2 * x;

        if (x > 0.5) {
            xCoverage = 0.5625 - (x3 / 6.0 - 3.0 * x2 * 0.25 + 1.125 * x);
        } else if (x > -0.5) {
            xCoverage = 0.5 - (0.75 * x - x3 / 3.0);
        } else {
            xCoverage = 0.4375 + (-x3 / 6.0 - 3.0 * x2 * 0.25 - 1.125 * x);
        }
    }

    // Repeat of above for y.
    half y;
    @if (highp) {
        float tDiff = rectF.y - sk_FragCoord.y;
        float bDiff = sk_FragCoord.y - rectF.w;
        y = half(max(tDiff, bDiff) * invr);
    } else {
        half tDiff = half(rectH.y - sk_FragCoord.y);
        half bDiff = half(sk_FragCoord.y - rectH.w);
        y = max(tDiff, bDiff) * invr;
    }
    half yCoverage;
    if (y > 1.5) {
        yCoverage = 0.0;
    } else if (y < -1.5) {
        yCoverage = 1.0;
    } else {
        half y2 = y * y;
        half y3 = y2 * y;

        if (y > 0.5) {
            yCoverage = 0.5625 - (y3 / 6.0 - 3.0 * y2 * 0.25 + 1.125 * y);
        } else if (y > -0.5) {
            yCoverage = 0.5 - (0.75 * y - y3 / 3.0);
        } else {
            yCoverage = 0.4375 + (-y3 / 6.0 - 3.0 * y2 * 0.25 - 1.125 * y);
        }
    }

    sk_OutColor = sk_InColor * xCoverage * yCoverage;
}

@setData(pdman) {
    float r[] {rect.fLeft, rect.fTop, rect.fRight, rect.fBottom};
    pdman.set4fv(highp ? rectF : rectH, 1, r);
}

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

@test(data) {
    float sigma = data->fRandom->nextRangeF(3,8);
    float width = data->fRandom->nextRangeF(200,300);
    float height = data->fRandom->nextRangeF(200,300);
    return GrRectBlurEffect::Make(data->proxyProvider(), *data->caps()->shaderCaps(),
                                  SkRect::MakeWH(width, height), sigma);
}
