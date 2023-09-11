/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkColorSpace.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/ganesh/GrBlurUtils.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/image/SkImage_Base.h"

namespace {

static GrSurfaceProxyView blur(GrRecordingContext* ctx,
                               GrSurfaceProxyView src,
                               SkIRect dstB,
                               SkIRect srcB,
                               float sigmaX,
                               float sigmaY,
                               SkTileMode mode) {
    auto resultSDC = GrBlurUtils::GaussianBlur(ctx,
                                                  src,
                                                  GrColorType::kRGBA_8888,
                                                  kPremul_SkAlphaType,
                                                  nullptr,
                                                  dstB,
                                                  srcB,
                                                  sigmaX,
                                                  sigmaY,
                                                  mode);
    if (!resultSDC) {
        return {};
    }
    return resultSDC->readSurfaceView();
};

// Performs tiling first of the src into dst bounds with a surrounding skirt so the blur can use
// clamp. Does repeated blurs rather than invoking downsampling.
static GrSurfaceProxyView slow_blur(GrRecordingContext* rContext,
                                    GrSurfaceProxyView src,
                                    SkIRect dstB,
                                    SkIRect srcB,
                                    float sigmaX,
                                    float sigmaY,
                                    SkTileMode mode) {
    auto tileInto = [rContext](GrSurfaceProxyView src,
                               SkIRect srcTileRect,
                               SkISize resultSize,
                               SkIPoint offset,
                               SkTileMode mode) {
        GrImageInfo info(GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr, resultSize);
        auto sfc = rContext->priv().makeSFC(info, /*label=*/{});
        if (!sfc) {
            return GrSurfaceProxyView{};
        }
        GrSamplerState sampler(SkTileModeToWrapMode(mode), SkFilterMode::kNearest);
        auto fp = GrTextureEffect::MakeSubset(src,
                                              kPremul_SkAlphaType,
                                              SkMatrix::Translate(-offset.x(), -offset.y()),
                                              sampler,
                                              SkRect::Make(srcTileRect),
                                              *rContext->priv().caps());
        sfc->fillWithFP(std::move(fp));
        return sfc->readSurfaceView();
    };

    SkIPoint outset = {skgpu::BlurSigmaRadius(sigmaX), skgpu::BlurSigmaRadius(sigmaY)};
    SkISize size = {dstB.width() + 2*outset.x(), dstB.height() + 2*outset.y()};
    src = tileInto(std::move(src), srcB, size, outset - dstB.topLeft(), mode);
    if (!src) {
        return {};
    }
    dstB = SkIRect::MakePtSize(outset, dstB.size());

    while (sigmaX || sigmaY) {
        float stepX = sigmaX;
        if (stepX > skgpu::kMaxLinearBlurSigma) {
            stepX = skgpu::kMaxLinearBlurSigma;
            // A blur of sigma1 followed by a blur of sigma2 is equiv. to a single blur of
            // sqrt(sigma1^2 + sigma2^2).
            sigmaX = sqrt(sigmaX*sigmaX - skgpu::kMaxLinearBlurSigma*skgpu::kMaxLinearBlurSigma);
        } else {
            sigmaX = 0.f;
        }
        float stepY = sigmaY;
        if (stepY > skgpu::kMaxLinearBlurSigma) {
            stepY = skgpu::kMaxLinearBlurSigma;
            sigmaY = sqrt(sigmaY*sigmaY- skgpu::kMaxLinearBlurSigma*skgpu::kMaxLinearBlurSigma);
        } else {
            sigmaY = 0.f;
        }
        auto bounds = SkIRect::MakeSize(src.dimensions());
        auto sdc = GrBlurUtils::GaussianBlur(rContext,
                                                std::move(src),
                                                GrColorType::kRGBA_8888,
                                                kPremul_SkAlphaType,
                                                nullptr,
                                                bounds,
                                                bounds,
                                                stepX,
                                                stepY,
                                                SkTileMode::kClamp);
        if (!sdc) {
            return {};
        }
        src = sdc->readSurfaceView();
    }
    // We have o use the original mode here because we may have only blurred in X or Y and then
    // the other dimension was not expanded.
    auto srcRect = SkIRect::MakeSize(src.dimensions());
    return tileInto(std::move(src), srcRect, dstB.size(), -outset, SkTileMode::kClamp);
};

// Makes a src texture for as a source for blurs. If 'contentArea' then the content will
// be in that rect, the 1-pixel surrounding border will be transparent black, and red outside of
// that. Otherwise, the content fills the dimensions.
GrSurfaceProxyView make_src_image(GrRecordingContext* rContext,
                                  SkISize dimensions,
                                  const SkIRect* contentArea = nullptr) {
    auto srcII = SkImageInfo::Make(dimensions, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, srcII);
    if (!surf) {
        return {};
    }

    float w, h;
    if (contentArea) {
        surf->getCanvas()->clear(SK_ColorRED);
        surf->getCanvas()->clipIRect(contentArea->makeOutset(1, 1));
        surf->getCanvas()->clear(SK_ColorTRANSPARENT);
        surf->getCanvas()->clipIRect(*contentArea);
        surf->getCanvas()->translate(contentArea->top(), contentArea->left());
        w = contentArea->width();
        h = contentArea->height();
    } else {
        w = dimensions.width();
        h = dimensions.height();
    }

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
    auto [src, ct] = skgpu::ganesh::AsView(rContext, img, skgpu::Mipmapped::kNo);
    return src;
}

} // namespace

namespace skiagm {

static GM::DrawResult run(GrRecordingContext* rContext, SkCanvas* canvas,  SkString* errorMsg,
                          bool subsetSrc, bool ref) {
    GrSurfaceProxyView src = make_src_image(rContext, {60, 60});
    if (!src) {
        *errorMsg = "Failed to create source image";
        return DrawResult::kSkip;
    }

    auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
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

    sdc->clear(SK_PMColor4fWHITE);

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
                sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, SkRect::Make(testArea));
            }
            // Do a blur for each dstRect in the set over our testArea-sized background.
            for (const auto& dstRect : dstRectSet) {
                const SkScalar sigmaX = src.width()  / 10.f;
                const SkScalar sigmaY = src.height() / 10.f;
                auto blurFn = ref ? slow_blur : blur;
                // Blur using the rect and draw on top.
                if (auto blurView = blurFn(rContext,
                                           src,
                                           dstRect,
                                           srcRect,
                                           sigmaX,
                                           sigmaY,
                                           mode)) {
                    auto fp = GrTextureEffect::Make(blurView,
                                                    kPremul_SkAlphaType,
                                                    SkMatrix::I(),
                                                    sampler,
                                                    caps);
                    // Compose against white (default paint color)
                    fp = GrBlendFragmentProcessor::Make<SkBlendMode::kSrcOver>(std::move(fp),
                                                                               /*dst=*/nullptr);
                    GrPaint paint;
                    // Compose against white (default paint color) and then replace the dst
                    // (SkBlendMode::kSrc).
                    fp = GrBlendFragmentProcessor::Make<SkBlendMode::kSrcOver>(std::move(fp),
                                                                               /*dst=*/nullptr);
                    paint.setColorFragmentProcessor(std::move(fp));
                    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                    sdc->fillRectToRect(nullptr,
                                        std::move(paint),
                                        GrAA::kNo,
                                        m,
                                        SkRect::Make(dstRect),
                                        SkRect::Make(blurView.dimensions()));
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
                    sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, dstR, &style);
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
                sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, m, srcR, &style);
            }
            trans.fX += testArea.width() + kPad;
        }
        trans.fX = kPad;
        trans.fY += testArea.height() + kPad;
    }

    return DrawResult::kOk;
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(gpu_blur_utils, rContext, canvas, errorMsg, 765, 955) {
    return run(rContext, canvas, errorMsg, false, false);
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(gpu_blur_utils_ref, rContext, canvas, errorMsg, 765, 955) {
    return run(rContext, canvas, errorMsg, false, true);
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(gpu_blur_utils_subset_rect, rContext, canvas, errorMsg, 485, 730) {
    return run(rContext, canvas, errorMsg, true, false);
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(gpu_blur_utils_subset_ref, rContext, canvas, errorMsg, 485, 730) {
    return run(rContext, canvas, errorMsg, true, true);
}

// Because of the way blur sigmas concat (sigTotal = sqrt(sig1^2 + sig2^2) generating these images
// for very large sigmas is incredibly slow. This can be enabled while working on the blur code to
// check results.
static bool constexpr kShowSlowRefImages = false;

static DrawResult do_very_large_blur_gm(GrRecordingContext* rContext,
                                        SkCanvas* canvas,
                                        SkString* errorMsg,
                                        GrSurfaceProxyView src,
                                        SkIRect srcB) {
    auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    // Clear to a color other than gray to contrast with test image.
    sdc->clear(SkColor4f{0.3f, 0.4f, 0.2f, 1});

    int x = 10;
    int y = 10;
    for (auto blurDirs : {0b01, 0b10, 0b11}) {
        for (int t = 0; t < kSkTileModeCount; ++t) {
            auto tm = static_cast<SkTileMode>(t);
            auto dstB = srcB.makeOutset(30, 30);
            for (float sigma : {0.f, 5.f, 25.f, 80.f}) {
                std::vector<decltype(blur)*> blurs;
                blurs.push_back(blur);
                if (kShowSlowRefImages) {
                    blurs.push_back(slow_blur);
                }
                for (auto b : blurs) {
                    float sigX = sigma*((blurDirs & 0b01) >> 0);
                    float sigY = sigma*((blurDirs & 0b10) >> 1);
                    GrSurfaceProxyView result = b(rContext, src, dstB, srcB, sigX, sigY, tm);
                    auto dstRect = SkIRect::MakeSize(dstB.size()).makeOffset(x, y);
                    // Draw a rect to show where the result should be so it's obvious if it's
                    // missing.
                    GrPaint paint;
                    paint.setColor4f(b == blur ? SkPMColor4f{0, 0, 1, 1} : SkPMColor4f{1, 0, 0, 1});
                    sdc->drawRect(nullptr,
                                  std::move(paint),
                                  GrAA::kNo,
                                  SkMatrix::I(),
                                  SkRect::Make(dstRect).makeOutset(0.5, 0.5),
                                  &GrStyle::SimpleHairline());
                    if (result) {
                        std::unique_ptr<GrFragmentProcessor> fp =
                                GrTextureEffect::Make(std::move(result), kPremul_SkAlphaType);
                        fp = GrBlendFragmentProcessor::Make<SkBlendMode::kSrcOver>(std::move(fp),
                                                                                   /*dst=*/nullptr);
                        sdc->fillRectToRectWithFP(SkIRect::MakeSize(dstB.size()),
                                                  dstRect,
                                                  std::move(fp));
                    }
                    x += dstB.width() + 10;
                }
            }
            x = 10;
            y += dstB.height() + 10;
        }
    }

    return DrawResult::kOk;
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(very_large_sigma_gpu_blur, rContext, canvas, errorMsg, 350, 1030) {
    auto src = make_src_image(rContext, {15, 15});
    auto srcB = SkIRect::MakeSize(src.dimensions());
    return do_very_large_blur_gm(rContext, canvas, errorMsg, std::move(src), srcB);
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(very_large_sigma_gpu_blur_subset,
                           rContext,
                           canvas,
                           errorMsg,
                           350, 1030) {
    auto srcB = SkIRect::MakeXYWH(2, 2, 15, 15);
    SkISize imageSize = SkISize{srcB.width() + 4, srcB.height() + 4};
    auto src = make_src_image(rContext, imageSize, &srcB);
    return do_very_large_blur_gm(rContext, canvas, errorMsg, std::move(src), srcB);
}

DEF_SIMPLE_GPU_GM_CAN_FAIL(very_large_sigma_gpu_blur_subset_transparent_border,
                           rContext,
                           canvas,
                           errorMsg,
                           355, 1055) {
    auto srcB = SkIRect::MakeXYWH(3, 3, 15, 15);
    SkISize imageSize = SkISize{srcB.width() + 4, srcB.height() + 4};
    auto src = make_src_image(rContext, imageSize, &srcB);
    return do_very_large_blur_gm(rContext, canvas, errorMsg, std::move(src), srcB.makeOutset(1, 1));
}

} // namespace skiagm
