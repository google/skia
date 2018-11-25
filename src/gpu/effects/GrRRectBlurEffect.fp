/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in float sigma;
layout(ctype=SkRect) in float4 rect;
in uniform float cornerRadius;
in uniform sampler2D ninePatchSampler;
layout(ctype=SkRect) uniform float4 proxyRect;
uniform half blurRadius;

@header {
    #include "GrClip.h"
    #include "GrContext.h"
    #include "GrContextPriv.h"
    #include "GrPaint.h"
    #include "GrProxyProvider.h"
    #include "GrRenderTargetContext.h"
    #include "GrStyle.h"
    #include "SkBlurMaskFilter.h"
    #include "SkGpuBlurUtils.h"
}

@class {
    static sk_sp<GrTextureProxy> find_or_create_rrect_blur_mask(GrContext* context,
                                                                const SkRRect& rrectToDraw,
                                                                const SkISize& size,
                                                                float xformedSigma) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 9);
        builder[0] = SkScalarCeilToInt(xformedSigma-1/6.0f);

        int index = 1;
        for (auto c : { SkRRect::kUpperLeft_Corner,  SkRRect::kUpperRight_Corner,
                        SkRRect::kLowerRight_Corner, SkRRect::kLowerLeft_Corner }) {
            SkASSERT(SkScalarIsInt(rrectToDraw.radii(c).fX) &&
                     SkScalarIsInt(rrectToDraw.radii(c).fY));
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fX);
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fY);
        }
        builder.finish();

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

        sk_sp<GrTextureProxy> mask(proxyProvider->findOrCreateProxyByUniqueKey(
                                                                 key, kBottomLeft_GrSurfaceOrigin));
        if (!mask) {
            // TODO: this could be approx but the texture coords will need to be updated
            sk_sp<GrRenderTargetContext> rtc(context->makeDeferredRenderTargetContextWithFallback(
                SkBackingFit::kExact, size.fWidth, size.fHeight, kAlpha_8_GrPixelConfig, nullptr));
            if (!rtc) {
                return nullptr;
            }

            GrPaint paint;

            rtc->clear(nullptr, 0x0, GrRenderTargetContext::CanClearFullscreen::kYes);
            rtc->drawRRect(GrNoClip(), std::move(paint), GrAA::kYes, SkMatrix::I(), rrectToDraw,
                           GrStyle::SimpleFill());

            sk_sp<GrTextureProxy> srcProxy(rtc->asTextureProxyRef());
            if (!srcProxy) {
                return nullptr;
            }
            sk_sp<GrRenderTargetContext> rtc2(
                      SkGpuBlurUtils::GaussianBlur(context,
                                                   std::move(srcProxy),
                                                   nullptr,
                                                   SkIRect::MakeWH(size.fWidth, size.fHeight),
                                                   SkIRect::EmptyIRect(),
                                                   xformedSigma,
                                                   xformedSigma,
                                                   GrTextureDomain::kIgnore_Mode,
                                                   SkBackingFit::kExact));
            if (!rtc2) {
                return nullptr;
            }

            mask = rtc2->asTextureProxyRef();
            if (!mask) {
                return nullptr;
            }
            SkASSERT(mask->origin() == kBottomLeft_GrSurfaceOrigin);
            proxyProvider->assignUniqueKeyToProxy(key, mask.get());
        }

        return mask;
    }
}

@optimizationFlags {
    kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrContext* context, float sigma,
                                                     float xformedSigma,
                                                     const SkRRect& srcRRect,
                                                     const SkRRect& devRRect);
}

@cpp {
    std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::Make(GrContext* context, float sigma,
                                                                 float xformedSigma,
                                                                 const SkRRect& srcRRect,
                                                                 const SkRRect& devRRect) {
        SkASSERT(!devRRect.isCircle() && !devRRect.isRect()); // Should've been caught up-stream

        // TODO: loosen this up
        if (!devRRect.isSimpleCircular()) {
            return nullptr;
        }

        // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be
        // sufficiently small relative to both the size of the corner radius and the
        // width (and height) of the rrect.
        SkRRect rrectToDraw;
        SkISize size;
        SkScalar ignored[SkBlurMaskFilter::kMaxDivisions];
        int ignoredSize;
        uint32_t ignored32;

        bool ninePatchable = SkBlurMaskFilter::ComputeBlurredRRectParams(srcRRect, devRRect,
                                                                         SkRect::MakeEmpty(),
                                                                         sigma, xformedSigma,
                                                                         &rrectToDraw, &size,
                                                                         ignored, ignored,
                                                                         ignored, ignored,
                                                                         &ignoredSize, &ignoredSize,
                                                                         &ignored32);
        if (!ninePatchable) {
            return nullptr;
        }

        sk_sp<GrTextureProxy> mask(find_or_create_rrect_blur_mask(context, rrectToDraw,
                                                                  size, xformedSigma));
        if (!mask) {
            return nullptr;
        }

        return std::unique_ptr<GrFragmentProcessor>(
                new GrRRectBlurEffect(xformedSigma, devRRect.getBounds(),
                                      devRRect.getSimpleRadii().fX, std::move(mask)));
    }
}

@test(d) {
    SkScalar w = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(1.f, 9.f);
    SkScalar sigma = d->fRandom->nextRangeF(1.f,10.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    return GrRRectBlurEffect::Make(d->context(), sigma, sigma, rrect, rrect);
}

void main() {
    // warp the fragment position to the appropriate part of the 9patch blur texture

    half2 rectCenter = (proxyRect.xy + proxyRect.zw) / 2.0;
    half2 translatedFragPos = sk_FragCoord.xy - proxyRect.xy;
    half threshold = cornerRadius + 2.0 * blurRadius;
    half2 middle = proxyRect.zw - proxyRect.xy - 2.0 * threshold;

    if (translatedFragPos.x >= threshold && translatedFragPos.x < (middle.x + threshold)) {
            translatedFragPos.x = threshold;
    } else if (translatedFragPos.x >= (middle.x + threshold)) {
        translatedFragPos.x -= middle.x - 1.0;
    }

    if (translatedFragPos.y > threshold && translatedFragPos.y < (middle.y+threshold)) {
        translatedFragPos.y = threshold;
    } else if (translatedFragPos.y >= (middle.y + threshold)) {
        translatedFragPos.y -= middle.y - 1.0;
    }

    half2 proxyDims = half2(2.0 * threshold + 1.0);
    half2 texCoord = translatedFragPos / proxyDims;

    sk_OutColor = sk_InColor * texture(ninePatchSampler, texCoord);
}

@setData(pdman) {
    float blurRadiusValue = 3.f * SkScalarCeilToScalar(sigma - 1 / 6.0f);
    pdman.set1f(blurRadius, blurRadiusValue);

    SkRect outset = rect;
    outset.outset(blurRadiusValue, blurRadiusValue);
    pdman.set4f(proxyRect, outset.fLeft, outset.fTop, outset.fRight, outset.fBottom);
}
