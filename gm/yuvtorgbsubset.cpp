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
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"

#include <memory>
#include <utility>

class SkCanvas;

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
    SkString getName() const override { return SkString("yuv_to_rgb_subset_effect"); }

    SkISize getISize() override { return {1310, 540}; }

    void makePixmaps() {
        SkYUVAInfo yuvaInfo = SkYUVAInfo({8, 8},
                                         SkYUVAInfo::PlaneConfig::kY_U_V,
                                         SkYUVAInfo::Subsampling::k420,
                                         kJPEG_Full_SkYUVColorSpace);
        SkColorType colorTypes[] = {kAlpha_8_SkColorType,
                                    kAlpha_8_SkColorType,
                                    kAlpha_8_SkColorType};
        SkYUVAPixmapInfo pmapInfo(yuvaInfo, colorTypes, nullptr);
        fPixmaps = SkYUVAPixmaps::Allocate(pmapInfo);

        unsigned char innerY[16] = {149, 160, 130, 105,
                                    160, 130, 105, 149,
                                    130, 105, 149, 160,
                                    105, 149, 160, 130};
        unsigned char innerU[4] = {43, 75, 145, 200};
        unsigned char innerV[4] = {88, 180, 200, 43};
        int outerYUV[] = {128, 128, 128};
        SkBitmap bitmaps[3];
        for (int i = 0; i < 3; ++i) {
            bitmaps[i].installPixels(fPixmaps.plane(i));
            bitmaps[i].eraseColor(SkColorSetARGB(outerYUV[i], 0, 0, 0));
        }
        SkPixmap innerYPM(SkImageInfo::MakeA8(4, 4), innerY, 4);
        SkPixmap innerUPM(SkImageInfo::MakeA8(2, 2), innerU, 2);
        SkPixmap innerVPM(SkImageInfo::MakeA8(2, 2), innerV, 2);
        bitmaps[0].writePixels(innerYPM, 2, 2);
        bitmaps[1].writePixels(innerUPM, 1, 1);
        bitmaps[2].writePixels(innerVPM, 1, 1);
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto context = GrAsDirectContext(canvas->recordingContext());
        if (!context) {
            return DrawResult::kSkip;
        }
        if (!fPixmaps.isValid()) {
            this->makePixmaps();
        }
        GrSurfaceProxyView views[SkYUVAInfo::kMaxPlanes];
        GrColorType colorTypes[SkYUVAInfo::kMaxPlanes];
        for (int i = 0; i < fPixmaps.numPlanes(); ++i) {
            SkBitmap bitmap;
            bitmap.installPixels(fPixmaps.plane(i));
            bitmap.setImmutable();
            views[i] = std::get<0>(GrMakeCachedBitmapProxyView(
                    context, bitmap, /*label=*/"DrawResult_GpuSetup", skgpu::Mipmapped::kNo));
            if (!views[i]) {
                *errorMsg = "Failed to create proxy";
                return context->abandoned() ? DrawResult::kSkip : DrawResult::kFail;
            }
            colorTypes[i] = SkColorTypeToGrColorType(bitmap.colorType());
        }
        fProxies = GrYUVATextureProxies(fPixmaps.yuvaInfo(), views, colorTypes);
        if (!fProxies.isValid()) {
            *errorMsg = "Failed to create GrYUVATextureProxies";
            return DrawResult::kFail;
        }
        return DrawResult::kOk;
    }

    void onGpuTeardown() override { fProxies = {}; }

    DrawResult onDraw(GrRecordingContext* rContext,
                      SkCanvas* canvas,
                      SkString* errorMsg) override {
        auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        static const GrSamplerState::Filter kFilters[] = {GrSamplerState::Filter::kNearest,
                                                          GrSamplerState::Filter::kLinear};
        static const SkRect kColorRect = SkRect::MakeLTRB(2.f, 2.f, 6.f, 6.f);

        // Outset to visualize wrap modes.
        SkRect rect = SkRect::Make(fProxies.yuvaInfo().dimensions());
        rect = rect.makeOutset(fProxies.yuvaInfo().width()/2.f, fProxies.yuvaInfo().height()/2.f);

        SkScalar y = kTestPad;
        // Rows are filter modes.
        for (uint32_t i = 0; i < std::size(kFilters); ++i) {
            SkScalar x = kTestPad;
            // Columns are non-subsetted followed by subsetted with each WrapMode in a row
            for (uint32_t j = 0; j < GrSamplerState::kWrapModeCount + 1; ++j) {
                SkMatrix ctm = SkMatrix::Translate(x, y);
                ctm.postScale(10.f, 10.f);

                const SkRect* subset = j > 0 ? &kColorRect : nullptr;

                auto wm = GrSamplerState::WrapMode::kClamp;
                if (j > 0) {
                    wm = static_cast<GrSamplerState::WrapMode>(j - 1);
                }
                GrSamplerState samplerState(wm, kFilters[i]);

                const auto& caps = *rContext->priv().caps();
                std::unique_ptr<GrFragmentProcessor> fp =
                        GrYUVtoRGBEffect::Make(fProxies, samplerState, caps, SkMatrix::I(), subset);
                if (fp) {
                    GrPaint grPaint;
                    grPaint.setColorFragmentProcessor(std::move(fp));
                    sdc->drawRect(nullptr, std::move(grPaint), GrAA::kYes, ctm, rect);
                }
                x += rect.width() + kTestPad;
            }

            y += rect.height() + kTestPad;
        }

        return DrawResult::kOk;
    }

private:
    SkYUVAPixmaps fPixmaps;
    GrYUVATextureProxies fProxies;

    inline static constexpr SkScalar kTestPad = 10.f;

    using INHERITED = GM;
};

DEF_GM(return new YUVtoRGBSubsetEffect;)
}  // namespace skiagm
