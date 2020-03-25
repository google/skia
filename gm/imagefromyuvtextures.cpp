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
#include "src/core/SkYUVMath.h"

class GrRenderTargetContext;

static sk_sp<SkColorFilter> yuv_to_rgb_colorfilter() {
    float m[20];
    SkColorMatrix_YUV2RGB(kJPEG_SkYUVColorSpace, m);
    return SkColorFilters::Matrix(m);
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
        // Original image, plus each color space drawn twice
        int numBitmaps = 2 * (kLastEnum_SkYUVColorSpace + 1) + 1;
        return SkISize::Make(kBmpSize + 2 * kPad, numBitmaps * (kBmpSize + kPad) + kPad);
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
        auto ii =
                SkImageInfo::Make(kBmpSize, kBmpSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        rgbBmp.allocPixels(ii);
        SkCanvas canvas(rgbBmp);
        canvas.drawPaint(paint);
        SkPMColor* rgbColors = static_cast<SkPMColor*>(rgbBmp.getPixels());

        SkImageInfo yinfo = SkImageInfo::Make(kBmpSize, kBmpSize, kGray_8_SkColorType,
                                              kUnpremul_SkAlphaType);
        fYUVBmps[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::Make(kBmpSize / 2, kBmpSize / 2, kGray_8_SkColorType,
                                              kUnpremul_SkAlphaType);
        fYUVBmps[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::Make(kBmpSize / 2, kBmpSize / 2, kGray_8_SkColorType,
                                              kUnpremul_SkAlphaType);
        fYUVBmps[2].allocPixels(vinfo);
        unsigned char* yPixels;
        signed char* uvPixels[2];
        yPixels = static_cast<unsigned char*>(fYUVBmps[0].getPixels());
        uvPixels[0] = static_cast<signed char*>(fYUVBmps[1].getPixels());
        uvPixels[1] = static_cast<signed char*>(fYUVBmps[2].getPixels());

        float m[20];
        SkColorMatrix_RGB2YUV(kJPEG_SkYUVColorSpace, m);
        // Here we encode using the kJPEG_SkYUVColorSpace (i.e., full-swing Rec 601) even though
        // we will draw it with all the supported yuv color spaces when converted back to RGB
        for (int i = 0; i < kBmpSize * kBmpSize; ++i) {
            auto r = (rgbColors[i] & 0x000000ff) >>  0;
            auto g = (rgbColors[i] & 0x0000ff00) >>  8;
            auto b = (rgbColors[i] & 0x00ff0000) >> 16;
            auto a = (rgbColors[i] & 0xff000000) >> 24;
            yPixels[i] = SkToU8(sk_float_round2int(m[0]*r + m[1]*g + m[2]*b + m[3]*a + 255*m[4]));
        }
        for (int j = 0; j < kBmpSize / 2; ++j) {
            for (int i = 0; i < kBmpSize / 2; ++i) {
                // Average together 4 pixels of RGB.
                int rgba[] = {0, 0, 0, 0};
                for (int y = 0; y < 2; ++y) {
                    for (int x = 0; x < 2; ++x) {
                        int rgbIndex = (2 * j + y) * kBmpSize + 2 * i + x;
                        rgba[0] += (rgbColors[rgbIndex] & 0x000000ff) >>  0;
                        rgba[1] += (rgbColors[rgbIndex] & 0x0000ff00) >>  8;
                        rgba[2] += (rgbColors[rgbIndex] & 0x00ff0000) >> 16;
                        rgba[3] += (rgbColors[rgbIndex] & 0xff000000) >> 24;
                    }
                }
                for (int c = 0; c < 4; ++c) {
                    rgba[c] /= 4;
                }
                int uvIndex = j * kBmpSize / 2 + i;
                uvPixels[0][uvIndex] = SkToU8(sk_float_round2int(
                        m[5]*rgba[0] + m[6]*rgba[1] + m[7]*rgba[2] + m[8]*rgba[3] + 255*m[9]));
                uvPixels[1][uvIndex] = SkToU8(sk_float_round2int(
                        m[10]*rgba[0] + m[11]*rgba[1] + m[12]*rgba[2] + m[13]*rgba[3] + 255*m[14]));
            }
        }
        fRGBImage = SkImage::MakeRasterCopy(SkPixmap(rgbBmp.info(), rgbColors, rgbBmp.rowBytes()));
    }

    void createYUVTextures(GrContext* context, GrBackendTexture yuvTextures[3]) {
        for (int i = 0; i < 3; ++i) {
            SkASSERT(fYUVBmps[i].width() == SkToInt(fYUVBmps[i].rowBytes()));
            yuvTextures[i] = context->createBackendTexture(&fYUVBmps[i].pixmap(), 1,
                                                           GrRenderable::kNo, GrProtected::kNo);
        }
    }

    void createResultTexture(GrContext* context, int width, int height,
                             GrBackendTexture* resultTexture) {
        *resultTexture = context->createBackendTexture(
                width, height, kRGBA_8888_SkColorType, SkColors::kTransparent,
                GrMipMapped::kNo, GrRenderable::kYes, GrProtected::kNo);
    }

    void deleteBackendTextures(GrContext* context, GrBackendTexture textures[], int n) {
        if (context->abandoned()) {
            return;
        }

        GrFlushInfo flushInfo;
        flushInfo.fFlags = kSyncCpu_GrFlushFlag;
        context->flush(flushInfo);

        for (int i = 0; i < n; ++i) {
            context->deleteBackendTexture(textures[i]);
        }
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
