/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrTest.h"
#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkImage.h"

namespace skiagm {
class ImageFromYUVTextures : public GM {
public:
    ImageFromYUVTextures() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("image_from_yuv_textures");
    }

    SkISize onISize() override {
        return SkISize::Make(50, 175);
    }

    void onOnceBeforeDraw() override {
        // We create an RGB bitmap and then extract YUV bmps where the U and V bitmaps are
        // subsampled by 2 in both dimensions.
        SkPaint paint;
        constexpr SkColor kColors[] =
            { SK_ColorBLUE, SK_ColorYELLOW, SK_ColorGREEN, SK_ColorWHITE };
        paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(0,0), kBmpSize / 2.f, kColors,
                                                     nullptr, SK_ARRAY_COUNT(kColors),
                                                     SkShader::kMirror_TileMode));
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

        // Here we encode using the NTC encoding (even though we will draw it with all the supported
        // yuv color spaces when converted back to RGB)
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
        GrGpu* gpu = context->contextPriv().getGpu();
        if (!gpu) {
            return;
        }

        for (int i = 0; i < 3; ++i) {
            SkASSERT(fYUVBmps[i].width() == SkToInt(fYUVBmps[i].rowBytes()));
            yuvTextures[i] = gpu->createTestingOnlyBackendTexture(fYUVBmps[i].getPixels(),
                                                                  fYUVBmps[i].width(),
                                                                  fYUVBmps[i].height(),
                                                                  kAlpha_8_GrPixelConfig,
                                                                  false, GrMipMapped::kNo);
        }
        context->resetContext();
    }

    void deleteYUVTextures(GrContext* context, GrBackendTexture yuvTextures[3]) {

        GrGpu* gpu = context->contextPriv().getGpu();
        if (!gpu) {
            return;
        }

        for (int i = 0; i < 3; ++i) {
            if (yuvTextures[i].isValid()) {
                gpu->deleteTestingOnlyBackendTexture(&yuvTextures[i]);
            }
        }

        context->resetContext();
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();
        if (!context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }


        constexpr SkScalar kPad = 10.f;

        SkISize sizes[] = {
            { fYUVBmps[0].width(), fYUVBmps[0].height()},
            { fYUVBmps[1].width(), fYUVBmps[1].height()},
            { fYUVBmps[2].width(), fYUVBmps[2].height()},
        };
        SkTArray<sk_sp<SkImage>> images;
        images.push_back(fRGBImage);
        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            GrBackendTexture yuvTextures[3];
            this->createYUVTextures(context, yuvTextures);
            images.push_back(SkImage::MakeFromYUVTexturesCopy(context,
                                                              static_cast<SkYUVColorSpace>(space),
                                                              yuvTextures, sizes,
                                                              kTopLeft_GrSurfaceOrigin));
            this->deleteYUVTextures(context, yuvTextures);
        }
        for (int i = 0; i < images.count(); ++ i) {
            SkScalar y = (i + 1) * kPad + i * fYUVBmps[0].height();
            SkScalar x = kPad;

            canvas->drawImage(images[i].get(), x, y);
        }
     }

private:
    sk_sp<SkImage>  fRGBImage;
    SkBitmap        fYUVBmps[3];

    static constexpr int kBmpSize = 32;

    typedef GM INHERITED;
};

DEF_GM(return new ImageFromYUVTextures;)
}

#endif
