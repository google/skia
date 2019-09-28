/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256

class MagnifierBench : public Benchmark {
public:
    MagnifierBench(bool small) :
        fIsSmall(small), fInitialized(false) {
    }

protected:
    const char* onGetName() override {
        return fIsSmall ? "magnifier_small" : "magnifier_large";
    }

    void onDelayedSetup() override {
        if (!fInitialized) {
            make_checkerboard();
            fInitialized = true;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
        SkPaint paint;
        paint.setImageFilter(
            SkImageFilters::Magnifier(
                SkRect::MakeXYWH(SkIntToScalar(w / 4),
                                 SkIntToScalar(h / 4),
                                 SkIntToScalar(w / 2),
                                 SkIntToScalar(h / 2)), 100, nullptr));

        for (int i = 0; i < loops; i++) {
            canvas->drawBitmap(fCheckerboard, 0, 0, &paint);
        }
    }

private:
    void make_checkerboard() {
        const int w = fIsSmall ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = fIsSmall ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
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

    bool fIsSmall;
    bool fInitialized;
    SkBitmap fCheckerboard;
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MagnifierBench(true); )
DEF_BENCH( return new MagnifierBench(false); )
