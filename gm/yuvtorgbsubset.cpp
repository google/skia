/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkYUVAIndex.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"

#include <memory>
#include <utility>

class SkCanvas;

#define YSIZE 8
#define USIZE 4
#define VSIZE 4

namespace skiagm {

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

        unsigned char innerY[16] = {149, 160, 130, 105,
                                    160, 130, 105, 149,
                                    130, 105, 149, 160,
                                    105, 149, 160, 130};
        unsigned char innerU[4] = {43, 75, 145, 200};
        unsigned char innerV[4] = {88, 180, 200, 43};
        int outerYUV[] = {128, 128, 128};
        for (int i = 0; i < 3; ++i) {
            fBitmaps[i].eraseColor(SkColorSetARGB(outerYUV[i], 0, 0, 0));
        }
        SkPixmap innerYPM(SkImageInfo::MakeA8(4, 4), innerY, 4);
        SkPixmap innerUPM(SkImageInfo::MakeA8(2, 2), innerU, 2);
        SkPixmap innerVPM(SkImageInfo::MakeA8(2, 2), innerV, 2);
        fBitmaps[0].writePixels(innerYPM, 2, 2);
        fBitmaps[1].writePixels(innerUPM, 1, 1);
        fBitmaps[2].writePixels(innerVPM, 1, 1);
        for (auto& fBitmap : fBitmaps) {
            fBitmap.setImmutable();
        }
    }

    DrawResult onDraw(GrRecordingContext* context, GrSurfaceDrawContext* surfaceDrawContext,
                      SkCanvas* canvas, SkString* errorMsg) override {
        GrSurfaceProxyView views[3];

        for (int i = 0; i < 3; ++i) {
            GrBitmapTextureMaker maker(context, fBitmaps[i], GrImageTexGenPolicy::kDraw);
            views[i] = maker.view(GrMipmapped::kNo);
            if (!views[i]) {
                *errorMsg = "Failed to create proxy";
                return DrawResult::kFail;
            }
        }

        static const GrSamplerState::Filter kFilters[] = {GrSamplerState::Filter::kNearest,
                                                          GrSamplerState::Filter::kLinear};
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
                SkMatrix ctm = SkMatrix::Translate(x, y);
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
                    grPaint.setColorFragmentProcessor(std::move(fp));
                    surfaceDrawContext->drawRect(
                            nullptr, std::move(grPaint), GrAA::kYes, ctm, rect);
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

    using INHERITED = GM;
};

DEF_GM(return new YUVtoRGBSubsetEffect;)
}  // namespace skiagm
