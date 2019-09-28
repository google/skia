/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256
#define BLUR_SIGMA_MINI     0.5f
#define BLUR_SIGMA_SMALL    1.0f
#define BLUR_SIGMA_LARGE    10.0f
#define BLUR_SIGMA_HUGE     80.0f


// When 'cropped' is set we apply a cropRect to the blurImageFilter. The crop rect is an inset of
// the source's natural dimensions. This is intended to exercise blurring a larger source bitmap
// to a smaller destination bitmap.

// When 'expanded' is set we apply a cropRect to the input of the blurImageFilter (a noOp
// offsetImageFilter). The crop rect in this case is an inset of the source's natural dimensions.
// An additional crop rect is applied to the blurImageFilter that is just the natural dimensions
// of the source (not inset). This is intended to exercise blurring a smaller source bitmap to a
// larger destination.

static SkBitmap make_checkerboard(int width, int height) {
    SkBitmap bm;
    bm.allocN32Pixels(width, height);
    SkCanvas canvas(bm);
    canvas.clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    for (int y = 0; y < height; y += 16) {
        for (int x = 0; x < width; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
        }
    }

    return bm;
}

class BlurImageFilterBench : public Benchmark {
public:
    BlurImageFilterBench(SkScalar sigmaX, SkScalar sigmaY,  bool small, bool cropped,
                         bool expanded)
      : fIsSmall(small)
      , fIsCropped(cropped)
      , fIsExpanded(expanded)
      , fInitialized(false)
      , fSigmaX(sigmaX)
      , fSigmaY(sigmaY) {
        fName.printf("blur_image_filter_%s%s%s_%.2f_%.2f",
            fIsSmall ? "small" : "large",
            fIsCropped ? "_cropped" : "",
            fIsExpanded ? "_expanded" : "",
            SkScalarToFloat(sigmaX), SkScalarToFloat(sigmaY));
        SkASSERT(!fIsExpanded || fIsCropped); // never want expansion w/o cropping
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        if (!fInitialized) {
            fCheckerboard = make_checkerboard(fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE,
                                              fIsSmall ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE);
            fInitialized = true;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        static const int kX = 0;
        static const int kY = 0;
        const SkIRect bmpRect = SkIRect::MakeXYWH(kX, kY, fCheckerboard.width(),
                                                  fCheckerboard.height());
        const SkIRect bmpRectInset = bmpRect.makeInset(10, 10);

        sk_sp<SkImageFilter> input = fIsExpanded
                                        ? SkImageFilters::Offset(0, 0, nullptr, &bmpRectInset)
                                        : nullptr;

        const SkIRect* crop =
            fIsExpanded ? &bmpRect : fIsCropped ? &bmpRectInset : nullptr;
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Blur(fSigmaX, fSigmaY, std::move(input), crop));

        for (int i = 0; i < loops; i++) {
            canvas->drawBitmap(fCheckerboard, kX, kY, &paint);
        }
    }

private:

    SkString fName;
    bool fIsSmall;
    bool fIsCropped;
    bool fIsExpanded;
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkScalar fSigmaX, fSigmaY;
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, 0, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, 0, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_LARGE, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_SMALL, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, true, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, true, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, false, false, false);)

DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, 0, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, 0, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_LARGE, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_SMALL, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, true, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, true, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, false, true, false);)

DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, 0, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, 0, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_LARGE, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_SMALL, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, true, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, true, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, false, true, true);)
