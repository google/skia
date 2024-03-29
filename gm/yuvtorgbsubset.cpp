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
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "tools/gpu/YUVUtils.h"

#include <memory>
#include <utility>

class SkCanvas;

namespace skiagm {

//////////////////////////////////////////////////////////////////////////////

// This GM tests subsetting YUV multiplanar images where the U and V
// planes have different resolution from Y. See skbug:8959

class YUVtoRGBSubsetEffect : public GM {
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

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg, GraphiteTestContext*) override {
        auto context = GrAsDirectContext(canvas->recordingContext());
        skgpu::graphite::Recorder* recorder = nullptr;
#if defined(SK_GRAPHITE)
        recorder = canvas->recorder();
#endif
        if (!context && !recorder) {
            return DrawResult::kSkip;
        }

        if (!fPixmaps.isValid()) {
            this->makePixmaps();
        }

        auto lazyYUV = sk_gpu_test::LazyYUVImage::Make(fPixmaps);
#if defined(SK_GRAPHITE)
        if (recorder) {
            fYUVImage = lazyYUV->refImage(recorder, sk_gpu_test::LazyYUVImage::Type::kFromPixmaps);
        } else
#endif
        {
            fYUVImage = lazyYUV->refImage(context, sk_gpu_test::LazyYUVImage::Type::kFromPixmaps);
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override { fYUVImage.reset(); }

    DrawResult onDraw(SkCanvas* canvas,
                      SkString* errorMsg) override {
        auto context = GrAsDirectContext(canvas->recordingContext());
        skgpu::graphite::Recorder* recorder = nullptr;
#if defined(SK_GRAPHITE)
        recorder = canvas->recorder();
#endif
        if (!context && !recorder) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        if (!fYUVImage) {
            *errorMsg = "No valid YUV image generated -- skipping";
            return DrawResult::kSkip;
        }

        static const SkFilterMode kFilters[] = {SkFilterMode::kNearest,
                                                SkFilterMode::kLinear};
        static const SkIRect kColorRect = SkIRect::MakeLTRB(2, 2, 6, 6);

        // Outset to visualize wrap modes.
        SkRect rect = SkRect::Make(fYUVImage->dimensions());
        rect = rect.makeOutset(fYUVImage->width()/2.f, fYUVImage->height()/2.f);

        SkScalar y = kTestPad;
        // Rows are filter modes.
        for (uint32_t i = 0; i < std::size(kFilters); ++i) {
            SkScalar x = kTestPad;
            // Columns are non-subsetted followed by subsetted with each TileMode in a row
            for (uint32_t j = 0; j < kSkTileModeCount + 1; ++j) {
                SkMatrix ctm = SkMatrix::Translate(x, y);
                ctm.postScale(10.f, 10.f);

                const SkIRect* subset = j > 0 ? &kColorRect : nullptr;

                auto tm = SkTileMode::kClamp;
                if (j > 0) {
                    tm = static_cast<SkTileMode>(j - 1);
                }

                canvas->save();
                canvas->concat(ctm);
                SkSamplingOptions sampling(kFilters[i]);
                SkPaint paint;
                // Draw black rectangle in background so rendering with Decal tilemode matches
                // the previously used ClampToBorder wrapmode.
                paint.setColor(SK_ColorBLACK);
                canvas->drawRect(rect, paint);
                if (subset) {
                    sk_sp<SkImage> subsetImg;
#if defined(SK_GRAPHITE)
                    if (recorder) {
                        subsetImg = fYUVImage->makeSubset(recorder, *subset, {false});
                    } else
#endif
                    {
                        subsetImg = fYUVImage->makeSubset(context, *subset);
                    }
                    SkASSERT(subsetImg);
                    paint.setShader(subsetImg->makeShader(tm, tm,
                                                          sampling, SkMatrix::Translate(2, 2)));
                } else {
                    paint.setShader(fYUVImage->makeShader(tm, tm,
                                                          sampling, SkMatrix::I()));
                }
                canvas->drawRect(rect, paint);
                canvas->restore();
                x += rect.width() + kTestPad;
            }

            y += rect.height() + kTestPad;
        }

        return DrawResult::kOk;
    }

private:
    SkYUVAPixmaps fPixmaps;
    sk_sp<SkImage> fYUVImage;

    inline static constexpr SkScalar kTestPad = 10.f;

    using INHERITED = GM;
};

DEF_GM(return new YUVtoRGBSubsetEffect;)
}  // namespace skiagm
