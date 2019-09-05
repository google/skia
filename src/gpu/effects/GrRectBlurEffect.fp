/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
#include "include/core/SkScalar.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrShaderCaps.h"
}

in float4 rect;

layout(key) bool highp = abs(rect.x) > 16000 || abs(rect.y) > 16000 ||
                         abs(rect.z) > 16000 || abs(rect.w) > 16000;

layout(when= highp) uniform float4 rectF;
layout(when=!highp) uniform half4  rectH;

// Texture that is a LUT for integral of normal distribution with the end points pinned to zero and
// 1. It's always logically six sigma wide with x=0 at the center of the texture. The function that
// populates it actually produces 1 - integral. The texture is always 1 texel tall.
in uniform sampler2D integral;
// Used to produce normalized texture coords for lookups in 'integral'
in uniform half invSixSigma;

// There is a fast variant of the effect that does 2 texture lookups and a more general one for
// wider blurs relative to rect sizes that does 4.
layout(key) in bool isFast;

@constructorParams {
    GrSamplerState samplerParams
}

@samplerParams(blurProfile) {
    samplerParams
}
@class {
static sk_sp<GrTextureProxy> CreateIntegralTexture(GrProxyProvider* proxyProvider,
                                                   float sixSigma) {
    // The texture we're producing represents the integral of a normal distribution over a six-sigma
    // range centered at zero. We want enough resolution so that the linear interpolation done in
    // texture lookup doesn't introduce noticeable artifacts. We conservatively choose to have 2
    // texels for each dst pixel.
    int minWidth = 2 * sk_float_ceil2int(sixSigma);
    // Bin by powers of 2 with a minimum so we get good profile reuse.
    int width = SkTMax(SkNextPow2(minWidth), 32);

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
    builder[0] = width;
    builder.finish();

    sk_sp<GrTextureProxy> blurProfile(proxyProvider->findOrCreateProxyByUniqueKey(
            key, GrColorType::kAlpha_8, kTopLeft_GrSurfaceOrigin));
    if (!blurProfile) {
        SkBitmap bitmap;
        if (!bitmap.tryAllocPixels(SkImageInfo::MakeA8(width, 1))) {
            return nullptr;
        }
        // This takes the desired width and the sigma. It expects that ceil(6 * sigma) == width and
        // asserts that it is. Note that this is where the 1 - integral comes into play, as that's
        // what ComputeBlurProfjle produces.
        SkBlurMask::ComputeBlurProfile(bitmap.getAddr8(0, 0), width, width / 6.f);
        bitmap.setImmutable();
        blurProfile = proxyProvider->createProxyFromBitmap(bitmap, GrMipMapped::kNo);
        if (!blurProfile) {
            return nullptr;
        }
        SkASSERT(blurProfile->origin() == kTopLeft_GrSurfaceOrigin);
        proxyProvider->assignUniqueKeyToProxy(key, blurProfile.get());
    }
    return blurProfile;
}
}

@make {
     static std::unique_ptr<GrFragmentProcessor> Make(GrProxyProvider* proxyProvider,
                                                      const GrShaderCaps& caps,
                                                      const SkRect& rect, float sigma) {
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
         auto integral = CreateIntegralTexture(proxyProvider, sixSigma);
         if (!integral) {
             return nullptr;
         }

         // In our fast variant we find the nearest horizontal and vertical edges and for each
         // do a lookup in the integral texture for each and multiply them. When the rect is
         // less than 6 sigma wide then things aren't so simple and we have to consider both the
         // left and right edge of the rectangle (and similar in y).
         bool isFast = sixSigma < (float) rect.width() || sixSigma < (float) rect.height();

         // In the fast variant we thing of the midpoint of the integral texture as aligning
         // with the closest rect edge both in x and y. To simplify texture coord calculation we
         // inset the rect so that the edge of the inset rect corresponds to t = 0 in the texture.
         // It actually simplifies things a bit in the !isFast case, too.
         float threeSigma = sixSigma / 2;
         auto insetRect = rect.makeInset(threeSigma, threeSigma);
         // 1 / (6 * sigma) is the domain of the integral texture. We use the inverse to produce
         // normalized texture coords from frag coord distances.
         float invSixSigma = 1.f / sixSigma;
         return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(insetRect,
                 std::move(integral), invSixSigma, isFast, GrSamplerState::ClampBilerp()));
     }
}

void main() {
        half xCoverage, yCoverage;
        @if (isFast) {
            // Get the smaller of the signed distance from the frag coord to the left and right
            // edges and similar for y.
            // The blur profile computed by SkMaskFilter::ComputeBlurProfile is actually 1 -
            // integral. So, the below computations align the left edge of the integral texture with
            // the inset rect's edge and the right edge of the texture being 6 * sigma outward from
            // the inset rect.
            half x;
            @ if (highp) {
                x = max(half(rectF.x - sk_FragCoord.x), half(sk_FragCoord.x - rectF.z));
            }
            else {
                x = max(half(rectH.x - sk_FragCoord.x), half(sk_FragCoord.x - rectH.z));
            }
            half y;
            @ if (highp) {
                y = max(half(rectF.y - sk_FragCoord.y), half(sk_FragCoord.y - rectF.w));
            }
            else {
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

            // When the 1D profile texture is sampled at x it actually gives us 1 - integral. (Just
            // because that's how SkMaskFilter::ComputeBlurProfile happens to work).
            // Also, our rect uniform was pre-inset by 3 sigma from the actual rect being blurred.
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
    return GrRectBlurEffect::Make(data->proxyProvider(), *data->caps()->shaderCaps(),
                                  SkRect::MakeWH(width, height), sigma);
}
