/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

class BilerpBench : public Benchmark {
public:
    // Defined constants for 4K dimensions
    static constexpr int k4KWidth = 3840;
    static constexpr int k4KHeight = 2160;

    BilerpBench(float scale)
            : fSrcSize(SkISize::Make(k4KWidth, k4KHeight))
            , fDstSize(SkSize::Make(k4KWidth * scale, k4KHeight * scale))
            , fRec709(false) {
        fName.printf("bilerp_4k_scale_%0.2f", scale);
    }

    BilerpBench(int srcW, int srcH, int dstW, int dstH, bool rec709 = false)
            : fSrcSize(SkISize::Make(srcW, srcH))
            , fDstSize(SkSize::Make(dstW, dstH))
            , fRec709(rec709) {
        fName.printf("bilerp_%dx%d_to_%dx%d%s", srcW, srcH, dstW, dstH, rec709 ? "_rec709" : "");
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    SkISize onGetSize() override {
        return SkISize::Make(SkScalarCeilToInt(fDstSize.fWidth),
                             SkScalarCeilToInt(fDstSize.fHeight));
    }

    void onDelayedSetup() override {
        fImage = ToolUtils::create_checkerboard_image(
                fSrcSize.fWidth, fSrcSize.fHeight, SK_ColorGRAY, SK_ColorDKGRAY, 16);
        if (fRec709) {
            fImage = fImage->reinterpretColorSpace(
                    SkColorSpace::MakeRGB(SkNamedTransferFn::kRec709, SkNamedGamut::kSRGB));
            SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(SkScalarCeilToInt(fDstSize.fWidth),
                                                             SkScalarCeilToInt(fDstSize.fHeight),
                                                             SkColorSpace::MakeSRGB());
            fSrgbSurface = SkSurfaces::Raster(dstInfo);
        }
        fDst = SkRect::MakeWH(fDstSize.fWidth, fDstSize.fHeight);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkCanvas* targetCanvas = fRec709 ? fSrgbSurface->getCanvas() : canvas;
        SkPaint paint;
        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);
        for (int i = 0; i < loops; i++) {
            targetCanvas->drawImageRect(fImage, fDst, sampling, &paint);
        }
    }

private:
    SkString fName;
    sk_sp<SkImage> fImage;
    sk_sp<SkSurface> fSrgbSurface;
    SkRect fDst;
    SkISize fSrcSize;
    SkSize fDstSize;
    bool fRec709;
    using INHERITED = Benchmark;
};

DEF_BENCH(return new BilerpBench(1.00f);)
DEF_BENCH(return new BilerpBench(0.99f);)
DEF_BENCH(return new BilerpBench(0.75f);)
DEF_BENCH(return new BilerpBench(0.50f);)
DEF_BENCH(return new BilerpBench(0.33f);)
DEF_BENCH(return new BilerpBench(1920, 1080, 3840, 2160, false);)
DEF_BENCH(return new BilerpBench(1920, 1080, 3840, 2160, true);)
