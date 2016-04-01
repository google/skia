/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImageSource.h"
#include "SkMergeImageFilter.h"
#include "SkSurface.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

static sk_sp<SkImage> make_bitmap() {
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(80, 80));
    surface->getCanvas()->clear(0x00000000);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF884422);
    paint.setTextSize(SkIntToScalar(96));
    const char* str = "g";
    surface->getCanvas()->drawText(str, strlen(str), 15, 55, paint);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_checkerboard() {
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(80, 80));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    for (int y = 0; y < 80; y += 16) {
        for (int x = 0; x < 80; x += 16) {
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas->restore();
        }
    }

    return surface->makeImageSnapshot();
}

class MergeBench : public Benchmark {
public:
    MergeBench(bool small) : fIsSmall(small), fInitialized(false) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "merge_small" : "merge_large";
    }

    void onDelayedSetup() override {
        if (!fInitialized) {
            fImage = make_bitmap();
            fCheckerboard = make_checkerboard();
            fInitialized = true;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = fIsSmall ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                              SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
        SkPaint paint;
        paint.setImageFilter(this->mergeBitmaps());
        for (int i = 0; i < loops; i++) {
            canvas->drawRect(r, paint);
        }
    }

private:
    sk_sp<SkImageFilter> mergeBitmaps() {
        return SkMergeImageFilter::Make(SkImageSource::Make(fCheckerboard),
                                        SkImageSource::Make(fImage));
    }

    bool fIsSmall;
    bool fInitialized;
    sk_sp<SkImage> fImage, fCheckerboard;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MergeBench(true); )
DEF_BENCH( return new MergeBench(false); )
