/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/image/SkImage_Base.h"

static GrSurfaceProxyView blur(GrRecordingContext* ctx,
                               GrSurfaceProxyView src,
                               SkIRect dstB,
                               SkIRect srcB,
                               float sigmaX,
                               float sigmaY,
                               SkTileMode mode) {
    auto resultRTC =
            SkGpuBlurUtils::GaussianBlur(ctx, src, GrColorType::kRGBA_8888, kPremul_SkAlphaType,
                                         nullptr, dstB, srcB, sigmaX, sigmaY, mode);
    if (!resultRTC) {
        return {};
    }
    return resultRTC->readSurfaceView();
};

static void run(GrRecordingContext* rContext, GrSurfaceDrawContext* rtc, bool subsetSrc, bool ref) {
    auto srcII = SkImageInfo::Make(60, 60, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, srcII);
    GrSurfaceProxyView src;
    if (surf) {
        SkScalar w = surf->width();
        SkScalar h = surf->height();
        surf->getCanvas()->drawColor(SK_ColorDKGRAY);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        // Draw four horizontal lines at 1/8, 1/4, 3/4, 7/8.
        paint.setStrokeWidth(h/12.f);
        paint.setColor(SK_ColorRED);
        surf->getCanvas()->drawLine({0.f, 1.f*h/8.f}, {w, 1.f*h/8.f}, paint);
        paint.setColor(/* sea foam */ 0xFF71EEB8);
        surf->getCanvas()->drawLine({0.f, 1.f*h/4.f}, {w, 1.f*h/4.f}, paint);
        paint.setColor(SK_ColorYELLOW);
        surf->getCanvas()->drawLine({0.f, 3.f*h/4.f}, {w, 3.f*h/4.f}, paint);
        paint.setColor(SK_ColorCYAN);
        surf->getCanvas()->drawLine({0.f, 7.f*h/8.f}, {w, 7.f*h/8.f}, paint);

        // Draw four vertical lines at 1/8, 1/4, 3/4, 7/8.
        paint.setStrokeWidth(w/12.f);
        paint.setColor(/* orange */ 0xFFFFA500);
        surf->getCanvas()->drawLine({1.f*w/8.f, 0.f}, {1.f*h/8.f, h}, paint);
        paint.setColor(SK_ColorBLUE);
        surf->getCanvas()->drawLine({1.f*w/4.f, 0.f}, {1.f*h/4.f, h}, paint);
        paint.setColor(SK_ColorMAGENTA);
        surf->getCanvas()->drawLine({3.f*w/4.f, 0.f}, {3.f*h/4.f, h}, paint);
        paint.setColor(SK_ColorGREEN);
        surf->getCanvas()->drawLine({7.f*w/8.f, 0.f}, {7.f*h/8.f, h}, paint);

        auto img = surf->makeImageSnapshot();
        if (auto v = as_IB(img)->view(rContext)) {
            src = *v;
        }
    }
    if (!src) {
        return;
    }

    SkIRect srcRect = SkIRect::MakeSize(src.dimensions());
    if (subsetSrc) {
        srcRect = SkIRect::MakeXYWH(2.f*srcRect.width() /8.f,
                                    1.f*srcRect.height()/8.f,
                                    5.f*srcRect.width() /8.f,
                                    6.f*srcRect.height()/8.f);
    }
    int srcW = srcRect.width();
    int srcH = srcRect.height();
    // Each set of rects is drawn in one test area so they probably should not abut or overlap
    // to visualize the blurs separately.
    const std::vector<SkIRect> dstRectSets[] = {
            // encloses source bounds.
            {
                    srcRect.makeOutset(srcW/5, srcH/5)
            },

            // partial overlap from above/below.
            {
                    SkIRect::MakeXYWH(srcRect.x(), srcRect.y() + 3*srcH/4, srcW, srcH),
                    SkIRect::MakeXYWH(srcRect.x(), srcRect.y() - 3*srcH/4, srcW, srcH)
            },

            // adjacent to each side of src bounds.
            {
                    srcRect.makeOffset(    0,  srcH),
                    srcRect.makeOffset( srcW,     0),
                    srcRect.makeOffset(    0, -srcH),
                    srcRect.makeOffset(-srcW,     0),
            },

            // fully outside src bounds in one direction.
            {
                    SkIRect::MakeXYWH(-6.f*srcW/8.f, -7.f*srcH/8.f,  4.f*srcW/8.f, 20.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(-1.f*srcW/8.f, -7.f*srcH/8.f, 16.f*srcW/8.f,  2.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(10.f*srcW/8.f, -3.f*srcH/8.f,  4.f*srcW/8.f, 16.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(-7.f*srcW/8.f, 14.f*srcH/8.f, 18.f*srcW/8.f,  1.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
            },

            // outside of src bounds in both directions.
            {
                    SkIRect::MakeXYWH(-5.f*srcW/8.f, -5.f*srcH/8.f, 2.f*srcW/8.f, 2.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(-5.f*srcW/8.f, 12.f*srcH/8.f, 2.f*srcW/8.f, 2.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(12.f*srcW/8.f, -5.f*srcH/8.f, 2.f*srcW/8.f, 2.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
                    SkIRect::MakeXYWH(12.f*srcW/8.f, 12.f*srcH/8.f, 2.f*srcW/8.f, 2.f*srcH/8.f)
                            .makeOffset(srcRect.topLeft()),
            },
    };

    const auto& caps = *rContext->priv().caps();

    static constexpr SkScalar kPad = 10;
    SkVector trans = {kPad, kPad};

    rtc->clear(SK_PMColor4fWHITE);

    SkIRect testArea = srcRect;
    testArea.outset(testArea.width(), testArea.height());
    for (const auto& dstRectSet : dstRectSets) {
        for (int t = 0; t < kSkTileModeCount; ++t) {
            auto mode = static_cast<SkTileMode>(t);
            GrSamplerState sampler(SkTileModeToWrapMode(mode), GrSamplerState::Filter::kNearest);
            SkMatrix m = SkMatrix::Translate(trans.x() - testArea.x(), trans.y() - testArea.y());
            // Draw the src subset in the tile mode faded as a reference before drawing the blur
            // on top.
            {
                static constexpr float kAlpha = 0.2f;
                auto fp = GrTextureEffect::MakeSubset(src, kPremul_SkAlphaType, SkMatrix::I(),
                                                      sampler, SkRect::Make(srcRect), caps);
                fp = GrFragmentProcessor::ModulateRGBA(std::move(fp),
                                                       {kAlpha, kAlpha, kAlpha, kAlpha});
                GrPaint paint;
                paint.setColorFragmentProcessor(std::move(fp));
                rtc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, SkRect::Make(testArea));
            }
            // If we're in ref mode we will create a temp image that has the original image
            // tiled into it and then do a clamp blur with adjusted params that should produce
            // the same result as the original params.
            std::unique_ptr<GrSurfaceDrawContext> refSrc;
            SkIRect refRect;
            if (ref) {
                // Blow up testArea into a much larger rect so that our clamp blur will not
                // reach anywhere near the edge of our temp surface.
                refRect = testArea.makeOutset(testArea.width(), testArea.height());
                refSrc = GrSurfaceDrawContext::Make(rContext, GrColorType::kRGBA_8888, nullptr,
                                                    SkBackingFit::kApprox, refRect.size());
                refSrc->clear(SK_PMColor4fWHITE);
                // Setup an effect to put the original src rect at the correct logical place
                // in the temp where the temp's origin is at the top left of refRect.
                SkMatrix tm = SkMatrix::Translate(refRect.left(), refRect.top());
                auto fp = GrTextureEffect::MakeSubset(src, kPremul_SkAlphaType, tm, sampler,
                                                      SkRect::Make(srcRect), caps);
                GrPaint paint;
                paint.setColorFragmentProcessor(std::move(fp));
                refSrc->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                 SkRect::Make(refRect.size()));
            }
            // Do a blur for each dstRect in the set over our testArea-sized background.
            for (const auto& dstRect : dstRectSet) {
                // Setup the normal blur args.
                const SkScalar sigmaX = src.width()  / 10.f;
                const SkScalar sigmaY = src.height() / 10.f;
                auto blurSrc = src;
                auto blurMode = mode;
                auto blurDstRect = dstRect;
                auto blurSrcRect = srcRect;
                if (ref) {
                    // Move the dst rect to be relative to our temp surface.
                    blurDstRect = dstRect.makeOffset(-refRect.topLeft());
                    // Our new src is the entire temp containing the tiled original.
                    blurSrcRect = SkIRect::MakeSize(refRect.size());
                    // This shouldn't really matter because we should have made a large enough
                    // temp that the edges don't come into play. But this puts us on a simpler
                    // path through SkGpuBlurUtils.
                    blurMode = SkTileMode::kClamp;
                    blurSrc = refSrc->readSurfaceView();
                }
                // Blur using the rect and draw on top.
                if (auto blurView = blur(rContext, blurSrc, blurDstRect, blurSrcRect,
                                         sigmaX, sigmaY, blurMode)) {
                    auto fp = GrTextureEffect::Make(blurView, kPremul_SkAlphaType, SkMatrix::I(),
                                                    sampler, caps);
                    GrPaint paint;
                    // Compose against white (default paint color) and then replace the dst
                    // (SkBlendMode::kSrc).
                    fp = GrBlendFragmentProcessor::Make(std::move(fp), /*dst=*/nullptr,
                                                        SkBlendMode::kSrcOver);
                    paint.setColorFragmentProcessor(std::move(fp));
                    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                    rtc->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, m,
                                        SkRect::Make(dstRect), SkRect::Make(blurView.dimensions()));
                }
                // Show the outline of the dst rect. Mostly for kDecal but also allows visual
                // confirmation that the resulting blur is the right size and in the right place.
                {
                    GrPaint paint;
                    static constexpr float kAlpha = 0.6f;
                    paint.setColor4f({0, kAlpha, 0, kAlpha});
                    SkPaint stroke;
                    stroke.setStyle(SkPaint::kStroke_Style);
                    stroke.setStrokeWidth(1.f);
                    GrStyle style(stroke);
                    auto dstR = SkRect::Make(dstRect).makeOutset(0.5f, 0.5f);
                    rtc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, dstR, &style);
                }
            }
            // Show the rect that's being blurred.
            {
                GrPaint paint;
                static constexpr float kAlpha = 0.3f;
                paint.setColor4f({0, 0, 0, kAlpha});
                SkPaint stroke;
                stroke.setStyle(SkPaint::kStroke_Style);
                stroke.setStrokeWidth(1.f);
                GrStyle style(stroke);
                auto srcR = SkRect::Make(srcRect).makeOutset(0.5f, 0.5f);
                rtc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, srcR, &style);
            }
            trans.fX += testArea.width() + kPad;
        }
        trans.fX = kPad;
        trans.fY += testArea.height() + kPad;
    }
}

DEF_SIMPLE_GPU_GM(gpu_blur_utils, ctx, rtc, canvas, 765, 955) { run(ctx, rtc, false, false); }

DEF_SIMPLE_GPU_GM(gpu_blur_utils_ref, ctx, rtc, canvas, 765, 955) { run(ctx, rtc, false, true); }

DEF_SIMPLE_GPU_GM(gpu_blur_utils_subset_rect, ctx, rtc, canvas, 485, 730) {
    run(ctx, rtc, true, false);
}

DEF_SIMPLE_GPU_GM(gpu_blur_utils_subset_ref, ctx, rtc, canvas, 485, 730) {
    run(ctx, rtc, true, true);
}
