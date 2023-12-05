/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GL backend.

#include "gm/gm.h"

#ifdef SK_GL
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

#include <algorithm>
#include <cstdint>
#include <memory>

namespace skiagm {
class RectangleTexture : public GM {
public:
    RectangleTexture() {
        this->setBGColor(0xFFFFFFFF);
    }

private:
    enum class ImageType {
        kGradientCircle,
        k2x2
    };

    SkString getName() const override { return SkString("rectangle_texture"); }

    SkISize getISize() override { return SkISize::Make(1180, 710); }

    SkBitmap makeImagePixels(int size, ImageType type) {
        auto ii = SkImageInfo::Make(size, size, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        switch (type) {
            case ImageType::kGradientCircle: {
                SkBitmap bmp;
                bmp.allocPixels(ii);
                SkPaint paint;
                SkCanvas canvas(bmp);
                SkPoint pts[] = {{0, 0}, {0, SkIntToScalar(size)}};
                SkColor colors0[] = {0xFF1060B0, 0xFF102030};
                paint.setShader(
                        SkGradientShader::MakeLinear(pts, colors0, nullptr, 2, SkTileMode::kClamp));
                canvas.drawPaint(paint);
                SkColor colors1[] = {0xFFA07010, 0xFFA02080};
                paint.setAntiAlias(true);
                paint.setShader(
                        SkGradientShader::MakeLinear(pts, colors1, nullptr, 2, SkTileMode::kClamp));
                canvas.drawCircle(size/2.f, size/2.f, 2.f*size/5, paint);
                return bmp;
            }
            case ImageType::k2x2: {
                SkBitmap bmp;
                bmp.allocPixels(ii);
                *bmp.getAddr32(0, 0) = 0xFF0000FF;
                *bmp.getAddr32(1, 0) = 0xFF00FF00;
                *bmp.getAddr32(0, 1) = 0xFFFF0000;
                *bmp.getAddr32(1, 1) = 0xFFFFFFFF;
                return bmp;
            }
        }
        SkUNREACHABLE;
    }

    sk_sp<SkImage> createRectangleTextureImg(GrDirectContext* dContext, GrSurfaceOrigin origin,
                                             const SkBitmap content) {
        SkASSERT(content.colorType() == kRGBA_8888_SkColorType);
        auto format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_RECTANGLE);
        auto bet = dContext->createBackendTexture(content.width(),
                                                  content.height(),
                                                  format,
                                                  skgpu::Mipmapped::kNo,
                                                  GrRenderable::kNo,
                                                  GrProtected::kNo,
                                                  /*label=*/"CreateRectangleTextureImage");
        if (!bet.isValid()) {
            return nullptr;
        }
        if (!dContext->updateBackendTexture(bet, content.pixmap(), origin, nullptr, nullptr)) {
            dContext->deleteBackendTexture(bet);
        }
        return SkImages::AdoptTextureFrom(dContext, bet, origin, kRGBA_8888_SkColorType);
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg, GraphiteTestContext*) override {
        auto context = GrAsDirectContext(canvas->recordingContext());
        if (!context || context->abandoned()) {
            return DrawResult::kSkip;
        }

        if (context->backend() != GrBackendApi::kOpenGL_GrBackend ||
            !static_cast<const GrGLCaps*>(context->priv().caps())->rectangleTextureSupport()) {
            *errorMsg = "This GM requires an OpenGL context that supports texture rectangles.";
            return DrawResult::kSkip;
        }

        auto gradCircle = this->makeImagePixels(50, ImageType::kGradientCircle);

        fGradImgs[0] = this->createRectangleTextureImg(context, kTopLeft_GrSurfaceOrigin,
                                                       gradCircle);
        fGradImgs[1] = this->createRectangleTextureImg(context, kBottomLeft_GrSurfaceOrigin,
                                                       gradCircle);
        SkASSERT(SkToBool(fGradImgs[0]) == SkToBool(fGradImgs[1]));
        if (!fGradImgs[0]) {
            *errorMsg = "Could not create gradient rectangle texture images.";
            return DrawResult::kFail;
        }

        fSmallImg = this->createRectangleTextureImg(context, kTopLeft_GrSurfaceOrigin,
                                                    this->makeImagePixels(2, ImageType::k2x2));
        if (!fSmallImg) {
            *errorMsg = "Could not create 2x2 rectangle texture image.";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fGradImgs[0] = fGradImgs[1] = nullptr;
        fSmallImg = nullptr;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkASSERT(fGradImgs[0] && fGradImgs[1] && fSmallImg);

        static constexpr SkScalar kPad = 5.f;

        const SkSamplingOptions kSamplings[] = {
            SkSamplingOptions(SkFilterMode::kNearest),
            SkSamplingOptions(SkFilterMode::kLinear),
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
            SkSamplingOptions(SkCubicResampler::Mitchell()),
        };

        constexpr SkScalar kScales[] = {1.0f, 1.2f, 0.75f};

        canvas->translate(kPad, kPad);
        for (size_t i = 0; i < kNumGradImages; ++i) {
            auto img = fGradImgs[i];
            int w = img->width();
            int h = img->height();
            for (auto scale : kScales) {
                canvas->save();
                canvas->scale(scale, scale);
                for (auto s : kSamplings) {
                    // drawImage
                    canvas->drawImage(img, 0, 0, s);
                    canvas->translate(w + kPad, 0);

                    // clamp/clamp shader
                    SkPaint clampPaint;
                    clampPaint.setShader(fGradImgs[i]->makeShader(s));
                    canvas->drawRect(SkRect::MakeWH(1.5f*w, 1.5f*h), clampPaint);
                    canvas->translate(1.5f*w + kPad, 0);

                    // repeat/mirror shader
                    SkPaint repeatPaint;
                    repeatPaint.setShader(fGradImgs[i]->makeShader(SkTileMode::kRepeat,
                                                                   SkTileMode::kMirror,
                                                                   s));
                    canvas->drawRect(SkRect::MakeWH(1.5f*w, 1.5f*h), repeatPaint);
                    canvas->translate(1.5f*w + kPad, 0);

                    // drawImageRect with kStrict
                    auto srcRect = SkRect::MakeXYWH(.25f*w, .25f*h, .50f*w, .50f*h);
                    auto dstRect = SkRect::MakeXYWH(      0,     0, .50f*w, .50f*h);
                    canvas->drawImageRect(fGradImgs[i], srcRect, dstRect, s, nullptr,
                                          SkCanvas::kStrict_SrcRectConstraint);
                    canvas->translate(.5f*w + kPad, 0);
                }
                canvas->restore();
                canvas->translate(0, kPad + 1.5f*h*scale);
            }
        }

        static constexpr SkScalar kOutset = 25.f;
        canvas->translate(kOutset, kOutset);
        auto dstRect = SkRect::Make(fSmallImg->dimensions()).makeOutset(kOutset, kOutset);

        const SkSamplingOptions gSamplings[] = {
            SkSamplingOptions(SkFilterMode::kNearest),
            SkSamplingOptions(SkFilterMode::kLinear),
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
            SkSamplingOptions(SkCubicResampler::Mitchell()),
        };

        for (const auto& sampling : gSamplings) {
            if (!sampling.useCubic && sampling.mipmap != SkMipmapMode::kNone) {
                // Medium is the same as Low for upscaling.
                continue;
            }
            canvas->save();
            for (int ty = 0; ty < kSkTileModeCount; ++ty) {
                canvas->save();
                for (int tx = 0; tx < kSkTileModeCount; ++tx) {
                    SkMatrix lm;
                    lm.setRotate(45.f, 1, 1);
                    lm.postScale(6.5f, 6.5f);
                    SkPaint paint;
                    paint.setShader(fSmallImg->makeShader(static_cast<SkTileMode>(tx),
                                                          static_cast<SkTileMode>(ty),
                                                          sampling,
                                                          lm));
                    canvas->drawRect(dstRect, paint);
                    canvas->translate(dstRect.width() + kPad, 0);
                }
                canvas->restore();
                canvas->translate(0, dstRect.height() + kPad);
            }
            canvas->restore();
            canvas->translate((dstRect.width() + kPad)*kSkTileModeCount, 0);
        }

        return DrawResult::kOk;
    }

private:
    static const int kNumGradImages = 2;

    sk_sp<SkImage> fGradImgs[kNumGradImages];
    sk_sp<SkImage> fSmallImg;

    using INHERITED = GM;
};

DEF_GM(return new RectangleTexture;)
}  // namespace skiagm
#endif
