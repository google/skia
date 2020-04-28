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
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrLumaColorFilterEffect.h"
#include "src/image/SkImage_Base.h"

DEF_SIMPLE_GPU_GM(gpu_blur_utils, ctx, rtc, canvas, 620, 155) {
    auto srcII = SkImageInfo::Make(50, 50, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kYes, srcII);
    GrSurfaceProxyView src;
    if (surf) {
        SkScalar w = surf->width();
        SkScalar h = surf->height();
        SkPoint pts[] = {{0, 0}, {w, h}};
        SkColor colors[] = {SK_ColorGREEN, SK_ColorBLUE};
        auto gradient = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
        SkPaint paint;
        paint.setShader(std::move(gradient));
        surf->getCanvas()->drawPaint(paint);
        paint.reset();
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);
        surf->getCanvas()->drawCircle({w / 2, h / 2}, std::sqrt(w * w + h * h) / 10.f, paint);
        paint.setStrokeWidth(h / 10.f);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorMAGENTA);
        surf->getCanvas()->drawLine({0.f, 3.f * h / 4.f}, {w, 3.f * h / 4.f}, paint);
        paint.setColor(SK_ColorCYAN);
        paint.setStrokeWidth(w / 15.f);
        surf->getCanvas()->drawLine({7.f * w / 8.f, 0.f}, {7.f * h / 8.f, h}, paint);

        auto img = surf->makeImageSnapshot();
        if (auto v = as_IB(img)->view(ctx)) {
            src = *v;
        }
    }
    if (!src) {
        return;
    }
    auto blur = [&](SkIRect dstB, SkTileMode mode) -> GrSurfaceProxyView {
        static constexpr SkScalar kSigma = 5;
        auto srcB = SkIRect::MakeSize(src.dimensions());
        auto resultRTC =
                SkGpuBlurUtils::GaussianBlur(ctx, src, GrColorType::kRGBA_8888, kPremul_SkAlphaType,
                                             nullptr, dstB, srcB, kSigma, kSigma, mode);
        if (!resultRTC) {
            return {};
        }
        return resultRTC->readSurfaceView();
    };
    SkIRect testArea = SkIRect::MakeSize(src.dimensions());
    testArea.outset(testArea.width(), testArea.height());

    SkVector trans = {};
    rtc->clear(SK_PMColor4fWHITE);
    const SkIRect dstRects[] = {
            SkIRect::MakeSize(src.dimensions()).makeOutset(20.f, 10.f),
    };

    for (const auto& r : dstRects) {
        for (int t = 0; t < kSkTileModeCount; ++t) {
            auto mode = static_cast<SkTileMode>(t);
            GrSamplerState sampler(SkTileModeToWrapMode(mode), GrSamplerState::Filter::kNearest);
            SkMatrix m = SkMatrix::MakeTrans(trans.x() - testArea.x(), trans.y() - testArea.y());
            // Draw the image in repeat mode with a luma filter to illustrate the tile mode.
            {
                auto fp = GrTextureEffect::Make(src, kPremul_SkAlphaType, SkMatrix::I(), sampler,
                                                *ctx->priv().caps());
                GrPaint paint;
                paint.addColorFragmentProcessor(std::move(fp));
                paint.addColorFragmentProcessor(GrLumaColorFilterEffect::Make(
                        GrLumaColorFilterEffect::OutputMode::kLumaAsRGB));
                rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, m, SkRect::Make(testArea));
            }
            // Slightly colorize the center where the original image actually lies.
            {
                auto fp = GrTextureEffect::Make(src, kPremul_SkAlphaType, SkMatrix::I(),
                                                GrSamplerState::Filter::kNearest);
                GrPaint paint;
                paint.addColorFragmentProcessor(std::move(fp));
                static constexpr float kFade = 0.3f;
                paint.setColor4f({kFade, kFade, kFade, kFade});
                rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, m,
                              SkRect::Make(src.dimensions()));
            }
            // Blur using the rect and draw on top.
            auto blurView = blur(r, mode);
            {
                auto fp = GrTextureEffect::Make(blurView, kPremul_SkAlphaType, SkMatrix::I(),
                                                sampler, *ctx->priv().caps());
                GrPaint paint;
                // Compose against white (default paint color) and then replace the dst (SkBlendMode::kSrc).
                fp = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(fp),
                                                                       SkBlendMode::kSrcOver);
                paint.addColorFragmentProcessor(std::move(fp));
                paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                rtc->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, m, SkRect::Make(r),
                                    SkRect::Make(blurView.dimensions()));
            }
            trans.fX += testArea.width() + 5;
        }
        trans.fY += testArea.height() + 5;
    }
}
