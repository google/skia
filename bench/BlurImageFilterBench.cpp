/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256
#define BLUR_SIGMA_SMALL    SkFloatToScalar(1.0f)
#define BLUR_SIGMA_LARGE    SkFloatToScalar(10.0f)

class BlurImageFilterBench : public SkBenchmark {
public:
    BlurImageFilterBench(void* param, SkScalar sigmaX, SkScalar sigmaY,  bool small) :
        INHERITED(param), fIsSmall(small), fInitialized(false), fSigmaX(sigmaX), fSigmaY(sigmaY) {
        fName.printf("blur_image_filter_%s_%.2f_%.2f", fIsSmall ? "small" : "large",
            SkScalarToFloat(sigmaX), SkScalarToFloat(sigmaY));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            make_checkerboard();
            fInitialized = true;
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setImageFilter(new SkBlurImageFilter(fSigmaX, fSigmaY))->unref();
        canvas->drawBitmap(fCheckerboard, 0, 0, &paint);
    }

private:
    void make_checkerboard() {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_LARGE : FILTER_HEIGHT_LARGE;
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        fCheckerboard.allocPixels();
        SkDevice device(fCheckerboard);
        SkCanvas canvas(&device);
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
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkScalar fSigmaX, fSigmaY;
    typedef SkBenchmark INHERITED;
};

DEF_BENCH(return new BlurImageFilterBench(p, BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, true);)
DEF_BENCH(return new BlurImageFilterBench(p, BLUR_SIGMA_SMALL, BLUR_SIGMA_SMALL, false);)
DEF_BENCH(return new BlurImageFilterBench(p, BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, true);)
DEF_BENCH(return new BlurImageFilterBench(p, BLUR_SIGMA_LARGE, BLUR_SIGMA_LARGE, false);)
