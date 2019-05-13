/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTo.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"

class GrRenderTargetContext;

static sk_sp<SkColorFilter> yuv_to_rgb_colorfilter() {
    static const float kJPEGConversionMatrix[20] = {
        1.0f,  0.0f,       1.402f,    0.0f, -180.0f/255,
        1.0f, -0.344136f, -0.714136f, 0.0f,  136.0f/255,
        1.0f,  1.772f,     0.0f,      0.0f, -227.6f/255,
        0.0f,  0.0f,       0.0f,      1.0f,    0.0f
    };

    return SkColorFilters::Matrix(kJPEGConversionMatrix);
}

namespace skiagm {
class ImageFromYUVTextures : public GpuGM {
public:
    ImageFromYUVTextures() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("image_from_yuv_textures");
    }

    SkISize onISize() override {
        return SkISize::Make(kBmpSize + 2 * kPad, 390);
    }

    void onOnceBeforeDraw() override {
        // We create an RGB bitmap and then extract YUV bmps where the U and V bitmaps are
        // subsampled by 2 in both dimensions.
        SkPaint paint;
        constexpr SkColor kColors[] =
            { SK_ColorBLUE, SK_ColorYELLOW, SK_ColorGREEN, SK_ColorWHITE };
        paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(0,0), kBmpSize / 2.f, kColors,
                                                     nullptr, SK_ARRAY_COUNT(kColors),
                                                     SkTileMode::kMirror));
        SkBitmap rgbBmp;
        rgbBmp.allocN32Pixels(kBmpSize, kBmpSize, true);
        SkCanvas canvas(rgbBmp);
        canvas.drawPaint(paint);
        SkPMColor* rgbColors = static_cast<SkPMColor*>(rgbBmp.getPixels());

        SkImageInfo yinfo = SkImageInfo::MakeA8(kBmpSize, kBmpSize);
        fYUVBmps[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(kBmpSize / 2, kBmpSize / 2);
        fYUVBmps[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(kBmpSize / 2, kBmpSize / 2);
        fYUVBmps[2].allocPixels(vinfo);
        unsigned char* yPixels;
        signed char* uvPixels[2];
        yPixels = static_cast<unsigned char*>(fYUVBmps[0].getPixels());
        uvPixels[0] = static_cast<signed char*>(fYUVBmps[1].getPixels());
        uvPixels[1] = static_cast<signed char*>(fYUVBmps[2].getPixels());

        // Here we encode using the kJPEG_SkYUVColorSpace (i.e., full-swing Rec 601) even though
        // we will draw it with all the supported yuv color spaces when converted back to RGB
        for (int i = 0; i < kBmpSize * kBmpSize; ++i) {
            yPixels[i] = static_cast<unsigned char>(0.299f * SkGetPackedR32(rgbColors[i]) +
                                                    0.587f * SkGetPackedG32(rgbColors[i]) +
                                                    0.114f * SkGetPackedB32(rgbColors[i]));
        }
        for (int j = 0; j < kBmpSize / 2; ++j) {
            for (int i = 0; i < kBmpSize / 2; ++i) {
                // Average together 4 pixels of RGB.
                int rgb[] = { 0, 0, 0 };
                for (int y = 0; y < 2; ++y) {
                    for (int x = 0; x < 2; ++x) {
                        int rgbIndex = (2 * j + y) * kBmpSize + 2 * i + x;
                        rgb[0] += SkGetPackedR32(rgbColors[rgbIndex]);
                        rgb[1] += SkGetPackedG32(rgbColors[rgbIndex]);
                        rgb[2] += SkGetPackedB32(rgbColors[rgbIndex]);
                    }
                }
                for (int c = 0; c < 3; ++c) {
                    rgb[c] /= 4;
                }
                int uvIndex = j * kBmpSize / 2 + i;
                uvPixels[0][uvIndex] = static_cast<signed char>(
                    ((-38 * rgb[0] -  74 * rgb[1] + 112 * rgb[2] + 128) >> 8) + 128);
                uvPixels[1][uvIndex] = static_cast<signed char>(
                    ((112 * rgb[0] -  94 * rgb[1] -  18 * rgb[2] + 128) >> 8) + 128);
            }
        }
        fRGBImage = SkImage::MakeRasterCopy(SkPixmap(rgbBmp.info(), rgbColors, rgbBmp.rowBytes()));
    }

    void createYUVTextures(GrContext* context, GrBackendTexture yuvTextures[3]) {
        GrGpu* gpu = context->priv().getGpu();
        if (!gpu) {
            return;
        }

        for (int i = 0; i < 3; ++i) {
            SkASSERT(fYUVBmps[i].width() == SkToInt(fYUVBmps[i].rowBytes()));
            yuvTextures[i] = gpu->createTestingOnlyBackendTexture(fYUVBmps[i].width(),
                                                                  fYUVBmps[i].height(),
                                                                  kAlpha_8_SkColorType,
                                                                  GrMipMapped::kNo,
                                                                  GrRenderable::kNo,
                                                                  fYUVBmps[i].getPixels());
        }
        context->resetContext();
    }

    void createResultTexture(GrContext* context, int width, int height,
                             GrBackendTexture* resultTexture) {
        GrGpu* gpu = context->priv().getGpu();
        if (!gpu) {
            return;
        }

        *resultTexture = gpu->createTestingOnlyBackendTexture(
                width, height, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kYes);

        context->resetContext();
    }

    void deleteBackendTextures(GrContext* context, GrBackendTexture textures[], int n) {
        if (context->abandoned()) {
            return;
        }

        GrGpu* gpu = context->priv().getGpu();
        if (!gpu) {
            return;
        }

        context->flush();
        gpu->testingOnly_flushGpuAndSync();
        for (int i = 0; i < n; ++i) {
            if (textures[i].isValid()) {
                gpu->deleteTestingOnlyBackendTexture(textures[i]);
            }
        }

        context->resetContext();
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        // draw the original
        SkScalar yOffset = kPad;
        canvas->drawImage(fRGBImage.get(), kPad, yOffset);
        yOffset += kBmpSize + kPad;

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            GrBackendTexture yuvTextures[3];
            this->createYUVTextures(context, yuvTextures);
            auto image = SkImage::MakeFromYUVTexturesCopy(context,
                                                          static_cast<SkYUVColorSpace>(space),
                                                          yuvTextures,
                                                          kTopLeft_GrSurfaceOrigin);
            this->deleteBackendTextures(context, yuvTextures, 3);

            SkPaint paint;
            if (kIdentity_SkYUVColorSpace == space) {
                // The identity color space needs post-processing to appear correct
                paint.setColorFilter(yuv_to_rgb_colorfilter());
            }

            canvas->drawImage(image.get(), kPad, yOffset, &paint);
            yOffset += kBmpSize + kPad;
        }

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            GrBackendTexture yuvTextures[3];
            GrBackendTexture resultTexture;
            this->createYUVTextures(context, yuvTextures);
            this->createResultTexture(
                    context, yuvTextures[0].width(), yuvTextures[0].height(), &resultTexture);
            auto image = SkImage::MakeFromYUVTexturesCopyWithExternalBackend(
                                                          context,
                                                          static_cast<SkYUVColorSpace>(space),
                                                          yuvTextures,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          resultTexture);

            SkPaint paint;
            if (kIdentity_SkYUVColorSpace == space) {
                // The identity color space needs post-processing to appear correct
                paint.setColorFilter(yuv_to_rgb_colorfilter());
            }
            canvas->drawImage(image.get(), kPad, yOffset, &paint);
            yOffset += kBmpSize + kPad;

            GrBackendTexture texturesToDelete[4]{
                    yuvTextures[0],
                    yuvTextures[1],
                    yuvTextures[2],
                    resultTexture,
            };
            this->deleteBackendTextures(context, texturesToDelete, 4);
        }
     }

private:
    sk_sp<SkImage>  fRGBImage;
    SkBitmap        fYUVBmps[3];

    static constexpr SkScalar kPad = 10.0f;
    static constexpr int kBmpSize  = 32;

    typedef GM INHERITED;
};

DEF_GM(return new ImageFromYUVTextures;)
}
