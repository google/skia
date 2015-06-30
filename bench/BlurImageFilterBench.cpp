/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256
#define BLUR_SIGMA_MINI     0.5f
#define BLUR_SIGMA_SMALL    1.0f
#define BLUR_SIGMA_LARGE    10.0f
#define BLUR_SIGMA_HUGE     80.0f

class BlurImageFilterBench : public Benchmark {
public:
    BlurImageFilterBench(SkScalar sigmaX, SkScalar sigmaY,  bool small, bool cropped) :
        fIsSmall(small), fIsCropped(cropped), fInitialized(false), fSigmaX(sigmaX), fSigmaY(sigmaY) {
        fName.printf("blur_image_filter_%s%s_%.2f_%.2f", fIsSmall ? "small" : "large",
            fIsCropped ? "_cropped" : "", SkScalarToFloat(sigmaX), SkScalarToFloat(sigmaY));
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPreDraw() override {
        if (!fInitialized) {
            make_checkerboard();
            fInitialized = true;
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkPaint paint;
        static const SkScalar kX = 0;
        static const SkScalar kY = 0;
        const SkRect bmpRect = SkRect::MakeXYWH(kX, kY,
                                                SkIntToScalar(fCheckerboard.width()),
                                                SkIntToScalar(fCheckerboard.height()));
        const SkImageFilter::CropRect cropRect =
                                        SkImageFilter::CropRect(bmpRect.makeInset(10.f, 10.f));
        const SkImageFilter::CropRect* crop = fIsCropped ? &cropRect : NULL;

        paint.setImageFilter(SkBlurImageFilter::Create(fSigmaX, fSigmaY, NULL, crop))->unref();

        for (int i = 0; i < loops; i++) {
            canvas->drawBitmap(fCheckerboard, kX, kY, &paint);
        }
    }

private:
    void make_checkerboard() {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_LARGE : FILTER_HEIGHT_LARGE;
        fCheckerboard.allocN32Pixels(w, h);
        SkCanvas canvas(fCheckerboard);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF804020);
        SkPaint lightPaint;
        lightPaint.setColor(0xFF244484);
        for (int y = 0; y < h; y += 16) {
            for (int x = 0; x < w; x += 16) {
                canvas.save();
                canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas.restore();
            }
        }
    }

    SkString fName;
    bool fIsSmall;
    bool fIsCropped;
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkScalar fSigmaX, fSigmaY;
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, 0, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, 0, false, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_LARGE, false, false);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_SMALL, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, true, false);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, false, false);)

DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, 0, false, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, 0, false, true);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_LARGE, false, true);)
DEF_BENCH(return new BlurImageFilterBench(0, BLUR_SIGMA_SMALL, false, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_MINI, BLUR_SIGMA_MINI, false, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, true, true);)
DEF_BENCH(return new BlurImageFilterBench(BLUR_SIGMA_HUGE, BLUR_SIGMA_HUGE, false, true);)
