/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in float sigma;
layout(ctype=SkRect) in float4 rect;
in uniform half cornerRadius;
in uniform sampler2D ninePatchSampler;
layout(ctype=SkRect) uniform float4 proxyRect;
uniform half blurRadius;

@header {
    #include "include/effects/SkBlurMaskFilter.h"
    #include "include/gpu/GrContext.h"
    #include "include/private/GrRecordingContext.h"
    #include "src/core/SkBlurPriv.h"
    #include "src/core/SkGpuBlurUtils.h"
    #include "src/core/SkRRectPriv.h"
    #include "src/gpu/GrCaps.h"
    #include "src/gpu/GrClip.h"
    #include "src/gpu/GrPaint.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrRecordingContextPriv.h"
    #include "src/gpu/GrRenderTargetContext.h"
    #include "src/gpu/GrStyle.h"
}

@class {
    static sk_sp<GrTextureProxy> find_or_create_rrect_blur_mask(GrRecordingContext* context,
                                                                const SkRRect& rrectToDraw,
                                                                const SkISize& size,
                                                                float xformedSigma) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        GrUniqueKey::Builder builder(&key, kDomain, 9, "RoundRect Blur Mask");
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

        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        sk_sp<GrTextureProxy> mask(proxyProvider->findOrCreateProxyByUniqueKey(
                key, GrColorType::kAlpha_8, kBottomLeft_GrSurfaceOrigin));
        if (!mask) {
            // TODO: this could be SkBackingFit::kApprox, but:
            //   1) The texture coords would need to be updated.
            //   2) We would have to use GrTextureDomain::kClamp_Mode for the GaussianBlur.
            auto rtc = context->priv().makeDeferredRenderTargetContextWithFallback(
                    SkBackingFit::kExact, size.fWidth, size.fHeight, GrColorType::kAlpha_8,
                    nullptr);
            if (!rtc) {
                return nullptr;
            }

            GrPaint paint;

            rtc->clear(nullptr, SK_PMColor4fTRANSPARENT,
                       GrRenderTargetContext::CanClearFullscreen::kYes);
            rtc->drawRRect(GrNoClip(), std::move(paint), GrAA::kYes, SkMatrix::I(), rrectToDraw,
                           GrStyle::SimpleFill());

            sk_sp<GrTextureProxy> srcProxy(rtc->asTextureProxyRef());
            if (!srcProxy) {
                return nullptr;
            }
            auto rtc2 =
                      SkGpuBlurUtils::GaussianBlur(context,
                                                   std::move(srcProxy),
                                                   rtc->colorInfo().colorType(),
                                                   rtc->colorInfo().alphaType(),
                                                   SkIPoint::Make(0, 0),
                                                   nullptr,
                                                   SkIRect::MakeWH(size.fWidth, size.fHeight),
                                                   SkIRect::EmptyIRect(),
                                                   xformedSigma,
                                                   xformedSigma,
                                                   GrTextureDomain::kIgnore_Mode,
                                                   SkBackingFit::kExact);
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
    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext* context,
                                                     float sigma,
                                                     float xformedSigma,
                                                     const SkRRect& srcRRect,
                                                     const SkRRect& devRRect);
}

@cpp {
    std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::Make(GrRecordingContext* context,
                                                                 float sigma,
                                                                 float xformedSigma,
                                                                 const SkRRect& srcRRect,
                                                                 const SkRRect& devRRect) {
        SkASSERT(!SkRRectPriv::IsCircle(devRRect) && !devRRect.isRect()); // Should've been caught up-stream

        // TODO: loosen this up
        if (!SkRRectPriv::IsSimpleCircular(devRRect)) {
            return nullptr;
        }

        // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be
        // sufficiently small relative to both the size of the corner radius and the
        // width (and height) of the rrect.
        SkRRect rrectToDraw;
        SkISize size;
        SkScalar ignored[kSkBlurRRectMaxDivisions];
        int ignoredSize;
        uint32_t ignored32;

        bool ninePatchable = SkComputeBlurredRRectParams(srcRRect, devRRect,
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
                                      SkRRectPriv::GetSimpleRadii(devRRect).fX, std::move(mask)));
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

    half2 rectCenter = half2((proxyRect.xy + proxyRect.zw) / 2.0);
    half2 translatedFragPos = half2(sk_FragCoord.xy - proxyRect.xy);
    half threshold = cornerRadius + 2.0 * blurRadius;
    half2 middle = half2(proxyRect.zw - proxyRect.xy - 2.0 * threshold);

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

    sk_OutColor = sk_InColor * sample(ninePatchSampler, texCoord);
}

@setData(pdman) {
    float blurRadiusValue = 3.f * SkScalarCeilToScalar(sigma - 1 / 6.0f);
    pdman.set1f(blurRadius, blurRadiusValue);

    SkRect outset = rect;
    outset.outset(blurRadiusValue, blurRadiusValue);
    pdman.set4f(proxyRect, outset.fLeft, outset.fTop, outset.fRight, outset.fBottom);
}
