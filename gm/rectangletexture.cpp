/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "gl/GrGLContext.h"
#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkImage.h"

namespace skiagm {
class RectangleTexture : public GpuGM {
public:
    RectangleTexture() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("rectangle_texture");
    }

    SkISize onISize() override { return SkISize::Make(1200, 500); }

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

        SkColor colors1[] = {0xFFA07010, 0xFFA02080};
        paint.setAntiAlias(true);
        paint.setShader(SkGradientShader::MakeLinear(pts, colors1, nullptr, 2,
                                                     SkShader::kClamp_TileMode));
        canvas.drawCircle(SkIntToScalar(width) / 2, SkIntToScalar(height) / 2,
                          SkIntToScalar(width + height) / 5, paint);
    }

    static const GrGLContext* GetGLContextIfSupported(GrContext* context) {
        GrGpu* gpu = context->priv().getGpu();
        if (!gpu) {
            return nullptr;
        }
        const GrGLContext* glCtx = gpu->glContextForTesting();
        if (!glCtx) {
            return nullptr;
        }

    #if 0 // TODO(bsalomon): use extensions on GLES?
        bool is_GL31 = glCtx->standard() == kGL_GrGLStandard
                    && glCtx->version()  >= GR_GL_VER(3, 1);
        if (!is_GL31
                && !glCtx->hasExtension("GL_ARB_texture_rectangle")
                && !glCtx->hasExtension("GL_ANGLE_texture_rectangle")) {
            return nullptr;
        }
    #else
        if (glCtx->standard() != kGL_GrGLStandard) {
            return nullptr;
        }
        if (glCtx->version() < GR_GL_VER(3,1)
                && !glCtx->hasExtension("GL_ARB_texture_rectangle")
                && !glCtx->hasExtension("GL_ANGLE_texture_rectangle")) {
            return nullptr;
        }
    #endif

        return glCtx;
    }

    sk_sp<SkImage> createRectangleTextureImg(GrContext* context, GrSurfaceOrigin origin, int width,
                                             int height, const uint32_t* pixels) {
        const GrGLContext* glCtx = GetGLContextIfSupported(context);
        if (!glCtx) {
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
        // static constexpr GrGLenum kTarget = GR_GL_TEXTURE_2D;
        static constexpr GrGLenum kTarget = GR_GL_TEXTURE_RECTANGLE;
        GrGLuint id = 0;
        GR_GL_CALL(gl, GenTextures(1, &id));
        GR_GL_CALL(gl, BindTexture(kTarget, id));
        GR_GL_CALL(gl, TexParameteri(kTarget, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
        GR_GL_CALL(gl, TexParameteri(kTarget, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
        GR_GL_CALL(gl, TexParameteri(kTarget, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
        GR_GL_CALL(gl, TexParameteri(kTarget, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));
        std::unique_ptr<uint32_t[]> tempPixels;
        if (origin == kBottomLeft_GrSurfaceOrigin) {
            tempPixels.reset(new uint32_t[width * height]);
            for (int y = 0; y < height; ++y) {
                std::copy_n(pixels + width * (height - y - 1), width, tempPixels.get() + width * y);
            }
            pixels = tempPixels.get();
        }
        GR_GL_CALL(gl, TexImage2D(kTarget, 0, GR_GL_RGBA, width, height, 0, format,
                                  GR_GL_UNSIGNED_BYTE, pixels));

        context->resetContext();
        GrGLTextureInfo info;
        info.fID = id;
        info.fTarget = kTarget;
        info.fFormat = GR_GL_RGBA8;

        GrBackendTexture rectangleTex(width, height, GrMipMapped::kNo, info);

        if (sk_sp<SkImage> image = SkImage::MakeFromAdoptedTexture(context, rectangleTex, origin,
                                                                   kRGBA_8888_SkColorType)) {
            return image;
        }
        GR_GL_CALL(gl, DeleteTextures(1, &id));
        return nullptr;
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (!GetGLContextIfSupported(context)) {
            *errorMsg = "this GM requires an OpenGL 3.1+ context";
            return DrawResult::kSkip;
        }

        constexpr int kWidth = 50;
        constexpr int kHeight = 50;
        constexpr SkScalar kPad = 5.f;

        SkPMColor pixels[kWidth * kHeight];
        this->fillPixels(kWidth, kHeight, pixels);

        sk_sp<SkImage> rectImgs[] = {
                this->createRectangleTextureImg(context, kTopLeft_GrSurfaceOrigin, kWidth, kHeight,
                                                pixels),
                this->createRectangleTextureImg(context, kBottomLeft_GrSurfaceOrigin, kWidth,
                                                kHeight, pixels),
        };
        SkASSERT(SkToBool(rectImgs[0]) == SkToBool(rectImgs[1]));
        if (!rectImgs[0]) {
            *errorMsg = "Could not create rectangle texture image.";
            return DrawResult::kFail;
        }

        constexpr SkFilterQuality kQualities[] = {
                kNone_SkFilterQuality,
                kLow_SkFilterQuality,
                kMedium_SkFilterQuality,
                kHigh_SkFilterQuality,
        };

        constexpr SkScalar kScales[] = {1.0f, 1.2f, 0.75f};

        canvas->translate(kPad, kPad);
        for (size_t i = 0; i < SK_ARRAY_COUNT(rectImgs); ++i) {
            for (auto s : kScales) {
                canvas->save();
                canvas->scale(s, s);
                for (auto q : kQualities) {
                    // drawImage
                    SkPaint plainPaint;
                    plainPaint.setFilterQuality(q);
                    canvas->drawImage(rectImgs[i], 0, 0, &plainPaint);
                    canvas->translate(kWidth + kPad, 0);

                    // clamp/clamp shader
                    SkPaint clampPaint;
                    clampPaint.setFilterQuality(q);
                    clampPaint.setShader(rectImgs[i]->makeShader());
                    canvas->drawRect(SkRect::MakeWH(1.5f * kWidth, 1.5f * kHeight), clampPaint);
                    canvas->translate(kWidth * 1.5f + kPad, 0);

                    // repeat/mirror shader
                    SkPaint repeatPaint;
                    repeatPaint.setFilterQuality(q);
                    repeatPaint.setShader(rectImgs[i]->makeShader(SkShader::kRepeat_TileMode,
                                                                  SkShader::kMirror_TileMode));
                    canvas->drawRect(SkRect::MakeWH(1.5f * kWidth, 1.5f * kHeight), repeatPaint);
                    canvas->translate(1.5f * kWidth + kPad, 0);

                    // drawImageRect with kStrict
                    auto srcRect = SkRect::MakeXYWH(.25f * rectImgs[i]->width(),
                                                    .25f * rectImgs[i]->height(),
                                                    .50f * rectImgs[i]->width(),
                                                    .50f * rectImgs[i]->height());
                    auto dstRect = SkRect::MakeXYWH(0, 0,
                                                    .50f * rectImgs[i]->width(),
                                                    .50f * rectImgs[i]->height());
                    canvas->drawImageRect(rectImgs[i], srcRect, dstRect, &plainPaint,
                                          SkCanvas::kStrict_SrcRectConstraint);
                    canvas->translate(kWidth * .5f + kPad, 0);
                }
                canvas->restore();
                canvas->translate(0, kPad + 1.5f * kHeight * s);
            }
        }
        return DrawResult::kOk;
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return new RectangleTexture;)
}
