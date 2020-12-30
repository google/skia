/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
in float sigma;
layout(ctype=SkRect) in float4 rect;
in uniform half cornerRadius;
in fragmentProcessor ninePatchFP;
layout(ctype=SkRect) uniform float4 proxyRect;
uniform half blurRadius;

@header {
    #include "include/core/SkRect.h"
    class GrRecordingContext;
    class SkRRect;
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
    #include "include/gpu/GrDirectContext.h"
    #include "include/gpu/GrRecordingContext.h"
    #include "src/core/SkAutoMalloc.h"
    #include "src/core/SkGpuBlurUtils.h"
    #include "src/core/SkRRectPriv.h"
    #include "src/gpu/GrBitmapTextureMaker.h"
    #include "src/gpu/GrCaps.h"
    #include "src/gpu/GrDirectContextPriv.h"
    #include "src/gpu/GrPaint.h"
    #include "src/gpu/GrProxyProvider.h"
    #include "src/gpu/GrRecordingContextPriv.h"
    #include "src/gpu/GrStyle.h"
    #include "src/gpu/GrSurfaceDrawContext.h"
    #include "src/gpu/GrThreadSafeCache.h"
    #include "src/gpu/effects/GrTextureEffect.h"

    static constexpr auto kBlurredRRectMaskOrigin = kTopLeft_GrSurfaceOrigin;

    static void make_blurred_rrect_key(GrUniqueKey* key,
                                       const SkRRect& rrectToDraw,
                                       float xformedSigma) {
        SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

        GrUniqueKey::Builder builder(key, kDomain, 9, "RoundRect Blur Mask");
        builder[0] = SkScalarCeilToInt(xformedSigma-1/6.0f);

        int index = 1;
        // TODO: this is overkill for _simple_ circular rrects
        for (auto c : { SkRRect::kUpperLeft_Corner,  SkRRect::kUpperRight_Corner,
                        SkRRect::kLowerRight_Corner, SkRRect::kLowerLeft_Corner }) {
            SkASSERT(SkScalarIsInt(rrectToDraw.radii(c).fX) &&
                     SkScalarIsInt(rrectToDraw.radii(c).fY));
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fX);
            builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fY);
        }
        builder.finish();
    }

    static bool fillin_view_on_gpu(
                            GrDirectContext* dContext,
                            const GrSurfaceProxyView& lazyView,
                            sk_sp<GrThreadSafeCache::Trampoline> trampoline,
                            const SkRRect& rrectToDraw,
                            const SkISize& dimensions,
                            float xformedSigma) {
        SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
        std::unique_ptr<GrSurfaceDrawContext> rtc = GrSurfaceDrawContext::MakeWithFallback(
                dContext, GrColorType::kAlpha_8, nullptr, SkBackingFit::kExact, dimensions, 1,
                GrMipmapped::kNo, GrProtected::kNo, kBlurredRRectMaskOrigin);
        if (!rtc) {
            return false;
        }

        GrPaint paint;

        rtc->clear(SK_PMColor4fTRANSPARENT);
        rtc->drawRRect(nullptr, std::move(paint), GrAA::kYes, SkMatrix::I(), rrectToDraw,
                       GrStyle::SimpleFill());

        GrSurfaceProxyView srcView = rtc->readSurfaceView();
        SkASSERT(srcView.asTextureProxy());
        auto rtc2 = SkGpuBlurUtils::GaussianBlur(dContext,
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
        if (!rtc2 || !rtc2->readSurfaceView()) {
            return false;
        }

        auto view = rtc2->readSurfaceView();
        SkASSERT(view.swizzle() == lazyView.swizzle());
        SkASSERT(view.origin() == lazyView.origin());
        trampoline->fProxy = view.asTextureProxyRef();

        return true;
    }

    // Evaluate the vertical blur at the specified 'y' value given the location of the top of the
    // rrect.
    static uint8_t eval_V(float top, int y,
                          const uint8_t* integral, int integralSize, float sixSigma) {
        if (top < 0) {
            return 0; // an empty column
        }

        float fT = (top - y - 0.5f) * (integralSize/sixSigma);
        if (fT < 0) {
            return 255;
        } else if (fT >= integralSize-1) {
            return 0;
        }

        int lower = (int) fT;
        float frac = fT - lower;

        SkASSERT(lower+1 < integralSize);

        return integral[lower] * (1.0f-frac) + integral[lower+1] * frac;
    }

    // Apply a gaussian 'kernel' horizontally at the specified 'x', 'y' location.
    static uint8_t eval_H(int x, int y, const std::vector<float>& topVec,
                          const float* kernel, int kernelSize,
                          const uint8_t* integral, int integralSize, float sixSigma) {
        SkASSERT(0 <= x && x < (int) topVec.size());
        SkASSERT(kernelSize % 2);

        float accum = 0.0f;

        int xSampleLoc = x - (kernelSize / 2);
        for (int i = 0; i < kernelSize; ++i, ++xSampleLoc) {
            if (xSampleLoc < 0 || xSampleLoc >= (int) topVec.size()) {
                continue;
            }

            accum += kernel[i] * eval_V(topVec[xSampleLoc], y, integral, integralSize, sixSigma);
        }

        return accum + 0.5f;
    }

    // Create a cpu-side blurred-rrect mask that is close to the version the gpu would've produced.
    // The match needs to be close bc the cpu- and gpu-generated version must be interchangeable.
    static GrSurfaceProxyView create_mask_on_cpu(GrRecordingContext* rContext,
                                                 const SkRRect& rrectToDraw,
                                                 const SkISize& dimensions,
                                                 float xformedSigma) {
        SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
        int radius = SkGpuBlurUtils::SigmaRadius(xformedSigma);
        int kernelSize = 2*radius + 1;

        SkASSERT(kernelSize %2);
        SkASSERT(dimensions.width() % 2);
        SkASSERT(dimensions.height() % 2);

        SkVector radii = rrectToDraw.getSimpleRadii();
        SkASSERT(SkScalarNearlyEqual(radii.fX, radii.fY));

        const int halfWidthPlus1 = (dimensions.width() / 2) + 1;
        const int halfHeightPlus1 = (dimensions.height() / 2) + 1;

        std::unique_ptr<float[]> kernel(new float[kernelSize]);

        SkGpuBlurUtils::Compute1DGaussianKernel(kernel.get(), xformedSigma, radius);

        SkBitmap integral;
        if (!SkGpuBlurUtils::CreateIntegralTable(6*xformedSigma, &integral)) {
            return {};
        }

        SkBitmap result;
        if (!result.tryAllocPixels(SkImageInfo::MakeA8(dimensions.width(), dimensions.height()))) {
            return {};
        }

        std::vector<float> topVec;
        topVec.reserve(dimensions.width());
        for (int x = 0; x < dimensions.width(); ++x) {
            if (x < rrectToDraw.rect().fLeft || x > rrectToDraw.rect().fRight) {
                topVec.push_back(-1);
            } else {
                if (x+0.5f < rrectToDraw.rect().fLeft + radii.fX) { // in the circular section
                    float xDist = rrectToDraw.rect().fLeft + radii.fX - x - 0.5f;
                    float h = sqrtf(radii.fX * radii.fX - xDist * xDist);
                    SkASSERT(0 <= h && h < radii.fY);
                    topVec.push_back(rrectToDraw.rect().fTop+radii.fX-h + 3*xformedSigma);
                } else {
                    topVec.push_back(rrectToDraw.rect().fTop + 3*xformedSigma);
                }
            }
        }

        for (int y = 0; y < halfHeightPlus1; ++y) {
            uint8_t* scanline = result.getAddr8(0, y);

            for (int x = 0; x < halfWidthPlus1; ++x) {
                scanline[x] = eval_H(x, y, topVec,
                                     kernel.get(), kernelSize,
                                     integral.getAddr8(0, 0), integral.width(), 6*xformedSigma);
                scanline[dimensions.width()-x-1] = scanline[x];
            }

            memcpy(result.getAddr8(0, dimensions.height()-y-1), scanline, result.rowBytes());
        }

        result.setImmutable();

        GrBitmapTextureMaker maker(rContext, result, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        auto view = maker.view(GrMipmapped::kNo);
        if (!view) {
            return {};
        }

        SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
        return view;
    }

    static std::unique_ptr<GrFragmentProcessor> find_or_create_rrect_blur_mask_fp(
            GrRecordingContext* rContext,
            const SkRRect& rrectToDraw,
            const SkISize& dimensions,
            float xformedSigma) {
        SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
        GrUniqueKey key;
        make_blurred_rrect_key(&key, rrectToDraw, xformedSigma);

        auto threadSafeCache = rContext->priv().threadSafeCache();

        // It seems like we could omit this matrix and modify the shader code to not normalize
        // the coords used to sample the texture effect. However, the "proxyDims" value in the
        // shader is not always the actual the proxy dimensions. This is because 'dimensions' here
        // was computed using integer corner radii as determined in
        // SkComputeBlurredRRectParams whereas the shader code uses the float radius to compute
        // 'proxyDims'. Why it draws correctly with these unequal values is a mystery for the ages.
        auto m = SkMatrix::Scale(dimensions.width(), dimensions.height());

        GrSurfaceProxyView view;

        if (GrDirectContext* dContext = rContext->asDirectContext()) {
            // The gpu thread gets priority over the recording threads. If the gpu thread is first,
            // it crams a lazy proxy into the cache and then fills it in later.
            auto[lazyView, trampoline] = GrThreadSafeCache::CreateLazyView(
                                    dContext, GrColorType::kAlpha_8, dimensions,
                                    kBlurredRRectMaskOrigin, SkBackingFit::kExact);
            if (!lazyView) {
                return nullptr;
            }

            view = threadSafeCache->findOrAdd(key, lazyView);
            if (view != lazyView) {
                SkASSERT(view.asTextureProxy());
                SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
                return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
            }

            if (!fillin_view_on_gpu(dContext, lazyView, std::move(trampoline),
                                    rrectToDraw, dimensions, xformedSigma)) {
                // In this case something has gone disastrously wrong so set up to drop the draw
                // that needed this resource and reduce future pollution of the cache.
                threadSafeCache->remove(key);
                return nullptr;
            }
        } else {
            view = threadSafeCache->find(key);
            if (view) {
                SkASSERT(view.asTextureProxy());
                SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
                return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
            }

            view = create_mask_on_cpu(rContext, rrectToDraw, dimensions, xformedSigma);
            if (!view) {
                return nullptr;
            }

            view = threadSafeCache->add(key, view);
        }

        SkASSERT(view.asTextureProxy());
        SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
        return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
    }

    std::unique_ptr<GrFragmentProcessor> GrRRectBlurEffect::Make(
            std::unique_ptr<GrFragmentProcessor> inputFP,
            GrRecordingContext* context,
            float sigma,
            float xformedSigma,
            const SkRRect& srcRRect,
            const SkRRect& devRRect) {
        // Should've been caught up-stream
#ifdef SK_DEBUG
        SkASSERTF(!SkRRectPriv::IsCircle(devRRect), "Unexpected circle. %d\n\t%s\n\t%s",
                  SkRRectPriv::IsCircle(srcRRect),
                  srcRRect.dumpToString(true).c_str(), devRRect.dumpToString(true).c_str());
        SkASSERTF(!devRRect.isRect(), "Unexpected rect. %d\n\t%s\n\t%s",
                  srcRRect.isRect(),
                  srcRRect.dumpToString(true).c_str(), devRRect.dumpToString(true).c_str());
#endif
        // TODO: loosen this up
        if (!SkRRectPriv::IsSimpleCircular(devRRect)) {
            return nullptr;
        }

        if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
            return inputFP;
        }

        // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be
        // sufficiently small relative to both the size of the corner radius and the
        // width (and height) of the rrect.
        SkRRect rrectToDraw;
        SkISize dimensions;
        SkScalar ignored[SkGpuBlurUtils::kBlurRRectMaxDivisions];

        bool ninePatchable = SkGpuBlurUtils::ComputeBlurredRRectParams(srcRRect, devRRect,
                                                                       sigma, xformedSigma,
                                                                       &rrectToDraw, &dimensions,
                                                                       ignored, ignored,
                                                                       ignored, ignored);
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

half4 main() {
    // Warp the fragment position to the appropriate part of the 9-patch blur texture by snipping
    // out the middle section of the proxy rect.
    float2 translatedFragPosFloat = sk_FragCoord.xy - proxyRect.LT;
    float2 proxyCenter = (proxyRect.RB - proxyRect.LT) * 0.5;
    half edgeSize = 2.0 * blurRadius + cornerRadius + 0.5;

    // Position the fragment so that (0, 0) marks the center of the proxy rectangle.
    // Negative coordinates are on the left/top side and positive numbers are on the right/bottom.
    translatedFragPosFloat -= proxyCenter;

    // Temporarily strip off the fragment's sign. x/y are now strictly increasing as we move away
    // from the center.
    half2 fragDirection = half2(sign(translatedFragPosFloat));
    translatedFragPosFloat = abs(translatedFragPosFloat);

    // Our goal is to snip out the "middle section" of the proxy rect (everything but the edge).
    // We've repositioned our fragment position so that (0, 0) is the centerpoint and x/y are always
    // positive, so we can subtract here and interpret negative results as being within the middle
    // section.
    half2 translatedFragPosHalf = half2(translatedFragPosFloat - (proxyCenter - edgeSize));

    // Remove the middle section by clamping to zero.
    translatedFragPosHalf = max(translatedFragPosHalf, 0);

    // Reapply the fragment's sign, so that negative coordinates once again mean left/top side and
    // positive means bottom/right side.
    translatedFragPosHalf *= fragDirection;

    // Offset the fragment so that (0, 0) marks the upper-left again, instead of the center point.
    translatedFragPosHalf += half2(edgeSize);

    half2 proxyDims = half2(2.0 * edgeSize);
    half2 texCoord = translatedFragPosHalf / proxyDims;

    return sample(inputFP) * sample(ninePatchFP, texCoord).a;
}

@setData(pdman) {
    float blurRadiusValue = 3.f * SkScalarCeilToScalar(sigma - 1 / 6.0f);
    pdman.set1f(blurRadius, blurRadiusValue);

    SkRect outset = rect;
    outset.outset(blurRadiusValue, blurRadiusValue);
    pdman.set4f(proxyRect, outset.fLeft, outset.fTop, outset.fRight, outset.fBottom);
}
