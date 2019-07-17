/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAIndex.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrSamplerState.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrFillRectOp.h"

#include <memory>
#include <utility>

class SkCanvas;

#define YSIZE 8
#define USIZE 4
#define VSIZE 4

namespace skiagm {
/**
 * This GM directly exercises GrYUVtoRGBEffect.
 */
class YUVtoRGBEffect : public GpuGM {
public:
    YUVtoRGBEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_to_rgb_effect");
    }

    SkISize onISize() override {
        int numRows = kLastEnum_SkYUVColorSpace + 1;
        return SkISize::Make(238, kDrawPad + numRows * kColorSpaceOffset);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bmp[3];
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        bmp[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(USIZE, USIZE);
        bmp[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(VSIZE, VSIZE);
        bmp[2].allocPixels(vinfo);
        unsigned char* pixels[3];
        for (int i = 0; i < 3; ++i) {
            pixels[i] = (unsigned char*)bmp[i].getPixels();
        }
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[]  = {0, 255, 0};
        const int inc[]   = {1, -1, 1};
        for (int i = 0; i < 3; ++i) {
            const size_t nbBytes = bmp[i].rowBytes() * bmp[i].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[i][j] = (unsigned char)color[i];
                color[i] = (color[i] == limit[i]) ? invl[i] : color[i] + inc[i];
            }
        }
        for (int i = 0; i < 3; ++i) {
            fImage[i] = SkImage::MakeFromBitmap(bmp[i]);
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrTextureProxy> proxies[3];

        for (int i = 0; i < 3; ++i) {
            proxies[i] = proxyProvider->createTextureProxy(fImage[i], GrRenderable::kNo, 1,
                                                           SkBudgeted::kYes, SkBackingFit::kExact);
            if (!proxies[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fImage[0]->width()),
                                               SkIntToScalar(fImage[0]->height()));
            renderRect.outset(kDrawPad, kDrawPad);

            SkScalar y = kDrawPad + kTestPad + space * kColorSpaceOffset;
            SkScalar x = kDrawPad + kTestPad;

            const int indices[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                       {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};

            for (int i = 0; i < 6; ++i) {
                SkYUVAIndex yuvaIndices[4] = {
                    { indices[i][0], SkColorChannel::kR },
                    { indices[i][1], SkColorChannel::kR },
                    { indices[i][2], SkColorChannel::kR },
                    { -1, SkColorChannel::kA }
                };

                std::unique_ptr<GrFragmentProcessor> fp(
                        GrYUVtoRGBEffect::Make(proxies, yuvaIndices,
                                               static_cast<SkYUVColorSpace>(space),
                                               GrSamplerState::Filter::kNearest));
                if (fp) {
                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    SkMatrix viewMatrix;
                    viewMatrix.setTranslate(x, y);
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrFillRectOp::MakeNonAARect(context, std::move(grPaint),
                                                        viewMatrix, renderRect));
                }
                x += renderRect.width() + kTestPad;
            }
        }
        return DrawResult::kOk;
     }

private:
    sk_sp<SkImage> fImage[3];

    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    static constexpr SkScalar kColorSpaceOffset = 36.f;

    typedef GM INHERITED;
};

DEF_GM(return new YUVtoRGBEffect;)

//////////////////////////////////////////////////////////////////////////////

class YUVNV12toRGBEffect : public GpuGM {
public:
    YUVNV12toRGBEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_nv12_to_rgb_effect");
    }

    SkISize onISize() override {
        int numRows = kLastEnum_SkYUVColorSpace + 1;
        return SkISize::Make(48, kDrawPad + numRows * kColorSpaceOffset);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bmp[2];
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        bmp[0].allocPixels(yinfo);
        SkImageInfo uvinfo = SkImageInfo::MakeN32Premul(USIZE, USIZE);
        bmp[1].allocPixels(uvinfo);
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[] = {0, 255, 0};
        const int inc[] = {1, -1, 1};

        {
            unsigned char* pixels = (unsigned char*)bmp[0].getPixels();
            const size_t nbBytes = bmp[0].rowBytes() * bmp[0].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[j] = (unsigned char)color[0];
                color[0] = (color[0] == limit[0]) ? invl[0] : color[0] + inc[0];
            }
        }

        {
            for (int y = 0; y < bmp[1].height(); ++y) {
                uint32_t* pixels = bmp[1].getAddr32(0, y);
                for (int j = 0; j < bmp[1].width(); ++j) {
                    pixels[j] = SkColorSetARGB(0, color[1], color[2], 0);
                    color[1] = (color[1] == limit[1]) ? invl[1] : color[1] + inc[1];
                    color[2] = (color[2] == limit[2]) ? invl[2] : color[2] + inc[2];
                }
            }
        }

        for (int i = 0; i < 2; ++i) {
            fImage[i] = SkImage::MakeFromBitmap(bmp[i]);
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrTextureProxy> proxies[2];

        for (int i = 0; i < 2; ++i) {
            proxies[i] = proxyProvider->createTextureProxy(fImage[i], GrRenderable::kNo, 1,
                                                           SkBudgeted::kYes, SkBackingFit::kExact);
            if (!proxies[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        SkYUVAIndex yuvaIndices[4] = {
            {  0, SkColorChannel::kR },
            {  1, SkColorChannel::kR },
            {  1, SkColorChannel::kG },
            { -1, SkColorChannel::kA }
        };

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fImage[0]->width()),
                                               SkIntToScalar(fImage[0]->height()));
            renderRect.outset(kDrawPad, kDrawPad);

            SkScalar y = kDrawPad + kTestPad + space * kColorSpaceOffset;
            SkScalar x = kDrawPad + kTestPad;

            GrPaint grPaint;
            grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
            auto fp = GrYUVtoRGBEffect::Make(proxies, yuvaIndices,
                                             static_cast<SkYUVColorSpace>(space),
                                             GrSamplerState::Filter::kNearest);
            if (fp) {
                SkMatrix viewMatrix;
                viewMatrix.setTranslate(x, y);
                grPaint.addColorFragmentProcessor(std::move(fp));
                std::unique_ptr<GrDrawOp> op(GrFillRectOp::MakeNonAARect(
                        context, std::move(grPaint), viewMatrix, renderRect));
                renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
            }
        }
        return DrawResult::kOk;
    }

private:
    sk_sp<SkImage> fImage[2];

    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    static constexpr SkScalar kColorSpaceOffset = 36.f;

    typedef GM INHERITED;
};

DEF_GM(return new YUVNV12toRGBEffect;)

//////////////////////////////////////////////////////////////////////////////

// This GM tests domain clamping on YUV multiplanar images where the U and V
// planes have different resolution from Y. See skbug:8959

class YUVtoRGBDomainEffect : public GpuGM {
public:
    YUVtoRGBDomainEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_to_rgb_domain_effect");
    }

    SkISize onISize() override {
        return SkISize::Make((YSIZE + kTestPad) * 3 + kDrawPad, (YSIZE + kTestPad) * 2 + kDrawPad);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bmp[3];
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        bmp[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(USIZE, USIZE);
        bmp[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(VSIZE, VSIZE);
        bmp[2].allocPixels(vinfo);

        int innerColor[] = {149, 43, 21};
        int outerColor[] = {128, 128, 128};
        for (int i = 0; i < 3; ++i) {
            bmp[i].eraseColor(SkColorSetARGB(outerColor[i], 0, 0, 0));
            SkIRect innerRect = i == 0 ? SkIRect::MakeLTRB(2, 2, 6, 6) : SkIRect::MakeLTRB(1, 1, 3, 3);
            bmp[i].erase(SkColorSetARGB(innerColor[i], 0, 0, 0), innerRect);
            fImage[i] = SkImage::MakeFromBitmap(bmp[i]);
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrTextureProxy> proxies[3];

        for (int i = 0; i < 3; ++i) {
            proxies[i] = proxyProvider->createTextureProxy(fImage[i], GrRenderable::kNo, 1,
                                                           SkBudgeted::kYes, SkBackingFit::kExact);
            if (!proxies[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        // Draw a 2x2 grid of the YUV images.
        // Rows = kNearest, kBilerp, Cols = No clamp, clamp
        static const GrSamplerState::Filter kFilters[] = {
                GrSamplerState::Filter::kNearest, GrSamplerState::Filter::kBilerp };
        static const SkRect kGreenRect = SkRect::MakeLTRB(2.f, 2.f, 6.f, 6.f);

        SkYUVAIndex yuvaIndices[4] = {
            { SkYUVAIndex::kY_Index, SkColorChannel::kR },
            { SkYUVAIndex::kU_Index, SkColorChannel::kR },
            { SkYUVAIndex::kV_Index, SkColorChannel::kR },
            { -1, SkColorChannel::kA }
        };
        SkRect rect = SkRect::MakeWH(YSIZE, YSIZE);

        SkScalar y = kDrawPad + kTestPad;
        for (uint32_t i = 0; i < SK_ARRAY_COUNT(kFilters); ++i) {
            SkScalar x = kDrawPad + kTestPad;

            for (uint32_t j = 0; j < 2; ++j) {
                SkMatrix ctm = SkMatrix::MakeTrans(x, y);
                ctm.postScale(10.f, 10.f);

                SkRect domain = kGreenRect;
                if (kFilters[i] == GrSamplerState::Filter::kNearest) {
                    // Make a very small inset for nearest-neighbor filtering so that 0.5px
                    // centers don't round out beyond the green pixels.
                    domain.inset(0.01f, 0.01f);
                }

                const SkRect* domainPtr = j > 0 ? &domain : nullptr;
                std::unique_ptr<GrFragmentProcessor> fp(GrYUVtoRGBEffect::Make(proxies, yuvaIndices,
                        kJPEG_SkYUVColorSpace, kFilters[i], SkMatrix::I(), domainPtr));
                if (fp) {
                    GrPaint grPaint;
                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->drawRect(
                            GrNoClip(), std::move(grPaint), GrAA::kYes, ctm, rect);
                }
                x += rect.width() + kTestPad;
            }

            y += rect.height() + kTestPad;
        }

        return DrawResult::kOk;
     }

private:
    sk_sp<SkImage> fImage[3];

    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;

    typedef GM INHERITED;
};

DEF_GM(return new YUVtoRGBDomainEffect;)
}
