/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmapSource.h"
#include "SkCanvas.h"
#include "SkMergeImageFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

class MergeBench : public Benchmark {
public:
    MergeBench(bool small) : fIsSmall(small), fInitialized(false) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "merge_small" : "merge_large";
    }

    void onPreDraw() override {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkRect r = fIsSmall ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                              SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
        SkPaint paint;
        paint.setImageFilter(mergeBitmaps())->unref();
        for (int i = 0; i < loops; i++) {
            canvas->drawRect(r, paint);
        }
    }

private:
    SkImageFilter* mergeBitmaps() {
        SkImageFilter* first = SkBitmapSource::Create(fCheckerboard);
        SkImageFilter* second = SkBitmapSource::Create(fBitmap);
        SkAutoUnref aur0(first);
        SkAutoUnref aur1(second);
        return SkMergeImageFilter::Create(first, second);
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFF884422);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "g";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(55), paint);
    }

    void make_checkerboard() {
        fCheckerboard.allocN32Pixels(80, 80);
        SkCanvas canvas(fCheckerboard);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF804020);
        SkPaint lightPaint;
        lightPaint.setColor(0xFF244484);
        for (int y = 0; y < 80; y += 16) {
            for (int x = 0; x < 80; x += 16) {
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
    SkBitmap fBitmap, fCheckerboard;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MergeBench(true); )
DEF_BENCH( return new MergeBench(false); )
