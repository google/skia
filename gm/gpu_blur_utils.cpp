/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/effects/SkGradientShader.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/image/SkImage_Base.h"

DEF_SIMPLE_GPU_GM(gpu_blur_utils, ctx, rtc, canvas, 985, 740) {
    auto srcII = SkImageInfo::Make(80, 80, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kYes, srcII);
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
        if (auto v = as_IB(img)->view(ctx)) {
            src = *v;
        }
    }
    if (!src) {
        return;
    }
    auto blur = [&](SkIRect dstB, SkIRect srcB, SkTileMode mode) -> GrSurfaceProxyView {
        const SkScalar sigmaX = src.width() /10.f;
        const SkScalar sigmaY = src.height()/10.f;
        auto resultRTC =
                SkGpuBlurUtils::GaussianBlur(ctx, src, GrColorType::kRGBA_8888, kPremul_SkAlphaType,
                                             nullptr, dstB, srcB, sigmaX, sigmaY, mode);
        if (!resultRTC) {
            return {};
        }
        return resultRTC->readSurfaceView();
    };

    SkVector trans = {};
    rtc->clear(SK_PMColor4fWHITE);
    const SkIRect dstRects[] = {
            SkIRect::MakeSize(src.dimensions()).makeOutset(src.width()/5, src.height()/5),
            SkIRect::MakeXYWH(0, 3*src.height()/4, src.width(), src.height()),
            SkIRect::MakeXYWH(0, src.height(), src.width(), src.height())
    };

    const SkIRect srcRects[] = {
            SkIRect::MakeSize(src.dimensions()),
    };

    const auto& caps = *ctx->priv().caps();
    static constexpr SkScalar kPad = 5;

    for (const auto& srcRect : srcRects) {
        SkIRect testArea = SkIRect::MakeSize(srcRect.size());
        testArea.outset(testArea.width(), testArea.height());
        for (const auto& dstRect : dstRects) {
            for (int t = 0; t < kSkTileModeCount; ++t) {
                auto mode = static_cast<SkTileMode>(t);
                GrSamplerState sampler(SkTileModeToWrapMode(mode),
                                       GrSamplerState::Filter::kNearest);
                SkMatrix m = SkMatrix::MakeTrans(trans.x() - testArea.x(),
                                                 trans.y() - testArea.y());
                // Draw the src subset in the tile mode faded as a reference before drawing the blur
                // on top
                {
                    auto fp = GrTextureEffect::MakeSubset(src, kPremul_SkAlphaType, SkMatrix::I(),
                                                          sampler, SkRect::Make(srcRect), caps);
                    GrPaint paint;
                    paint.addColorFragmentProcessor(std::move(fp));
                    static constexpr float kAlpha = 0.2f;
                    paint.setColor4f({kAlpha, kAlpha, kAlpha, kAlpha});
                    rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, m,
                                  SkRect::Make(testArea));
                }
                // Blur using the rect and draw on top.
                if (auto blurView = blur(dstRect, srcRect, mode)) {
                    auto fp = GrTextureEffect::Make(blurView, kPremul_SkAlphaType, SkMatrix::I(),
                                                    sampler, caps);
                    GrPaint paint;
                    // Compose against white (default paint color) and then replace the dst
                    // (SkBlendMode::kSrc).
                    fp = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(fp),
                                                                           SkBlendMode::kSrcOver);
                    paint.addColorFragmentProcessor(std::move(fp));
                    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                    rtc->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, m,
                                        SkRect::Make(dstRect), SkRect::Make(blurView.dimensions()));
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
                    rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, m, srcR, &style);
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
                    rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, m, dstR, &style);
                }
                trans.fX += testArea.width() + kPad;
            }
            trans.fX = 0;
            trans.fY += testArea.height() + kPad;
        }
    }
}
