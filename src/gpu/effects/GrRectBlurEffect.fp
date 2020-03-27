/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
#include <cmath>
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrShaderCaps.h"
}

in float4 rect;

layout(key) bool highp = abs(rect.x) > 16000 || abs(rect.y) > 16000 ||
                         abs(rect.z) > 16000 || abs(rect.w) > 16000;

layout(when= highp) uniform float4 rectF;
layout(when=!highp) uniform half4  rectH;

// Texture that is a LUT for integral of normal distribution. The value at x (where x is a texture
// coord between 0 and 1) is the integral from -inf to (3 * sigma * (-2 * x - 1)). I.e. x is mapped
// 0 3*sigma to -3 sigma. The flip saves a reversal in the shader.
in uniform sampler2D integral;
// Used to produce normalized texture coords for lookups in 'integral'
in uniform half invSixSigma;

// There is a fast variant of the effect that does 2 texture lookups and a more general one for
// wider blurs relative to rect sizes that does 4.
layout(key) in bool isFast;

@constructorParams {
    GrSamplerState samplerParams
}

@samplerParams(integral) {
    samplerParams
}
@class {
static GrSurfaceProxyView CreateIntegralTexture(GrRecordingContext* context, float sixSigma) {
    // The texture we're producing represents the integral of a normal distribution over a six-sigma
    // range centered at zero. We want enough resolution so that the linear interpolation done in
    // texture lookup doesn't introduce noticeable artifacts. We conservatively choose to have 2
    // texels for each dst pixel.
    int minWidth = 2 * sk_float_ceil2int(sixSigma);
    // Bin by powers of 2 with a minimum so we get good profile reuse.
    int width = std::max(SkNextPow2(minWidth), 32);

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
    builder[0] = width;
    builder.finish();

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    if (sk_sp<GrTextureProxy> proxy = proxyProvider->findOrCreateProxyByUniqueKey(key)) {
        GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                   GrColorType::kAlpha_8);
        return {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::MakeA8(width, 1))) {
        return {};
    }
    *bitmap.getAddr8(0, 0) = 255;
    const float invWidth = 1.f / width;
    for (int i = 1; i < width - 1; ++i) {
        float x = (i + 0.5f) * invWidth;
        x = (-6 * x + 3) * SK_ScalarRoot2Over2;
        float integral = 0.5f * (std::erf(x) + 1.f);
        *bitmap.getAddr8(i, 0) = SkToU8(sk_float_round2int(255.f * integral));
    }
    *bitmap.getAddr8(width - 1, 0) = 0;
    bitmap.setImmutable();

    GrBitmapTextureMaker maker(context, bitmap, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    auto view = maker.view(GrMipMapped::kNo);
    if (!view) {
        return {};
    }
    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    return view;
}
}

@make {
     static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext* context,
                                                      const GrShaderCaps& caps,
                                                      const SkRect& rect, float sigma) {
         SkASSERT(rect.isSorted());
         if (!caps.floatIs32Bits()) {
             // We promote the math that gets us into the Gaussian space to full float when the rect
             // coords are large. If we don't have full float then fail. We could probably clip the
             // rect to an outset device bounds instead.
             if (SkScalarAbs(rect.fLeft)  > 16000.f || SkScalarAbs(rect.fTop)    > 16000.f ||
                 SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f) {
                    return nullptr;
             }
         }

         const float sixSigma = 6 * sigma;
         GrSurfaceProxyView integral = CreateIntegralTexture(context, sixSigma);
         if (!integral) {
             return nullptr;
         }

         // In the fast variant we think of the midpoint of the integral texture as aligning
         // with the closest rect edge both in x and y. To simplify texture coord calculation we
         // inset the rect so that the edge of the inset rect corresponds to t = 0 in the texture.
         // It actually simplifies things a bit in the !isFast case, too.
         float threeSigma = sixSigma / 2;
         SkRect insetRect = {rect.fLeft   + threeSigma,
                             rect.fTop    + threeSigma,
                             rect.fRight  - threeSigma,
                             rect.fBottom - threeSigma};

         // In our fast variant we find the nearest horizontal and vertical edges and for each
         // do a lookup in the integral texture for each and multiply them. When the rect is
         // less than 6 sigma wide then things aren't so simple and we have to consider both the
         // left and right edge of the rectangle (and similar in y).
         bool isFast = insetRect.isSorted();
         // 1 / (6 * sigma) is the domain of the integral texture. We use the inverse to produce
         // normalized texture coords from frag coord distances.
         float invSixSigma = 1.f / sixSigma;
         return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(insetRect,
                 std::move(integral), invSixSigma, isFast, GrSamplerState::Filter::kBilerp));
     }
}

void main() {
        half xCoverage, yCoverage;
        @if (isFast) {
            // Get the smaller of the signed distance from the frag coord to the left and right
            // edges and similar for y.
            // The integral texture goes "backwards" (from 3*sigma to -3*sigma), So, the below
            // computations align the left edge of the integral texture with the inset rect's edge
            // extending outward 6 * sigma from the inset rect.
            half x, y;
            @if (highp) {
                x = max(half(rectF.x - sk_FragCoord.x), half(sk_FragCoord.x - rectF.z));
                y = max(half(rectF.y - sk_FragCoord.y), half(sk_FragCoord.y - rectF.w));
           } else {
                x = max(half(rectH.x - sk_FragCoord.x), half(sk_FragCoord.x - rectH.z));
                y = max(half(rectH.y - sk_FragCoord.y), half(sk_FragCoord.y - rectH.w));
            }
            xCoverage = sample(integral, half2(x * invSixSigma, 0.5)).a;
            yCoverage = sample(integral, half2(y * invSixSigma, 0.5)).a;
            sk_OutColor = sk_InColor * xCoverage * yCoverage;
        } else {
            // We just consider just the x direction here. In practice we compute x and y separately
            // and multiply them together.
            // We define our coord system so that the point at which we're evaluating a kernel
            // defined by the normal distribution (K) as  0. In this coord system let L be left
            // edge and R be the right edge of the rectangle.
            // We can calculate C by integrating K with the half infinite ranges outside the L to R
            // range and subtracting from 1:
            //   C = 1 - <integral of K from from -inf to  L> - <integral of K from R to inf>
            // K is symmetric about x=0 so:
            //   C = 1 - <integral of K from from -inf to  L> - <integral of K from -inf to -R>

            // The integral texture goes "backwards" (from 3*sigma to -3*sigma) which is factored
            // in to the below calculations.
            // Also, our rect uniform was pre-inset by 3 sigma from the actual rect being blurred,
            // also factored in.
            half l, r, t, b;
            @if (highp) {
                l = half(sk_FragCoord.x - rectF.x);
                r = half(rectF.z - sk_FragCoord.x);
                t = half(sk_FragCoord.y - rectF.y);
                b = half(rectF.w - sk_FragCoord.y);
            } else {
                l = half(sk_FragCoord.x - rectH.x);
                r = half(rectH.z - sk_FragCoord.x);
                t = half(sk_FragCoord.y - rectH.y);
                b = half(rectH.w - sk_FragCoord.y);
            }
            half il = 1 + l * invSixSigma;
            half ir = 1 + r * invSixSigma;
            half it = 1 + t * invSixSigma;
            half ib = 1 + b * invSixSigma;
            xCoverage = 1 - sample(integral, half2(il, 0.5)).a
                          - sample(integral, half2(ir, 0.5)).a;
            yCoverage = 1 - sample(integral, half2(it, 0.5)).a
                          - sample(integral, half2(ib, 0.5)).a;
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
    return GrRectBlurEffect::Make(data->context(), *data->caps()->shaderCaps(),
                                  SkRect::MakeWH(width, height), sigma);
}
