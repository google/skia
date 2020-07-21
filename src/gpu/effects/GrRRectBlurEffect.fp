/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? inputFP;
in float sigma;
layout(ctype=SkRect) in float4 rect;
in uniform half cornerRadius;
in fragmentProcessor ninePatchFP;
layout(ctype=SkRect) uniform float4 proxyRect;
uniform half blurRadius;

@header {
    #include "include/gpu/GrRecordingContext.h"
    #include "src/core/SkBlurPriv.h"
    #include "src/core/SkGpuBlurUtils.h"
    #include "src/core/SkRRectPriv.h"
    #include "src/gpu/GrCaps.h"
    #include "src/gpu/GrPaint.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrRecordingContextPriv.h"
    #include "src/gpu/GrRenderTargetContext.h"
    #include "src/gpu/GrStyle.h"
    #include "src/gpu/effects/GrTextureEffect.h"
}

@class {
    static std::unique_ptr<GrFragmentProcessor> find_or_create_rrect_blur_mask_fp(
            GrRecordingContext* context,
            const SkRRect& rrectToDraw,
            const SkISize& dimensions,
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

        // It seems like we could omit this matrix and modify the shader code to not normalize
        // the coords used to sample the texture effect. However, the "proxyDims" value in the
        // shader is not always the actual the proxy dimensions. This is because 'dimensions' here
        // was computed using integer corner radii as determined in
        // SkComputeBlurredRRectParams whereas the shader code uses the float radius to compute
        // 'proxyDims'. Why it draws correctly with these unequal values is a mystery for the ages.
        auto m = SkMatrix::Scale(dimensions.width(), dimensions.height());
        static constexpr auto kMaskOrigin = kBottomLeft_GrSurfaceOrigin;
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        if (auto view = proxyProvider->findCachedProxyWithColorTypeFallback(
                key, kMaskOrigin, GrColorType::kAlpha_8, 1)) {
            return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
        }

        auto rtc = GrRenderTargetContext::MakeWithFallback(
                context, GrColorType::kAlpha_8, nullptr, SkBackingFit::kExact, dimensions, 1,
                GrMipmapped::kNo, GrProtected::kNo, kMaskOrigin);
        if (!rtc) {
            return nullptr;
        }

        GrPaint paint;

        rtc->clear(SK_PMColor4fTRANSPARENT);
        rtc->drawRRect(nullptr, std::move(paint), GrAA::kYes, SkMatrix::I(), rrectToDraw,
                       GrStyle::SimpleFill());

        GrSurfaceProxyView srcView = rtc->readSurfaceView();
        if (!srcView) {
            return nullptr;
        }
        SkASSERT(srcView.asTextureProxy());
        auto rtc2 = SkGpuBlurUtils::GaussianBlur(context,
                                                 std::move(srcView),
                                                 rtc->colorInfo().colorType(),
                                                 rtc->colorInfo().alphaType(),
                                                 nullptr,
                                                 SkIRect::MakeSize(dimensions),
                                                 SkIRect::MakeSize(dimensions),
                                                 xformedSigma,
                                                 xformedSigma,
                                                 SkTileMode::kClamp,
                                                 SkBackingFit::kExact);
        if (!rtc2) {
            return nullptr;
        }

        GrSurfaceProxyView mask = rtc2->readSurfaceView();
        if (!mask) {
            return nullptr;
        }
        SkASSERT(mask.asTextureProxy());
        SkASSERT(mask.origin() == kMaskOrigin);
        proxyProvider->assignUniqueKeyToProxy(key, mask.asTextureProxy());
        return GrTextureEffect::Make(std::move(mask), kPremul_SkAlphaType, m);
    }
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
            kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     GrRecordingContext* context,
                                                     float sigma,
                                                     float xformedSigma,
                                                     const SkRRect& srcRRect,
                                                     const SkRRect& devRRect);
}

@cpp {
    std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::Make(
            std::unique_ptr<GrFragmentProcessor> inputFP,
            GrRecordingContext* context,
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
        SkISize dimensions;
        SkScalar ignored[kSkBlurRRectMaxDivisions];
        int ignoredSize;
        uint32_t ignored32;

        bool ninePatchable = SkComputeBlurredRRectParams(srcRRect, devRRect,
                                                         SkRect::MakeEmpty(),
                                                         sigma, xformedSigma,
                                                         &rrectToDraw, &dimensions,
                                                         ignored, ignored,
                                                         ignored, ignored,
                                                         &ignoredSize, &ignoredSize,
                                                         &ignored32);
        if (!ninePatchable) {
            return nullptr;
        }

        std::unique_ptr<GrFragmentProcessor> maskFP = find_or_create_rrect_blur_mask_fp(
                context, rrectToDraw, dimensions, xformedSigma);
        if (!maskFP) {
            return nullptr;
        }

        return std::unique_ptr<GrFragmentProcessor>(
                new GrRRectBlurEffect(std::move(inputFP), xformedSigma, devRRect.getBounds(),
                                      SkRRectPriv::GetSimpleRadii(devRRect).fX, std::move(maskFP)));
    }
}

@test(d) {
    SkScalar w = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(100.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(1.f, 9.f);
    SkScalar sigma = d->fRandom->nextRangeF(1.f,10.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    return GrRRectBlurEffect::Make(d->inputFP(), d->context(), sigma, sigma, rrect, rrect);
}

void main() {
    // Warp the fragment position to the appropriate part of the 9-patch blur texture by snipping
    // out the middle section of the proxy rect.
    half2 translatedFragPos = half2(sk_FragCoord.xy - proxyRect.LT);
    half2 proxyCenter = half2((proxyRect.RB - proxyRect.LT) * 0.5);
    half edgeSize = 2.0 * blurRadius + cornerRadius + 0.5;

    // Position the fragment so that (0, 0) marks the center of the proxy rectangle.
    // Negative coordinates are on the left/top side and positive numbers are on the right/bottom.
    translatedFragPos -= proxyCenter;

    // Temporarily strip off the fragment's sign. x/y are now strictly increasing as we move away
    // from the center.
    half2 fragDirection = sign(translatedFragPos);
    translatedFragPos = abs(translatedFragPos);

    // Our goal is to snip out the "middle section" of the proxy rect (everything but the edge).
    // We've repositioned our fragment position so that (0, 0) is the centerpoint and x/y are always
    // positive, so we can subtract here and interpret negative results as being within the middle
    // section.
    translatedFragPos -= proxyCenter - edgeSize;

    // Remove the middle section by clamping to zero.
    translatedFragPos = max(translatedFragPos, 0);

    // Reapply the fragment's sign, so that negative coordinates once again mean left/top side and
    // positive means bottom/right side.
    translatedFragPos *= fragDirection;

    // Offset the fragment so that (0, 0) marks the upper-left again, instead of the center point.
    translatedFragPos += half2(edgeSize);

    half2 proxyDims = half2(2.0 * edgeSize);
    half2 texCoord = translatedFragPos / proxyDims;

    half4 inputColor = sample(inputFP);
    sk_OutColor = inputColor * sample(ninePatchFP, texCoord);
}

@setData(pdman) {
    float blurRadiusValue = 3.f * SkScalarCeilToScalar(sigma - 1 / 6.0f);
    pdman.set1f(blurRadius, blurRadiusValue);

    SkRect outset = rect;
    outset.outset(blurRadiusValue, blurRadiusValue);
    pdman.set4f(proxyRect, outset.fLeft, outset.fTop, outset.fRight, outset.fBottom);
}
