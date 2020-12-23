/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"

static sk_sp<SkImage> make_image(int w, int h) {
    auto surf = SkSurface::MakeRaster(SkImageInfo::Make(w, h,
                                                        kN32_SkColorType, kOpaque_SkAlphaType));
    auto canvas = surf->getCanvas();
    canvas->clear(SK_ColorBLACK);

    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorRED);
    canvas->drawCircle(SkIntToScalar(w)/2, SkIntToScalar(h)/2,
                      SkIntToScalar(std::min(w, h))*3/8, p);

    SkRect r;
    r.setWH(SkIntToScalar(w), SkIntToScalar(h));
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(4));
    p.setColor(SK_ColorBLUE);
    canvas->drawRect(r, p);
    return surf->makeImageSnapshot();
}

/*  Variants for bitmaprect
    src : entire bitmap, subset, fractional subset
    dst : same size as src, diff size
    paint : filter-p
 */

class BitmapRectBench : public Benchmark {
    sk_sp<SkImage>          fImage;
    bool                    fSlightMatrix;
    uint8_t                 fAlpha;
    SkSamplingOptions       fSampling;
    SkString                fName;
    SkRect                  fSrcR, fDstR;

    static const int kWidth = 128;
    static const int kHeight = 128;
public:
    BitmapRectBench(U8CPU alpha, SkFilterQuality filterQuality, bool slightMatrix)
        : fSlightMatrix(slightMatrix)
        , fAlpha(SkToU8(alpha))
        , fSampling(filterQuality)
    {}

protected:
    const char* onGetName() override {
        fName.printf("imagerect_%02X_%sfilter_%s",
                     fAlpha,
                     fSampling == SkSamplingOptions() ? "no" : "",
                     fSlightMatrix ? "trans" : "identity");
        return fName.c_str();
    }

    void onDelayedSetup() override {
        fImage = make_image(kWidth, kHeight);

        fSrcR.setWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        fDstR.setWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        if (fSlightMatrix) {
            // want fractional translate
            fDstR.offset(SK_Scalar1 / 3, SK_Scalar1 * 5 / 7);
            // want enough to create a scale matrix, but not enough to scare
            // off our sniffer which tries to see if the matrix is "effectively"
            // translate-only.
            fDstR.fRight += SK_Scalar1 / (kWidth * 60);
        }
    }


    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;

        SkPaint paint;
        this->setupPaint(&paint);
        paint.setAlpha(fAlpha);

        for (int i = 0; i < loops; i++) {
            canvas->drawImageRect(fImage.get(), fSrcR, fDstR, fSampling, &paint,
                                  SkCanvas::kStrict_SrcRectConstraint);
        }
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH(return new BitmapRectBench(0xFF, kNone_SkFilterQuality, false))
DEF_BENCH(return new BitmapRectBench(0x80, kNone_SkFilterQuality, false))
DEF_BENCH(return new BitmapRectBench(0xFF, kLow_SkFilterQuality, false))
DEF_BENCH(return new BitmapRectBench(0x80, kLow_SkFilterQuality, false))

DEF_BENCH(return new BitmapRectBench(0xFF, kNone_SkFilterQuality, true))
DEF_BENCH(return new BitmapRectBench(0xFF, kLow_SkFilterQuality, true))
