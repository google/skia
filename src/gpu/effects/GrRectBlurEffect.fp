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

    // Get the smaller of the signed distance from the frag coord to the left and right edges and
    // similar for y.
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
    // This approximation was arrived at by eyeballing it. It looks visually closer than what a
    // R^2 regression produces.
    half r = 1 / (2.0 * sigma);
    x *= r;
    y *= r;
    half xCoverage;
    half yCoverage;
    @if (false) {
        half a = .417976;
        half c = 2.04804;
        xCoverage = clamp(a * atan(c * x) + 0.5, 0, 1);
        yCoverage = clamp(a * atan(c * y) + 0.5, 0, 1);
    } else if (false) {
        half aa = 1.0;
        xCoverage = smoothstep(-aa, aa, x);
        yCoverage = smoothstep(-aa, aa, y);
    } else if (false) {
        half a = -0.491892;
        half b = 1.114422;
        half c = 2.90153;
        half g = .178283;
        half h = 1.12178;
        half i = 1.04671;
        half j = 0.501241;

        x = clamp(x, -1.5, 1.5);
        half sx = sin(h * x + i);
        xCoverage = a * sin(b * x + c) + g * sx * sx * sx + j;

        y = clamp(y, -1.5, 1.5);
        half sy = sin(h * y + i);
        yCoverage = a * sin(b * y + c) + g * sy * sy * sy + j;
    } else if (false) {
        half x2 = x * x;
        half x3 = x2 *x;
        half x5 = x2 * x3;
        half a = .735299;
        half b = .31384;
        half c = .06111111;
        xCoverage = clamp(a*x - b*x3 + c*x5 + 0.5, 0, 1);
        half y2 = y * y;
        half y3 = y2 *y;
        half y5 = y2 * y3;
        yCoverage = clamp(a*y - b*y3 + c*y5 + 0.5, 0, 1);
    } else if (true) {
        x = -clamp(x, -1.5, 1.5);
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

        y = -clamp(y, -1.5, 1.5);
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
