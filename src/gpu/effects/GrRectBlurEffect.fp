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
        // Get the smaller of the signed distance from the frag coord to the left and right edges
        // and similar for y.
        half x;
        @if (highp) {
            x = min(half(sk_FragCoord.x - rectF.x), half(rectF.z - sk_FragCoord.x));
        } else {
            x = min(half(sk_FragCoord.x - rectH.x), half(rectH.z - sk_FragCoord.x));
        }
        half y;
        @if (highp) {
            y = min(half(sk_FragCoord.y - rectF.y), half(rectF.w - sk_FragCoord.y));
        } else {
            y = min(half(sk_FragCoord.y - rectH.y), half(rectH.w - sk_FragCoord.y));
        }
        // The sw code computes an approximation of an integral of the Gaussian from -inf to x,
        // where x is the signed distance to the edge (positive inside the rect). The approximation
        // is based on three box filters and is a piecewise cubic. The piecewise nature introduces
        // branches so here we use a 5th degree very close approximation of the piecewise cubic. The
        // piecewise cubic goes from 0 to 1 as x goes from -1.5 to 1.5.
        half r = 1 / (2.0 * sigma);
        x *= r;
        y *= r;
        // The polynomial is such that we can either clamp the domain or the range. Clamping the
        // range (xCoverage/yCoverage) seems to be faster but the polynomial quickly produces very
        // large absolute values outside the [-1.5, 1.5] domain and some mobile GPUs don't seem to
        // properly produce -infs or infs in that case. So instead we clamp the domain (x/y). The
        // perf is probably because clamping to [0, 1] is faster than clamping to [-1.5, 1.5].
        x = clamp(x, -1.5, 1.5);
        y = clamp(y, -1.5, 1.5);
        half x2 = x * x;
        half x3 = x2 * x;
        half x5 = x2 * x3;
        half a =  0.734822;
        half b = -0.313376;
        half c =  0.0609169;
        half d =  0.5;
        half xCoverage = a * x + b * x3 + c * x5 + d;
        half y2 = y * y;
        half y3 = y2 * y;
        half y5 = y2 * y3;
        half yCoverage = a * y + b * y3 + c * y5 + d;
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
