/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkMagnifierImageFilter.h"
#include "SkRandom.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256

class MagnifierBench : public SkBenchmark {
public:
    MagnifierBench(bool small) :
        fIsSmall(small), fInitialized(false) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "magnifier_small" : "magnifier_large";
    }

    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            make_checkerboard();
            fInitialized = true;
        }
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
        SkPaint paint;
        paint.setImageFilter(
            new SkMagnifierImageFilter(
                SkRect::MakeXYWH(SkIntToScalar(w / 4),
                                 SkIntToScalar(h / 4),
                                 SkIntToScalar(w / 2),
                                 SkIntToScalar(h / 2)), 100))->unref();

        for (int i = 0; i < loops; i++) {
            canvas->drawBitmap(fCheckerboard, 0, 0, &paint);
        }
    }

private:
    void make_checkerboard() {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_LARGE : FILTER_HEIGHT_LARGE;
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        fCheckerboard.allocPixels();
        SkBitmapDevice device(fCheckerboard);
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

    bool fIsSmall;
    bool fInitialized;
    SkBitmap fCheckerboard;
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MagnifierBench(true); )
DEF_BENCH( return new MagnifierBench(false); )
