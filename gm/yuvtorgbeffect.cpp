/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrDrawContextPriv.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkGradientShader.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "effects/GrYUVEffect.h"

#define YSIZE 8
#define USIZE 4
#define VSIZE 4

namespace skiagm {
/**
 * This GM directly exercises GrYUVtoRGBEffect.
 */
class YUVtoRGBEffect : public GM {
public:
    YUVtoRGBEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_to_rgb_effect");
    }

    SkISize onISize() override {
        return SkISize::Make(238, 120);
    }

    void onOnceBeforeDraw() override {
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        fBmp[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(USIZE, USIZE);
        fBmp[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(VSIZE, VSIZE);
        fBmp[2].allocPixels(vinfo);
        unsigned char* pixels[3];
        for (int i = 0; i < 3; ++i) {
            pixels[i] = (unsigned char*)fBmp[i].getPixels();
        }
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[]  = {0, 255, 0};
        const int inc[]   = {1, -1, 1};
        for (int i = 0; i < 3; ++i) {
            const size_t nbBytes = fBmp[i].rowBytes() * fBmp[i].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[i][j] = (unsigned char)color[i];
                color[i] = (color[i] == limit[i]) ? invl[i] : color[i] + inc[i];
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;        
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        SkAutoTUnref<GrTexture> texture[3];
        texture[0].reset(GrRefCachedBitmapTexture(context, fBmp[0],
                                                  GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));
        texture[1].reset(GrRefCachedBitmapTexture(context, fBmp[1],
                                                  GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));
        texture[2].reset(GrRefCachedBitmapTexture(context, fBmp[2],
                                                  GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));

        if (!texture[0] || !texture[1] || !texture[2]) {
            return;
        }

        static const SkScalar kDrawPad = 10.f;
        static const SkScalar kTestPad = 10.f;
        static const SkScalar kColorSpaceOffset = 36.f;
        SkISize sizes[3] = {{YSIZE, YSIZE}, {USIZE, USIZE}, {VSIZE, VSIZE}};

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace;
             ++space) {
            SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fBmp[0].width()),
                                               SkIntToScalar(fBmp[0].height()));
            renderRect.outset(kDrawPad, kDrawPad);

            SkScalar y = kDrawPad + kTestPad + space * kColorSpaceOffset;
            SkScalar x = kDrawPad + kTestPad;

            const int indices[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                       {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};

            for (int i = 0; i < 6; ++i) {
                GrPaint grPaint;
                grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));
                sk_sp<GrFragmentProcessor> fp(GrYUVEffect::MakeYUVToRGB(
                    texture[indices[i][0]], texture[indices[i][1]], texture[indices[i][2]], sizes,
                    static_cast<SkYUVColorSpace>(space), false));
                if (fp) {
                    SkMatrix viewMatrix;
                    viewMatrix.setTranslate(x, y);
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    SkAutoTUnref<GrDrawBatch> batch(
                            GrRectBatchFactory::CreateNonAAFill(GrColor_WHITE, viewMatrix,
                                                                renderRect, nullptr, nullptr));
                    drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
                }
                x += renderRect.width() + kTestPad;
            }
        }
     }

private:
    SkBitmap fBmp[3];

    typedef GM INHERITED;
};

DEF_GM(return new YUVtoRGBEffect;)

//////////////////////////////////////////////////////////////////////////////

class YUVNV12toRGBEffect : public GM {
public:
    YUVNV12toRGBEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_nv12_to_rgb_effect");
    }

    SkISize onISize() override {
        return SkISize::Make(48, 120);
    }

    void onOnceBeforeDraw() override {
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        fBmp[0].allocPixels(yinfo);
        SkImageInfo uvinfo = SkImageInfo::MakeN32Premul(USIZE, USIZE);
        fBmp[1].allocPixels(uvinfo);
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[] = {0, 255, 0};
        const int inc[] = {1, -1, 1};

        {
            unsigned char* pixels = (unsigned char*)fBmp[0].getPixels();
            const size_t nbBytes = fBmp[0].rowBytes() * fBmp[0].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[j] = (unsigned char)color[0];
                color[0] = (color[0] == limit[0]) ? invl[0] : color[0] + inc[0];
            }
        }

        {
            for (int y = 0; y < fBmp[1].height(); ++y) {
                uint32_t* pixels = fBmp[1].getAddr32(0, y);
                for (int j = 0; j < fBmp[1].width(); ++j) {
                    pixels[j] = SkColorSetARGB(0, color[1], color[2], 0);
                    color[1] = (color[1] == limit[1]) ? invl[1] : color[1] + inc[1];
                    color[2] = (color[2] == limit[2]) ? invl[2] : color[2] + inc[2];
                }
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        SkAutoTUnref<GrTexture> texture[3];
        texture[0].reset(GrRefCachedBitmapTexture(context, fBmp[0], GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));
        texture[1].reset(GrRefCachedBitmapTexture(context, fBmp[1], GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));
        texture[2].reset(GrRefCachedBitmapTexture(context, fBmp[1], GrTextureParams::ClampBilerp(),
                                                  SkSourceGammaTreatment::kRespect));

        if (!texture[0] || !texture[1] || !texture[2]) {
            return;
        }

        static const SkScalar kDrawPad = 10.f;
        static const SkScalar kTestPad = 10.f;
        static const SkScalar kColorSpaceOffset = 36.f;
        SkISize sizes[3] = {{YSIZE, YSIZE}, {USIZE, USIZE}, {VSIZE, VSIZE}};

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            SkRect renderRect =
                SkRect::MakeWH(SkIntToScalar(fBmp[0].width()), SkIntToScalar(fBmp[0].height()));
            renderRect.outset(kDrawPad, kDrawPad);

            SkScalar y = kDrawPad + kTestPad + space * kColorSpaceOffset;
            SkScalar x = kDrawPad + kTestPad;

            GrPaint grPaint;
            grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));
            sk_sp<GrFragmentProcessor> fp(
                GrYUVEffect::MakeYUVToRGB(texture[0], texture[1], texture[2], sizes,
                                          static_cast<SkYUVColorSpace>(space), true));
            if (fp) {
                SkMatrix viewMatrix;
                viewMatrix.setTranslate(x, y);
                grPaint.addColorFragmentProcessor(fp);
                SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(
                    GrColor_WHITE, viewMatrix, renderRect, nullptr, nullptr));
                drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
            }
        }
    }

private:
    SkBitmap fBmp[2];

    typedef GM INHERITED;
};

DEF_GM(return new YUVNV12toRGBEffect;)
}

#endif
