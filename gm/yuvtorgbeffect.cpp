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
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSamplerState.h"
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
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        fBitmaps[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(USIZE, USIZE);
        fBitmaps[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(VSIZE, VSIZE);
        fBitmaps[2].allocPixels(vinfo);
        unsigned char* pixels[3];
        for (int i = 0; i < 3; ++i) {
            pixels[i] = (unsigned char*)fBitmaps[i].getPixels();
        }
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[]  = {0, 255, 0};
        const int inc[]   = {1, -1, 1};
        for (int i = 0; i < 3; ++i) {
            const size_t nbBytes = fBitmaps[i].rowBytes() * fBitmaps[i].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[i][j] = (unsigned char)color[i];
                color[i] = (color[i] == limit[i]) ? invl[i] : color[i] + inc[i];
            }
        }
        for (int i = 0; i < 3; ++i) {
            fBitmaps[i].setImmutable();
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrSurfaceProxyView views[3];

        for (int i = 0; i < 3; ++i) {
            GrBitmapTextureMaker maker(context, fBitmaps[i], GrImageTexGenPolicy::kDraw);
            views[i] = maker.view(GrMipMapped::kNo);
            if (!views[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        for (int space = kJPEG_SkYUVColorSpace; space <= kLastEnum_SkYUVColorSpace; ++space) {
            SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fBitmaps[0].width()),
                                               SkIntToScalar(fBitmaps[0].height()));
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
                const auto& caps = *context->priv().caps();
                std::unique_ptr<GrFragmentProcessor> fp(GrYUVtoRGBEffect::Make(
                        views, yuvaIndices, static_cast<SkYUVColorSpace>(space),
                        GrSamplerState::Filter::kNearest, caps));
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
    SkBitmap fBitmaps[3];

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
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        fBitmaps[0].allocPixels(yinfo);
        SkImageInfo uvinfo = SkImageInfo::MakeN32Premul(USIZE, USIZE);
        fBitmaps[1].allocPixels(uvinfo);
        int color[] = {0, 85, 170};
        const int limit[] = {255, 0, 255};
        const int invl[] = {0, 255, 0};
        const int inc[] = {1, -1, 1};

        {
            unsigned char* pixels = (unsigned char*)fBitmaps[0].getPixels();
            const size_t nbBytes = fBitmaps[0].rowBytes() * fBitmaps[0].height();
            for (size_t j = 0; j < nbBytes; ++j) {
                pixels[j] = (unsigned char)color[0];
                color[0] = (color[0] == limit[0]) ? invl[0] : color[0] + inc[0];
            }
        }

        {
            for (int y = 0; y < fBitmaps[1].height(); ++y) {
                uint32_t* pixels = fBitmaps[1].getAddr32(0, y);
                for (int j = 0; j < fBitmaps[1].width(); ++j) {
                    pixels[j] = SkColorSetARGB(0, color[1], color[2], 0);
                    color[1] = (color[1] == limit[1]) ? invl[1] : color[1] + inc[1];
                    color[2] = (color[2] == limit[2]) ? invl[2] : color[2] + inc[2];
                }
            }
        }

        for (int i = 0; i < 2; ++i) {
            fBitmaps[i].setImmutable();
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrSurfaceProxyView views[2];

        for (int i = 0; i < 2; ++i) {
            GrBitmapTextureMaker maker(context, fBitmaps[i], GrImageTexGenPolicy::kDraw);
            views[i] = maker.view(GrMipMapped::kNo);
            if (!views[i]) {
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
            SkRect renderRect = SkRect::MakeWH(SkIntToScalar(fBitmaps[0].width()),
                                               SkIntToScalar(fBitmaps[0].height()));
            renderRect.outset(kDrawPad, kDrawPad);

            SkScalar y = kDrawPad + kTestPad + space * kColorSpaceOffset;
            SkScalar x = kDrawPad + kTestPad;

            GrPaint grPaint;
            grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
            const auto& caps = *context->priv().caps();
            auto fp = GrYUVtoRGBEffect::Make(views, yuvaIndices,
                                             static_cast<SkYUVColorSpace>(space),
                                             GrSamplerState::Filter::kNearest, caps);
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
    SkBitmap fBitmaps[2];

    static constexpr SkScalar kDrawPad = 10.f;
    static constexpr SkScalar kTestPad = 10.f;
    static constexpr SkScalar kColorSpaceOffset = 36.f;

    typedef GM INHERITED;
};

DEF_GM(return new YUVNV12toRGBEffect;)

//////////////////////////////////////////////////////////////////////////////

// This GM tests subsetting YUV multiplanar images where the U and V
// planes have different resolution from Y. See skbug:8959

class YUVtoRGBSubsetEffect : public GpuGM {
public:
    YUVtoRGBSubsetEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("yuv_to_rgb_subset_effect");
    }

    SkISize onISize() override { return {1310, 540}; }

    void onOnceBeforeDraw() override {
        SkImageInfo yinfo = SkImageInfo::MakeA8(YSIZE, YSIZE);
        fBitmaps[0].allocPixels(yinfo);
        SkImageInfo uinfo = SkImageInfo::MakeA8(USIZE, USIZE);
        fBitmaps[1].allocPixels(uinfo);
        SkImageInfo vinfo = SkImageInfo::MakeA8(VSIZE, VSIZE);
        fBitmaps[2].allocPixels(vinfo);

        int innerY = 149;
        unsigned char innerU[4] = {43, 75, 145, 200};
        unsigned char innerV[4] = {88, 180, 200, 43};
        int outerYUV[] = {128, 128, 128};
        for (int i = 0; i < 3; ++i) {
            fBitmaps[i].eraseColor(SkColorSetARGB(outerYUV[i], 0, 0, 0));
        }
        fBitmaps[0].eraseARGB(innerY, 0, 0, 0);
        SkPixmap innerUPM(SkImageInfo::MakeA8(2, 2), innerU, 2);
        SkPixmap innerVPM(SkImageInfo::MakeA8(2, 2), innerV, 2);
        fBitmaps[1].writePixels(innerUPM, 1, 1);
        fBitmaps[2].writePixels(innerVPM, 1, 1);
        for (auto& fBitmap : fBitmaps) {
            fBitmap.setImmutable();
        }
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrSurfaceProxyView views[3];

        for (int i = 0; i < 3; ++i) {
            GrBitmapTextureMaker maker(context, fBitmaps[i], GrImageTexGenPolicy::kDraw);
            views[i] = maker.view(GrMipMapped::kNo);
            if (!views[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        static const GrSamplerState::Filter kFilters[] = {
                GrSamplerState::Filter::kNearest, GrSamplerState::Filter::kBilerp };
        static const SkRect kColorRect = SkRect::MakeLTRB(2.f, 2.f, 6.f, 6.f);

        SkYUVAIndex yuvaIndices[4] = {
            { SkYUVAIndex::kY_Index, SkColorChannel::kR },
            { SkYUVAIndex::kU_Index, SkColorChannel::kR },
            { SkYUVAIndex::kV_Index, SkColorChannel::kR },
            { -1, SkColorChannel::kA }
        };
        // Outset to visualize wrap modes.
        SkRect rect = SkRect::MakeWH(YSIZE, YSIZE).makeOutset(YSIZE/2, YSIZE/2);

        SkScalar y = kTestPad;
        // Rows are filter modes.
        for (uint32_t i = 0; i < SK_ARRAY_COUNT(kFilters); ++i) {
            SkScalar x = kTestPad;
            // Columns are non-subsetted followed by subsetted with each WrapMode in a row
            for (uint32_t j = 0; j < GrSamplerState::kWrapModeCount + 1; ++j) {
                SkMatrix ctm = SkMatrix::MakeTrans(x, y);
                ctm.postScale(10.f, 10.f);

                const SkRect* subset = j > 0 ? &kColorRect : nullptr;

                GrSamplerState samplerState;
                samplerState.setFilterMode(kFilters[i]);
                if (j > 0) {
                    auto wm = static_cast<GrSamplerState::WrapMode>(j - 1);
                    samplerState.setWrapModeX(wm);
                    samplerState.setWrapModeY(wm);
                }
                const auto& caps = *context->priv().caps();
                std::unique_ptr<GrFragmentProcessor> fp(
                        GrYUVtoRGBEffect::Make(views, yuvaIndices, kJPEG_SkYUVColorSpace,
                                               samplerState, caps, SkMatrix::I(), subset));
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
    SkBitmap fBitmaps[3];

    static constexpr SkScalar kTestPad = 10.f;

    typedef GM INHERITED;
};

DEF_GM(return new YUVtoRGBSubsetEffect;)
}
