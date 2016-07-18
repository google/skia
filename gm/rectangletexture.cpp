/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrGpu.h"
#include "GrTest.h"
#include "gl/GrGLContext.h"
#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkImage.h"

namespace skiagm {
class RectangleTexture : public GM {
public:
    RectangleTexture() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("rectangle_texture");
    }

    SkISize onISize() override {
        return SkISize::Make(1035, 240);
    }

    void fillPixels(int width, int height, void *pixels) {
        SkBitmap bmp;
        bmp.setInfo(SkImageInfo::MakeN32(width, height, kOpaque_SkAlphaType), width * 4);
        bmp.setPixels(pixels);
        SkPaint paint;
        SkCanvas canvas(bmp);
        SkPoint pts[] = { {0, 0}, {0, SkIntToScalar(height)} };
        SkColor colors0[] = { 0xFF1060B0 , 0xFF102030 };
        paint.setShader(SkGradientShader::MakeLinear(pts, colors0, nullptr, 2,
                                                     SkShader::kClamp_TileMode));
        canvas.drawPaint(paint);

        SkColor colors1[] = { 0xFFA07010 , 0xFFA02080 };
        paint.setAntiAlias(true);
        paint.setShader(SkGradientShader::MakeLinear(pts, colors1, nullptr, 2,
                                                     SkShader::kClamp_TileMode));
        canvas.drawCircle(SkIntToScalar(width) / 2, SkIntToScalar(height) / 2,
                          SkIntToScalar(width + height) / 5, paint);
    }

    sk_sp<SkImage> createRectangleTextureImg(GrContext* context, int width, int height,
                                             void* pixels) {
        if (!context) {
            return nullptr;
        }
        GrGpu* gpu = context->getGpu();
        if (!gpu) {
            return nullptr;
        }
        const GrGLContext* glCtx = gpu->glContextForTesting();
        if (!glCtx) {
            return nullptr;
        }

        if (!(kGL_GrGLStandard == glCtx->standard() && glCtx->version() >= GR_GL_VER(3, 1)) &&
            !glCtx->hasExtension("GL_ARB_texture_rectangle")) {
            return nullptr;
        }

        // We will always create the GL texture as GL_RGBA, however the pixels uploaded may be
        // be RGBA or BGRA, depending on how SkPMColor was compiled.
        GrGLenum format;
        if (kSkia8888_GrPixelConfig == kBGRA_8888_GrPixelConfig) {
            format = GR_GL_BGRA;
        } else {
            SkASSERT(kSkia8888_GrPixelConfig == kRGBA_8888_GrPixelConfig);
            format = GR_GL_RGBA;
        }

        const GrGLInterface* gl = glCtx->interface();
// Useful for debugging whether errors result from use of RECTANGLE
// #define TARGET GR_GL_TEXTURE_2D
#define TARGET GR_GL_TEXTURE_RECTANGLE
        GrGLuint id = 0;
        GR_GL_CALL(gl, GenTextures(1, &id));
        GR_GL_CALL(gl, BindTexture(TARGET, id));
        GR_GL_CALL(gl, TexParameteri(TARGET, GR_GL_TEXTURE_MAG_FILTER,
                                     GR_GL_NEAREST));
        GR_GL_CALL(gl, TexParameteri(TARGET, GR_GL_TEXTURE_MIN_FILTER,
                                     GR_GL_NEAREST));
        GR_GL_CALL(gl, TexParameteri(TARGET, GR_GL_TEXTURE_WRAP_S,
                                     GR_GL_CLAMP_TO_EDGE));
        GR_GL_CALL(gl, TexParameteri(TARGET, GR_GL_TEXTURE_WRAP_T,
                                     GR_GL_CLAMP_TO_EDGE));
        GR_GL_CALL(gl, TexImage2D(TARGET, 0, GR_GL_RGBA, width, height, 0,
                                  format, GR_GL_UNSIGNED_BYTE, pixels));


        context->resetContext();
        GrGLTextureInfo info;
        info.fID = id;
        info.fTarget = TARGET;
        GrBackendTextureDesc desc;
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fOrigin = kTopLeft_GrSurfaceOrigin;
        desc.fTextureHandle = reinterpret_cast<GrBackendObject>(&info);
        if (sk_sp<SkImage> image = SkImage::MakeFromAdoptedTexture(context, desc)) {
            return image;
        }
        GR_GL_CALL(gl, DeleteTextures(1, &id));
        return nullptr;
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext *context = canvas->getGrContext();
        if (!context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        static const int kWidth = 50;
        static const int kHeight = 50;
        static const SkScalar kPad = 5.f;

        SkPMColor pixels[kWidth * kHeight];
        this->fillPixels(kWidth, kHeight, pixels);
        sk_sp<SkImage> rectImg(this->createRectangleTextureImg(context, kWidth, kHeight, pixels));

        if (!rectImg) {
            SkPaint paint;
            paint.setAntiAlias(true);
            static const char* kMsg = "Could not create rectangle texture image.";
            canvas->drawText(kMsg, strlen(kMsg), 10, 100, paint);
            return;
        }

        static const SkFilterQuality kQualities[] = {
            kNone_SkFilterQuality,
            kLow_SkFilterQuality,
            kMedium_SkFilterQuality,
            kHigh_SkFilterQuality,
        };

        static const SkScalar kScales[] = { 1.0f, 1.2f, 0.75f };

        canvas->translate(kPad, kPad);
        for (auto s : kScales) {
            canvas->save();
            canvas->scale(s, s);
            for (auto q : kQualities) {
                SkPaint plainPaint;
                plainPaint.setFilterQuality(q);
                canvas->drawImage(rectImg.get(), 0, 0, &plainPaint);
                canvas->translate(kWidth + kPad, 0);

                SkPaint clampPaint;
                clampPaint.setFilterQuality(q);
                clampPaint.setShader(rectImg->makeShader(SkShader::kClamp_TileMode,
                                                         SkShader::kClamp_TileMode));
                canvas->drawRect(SkRect::MakeWH(1.5f * kWidth, 1.5f * kHeight), clampPaint);
                canvas->translate(kWidth * 1.5f + kPad, 0);

                SkPaint repeatPaint;
                repeatPaint.setFilterQuality(q);
                repeatPaint.setShader(rectImg->makeShader(SkShader::kRepeat_TileMode,
                                                          SkShader::kMirror_TileMode));
                canvas->drawRect(SkRect::MakeWH(1.5f * kWidth, 1.5f * kHeight), repeatPaint);
                canvas->translate(1.5f * kWidth + kPad, 0);
            }
            canvas->restore();
            canvas->translate(0, kPad + 1.5f * kHeight * s);
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return new RectangleTexture;)
}

#endif
