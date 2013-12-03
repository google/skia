/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkCanvas.h"
#include "SkMergeImageFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

class MergeBench : public SkBenchmark {
public:
    MergeBench(bool small) : fIsSmall(small), fInitialized(false) { }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fIsSmall ? "merge_small" : "merge_large";
    }

    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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
        SkImageFilter* first = new SkBitmapSource(fCheckerboard);
        SkImageFilter* second = new SkBitmapSource(fBitmap);
        SkAutoUnref aur0(first);
        SkAutoUnref aur1(second);
        return new SkMergeImageFilter(first, second);
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFF884422);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "g";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(55), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fCheckerboard.allocPixels();
        SkBitmapDevice device(fCheckerboard);
        SkCanvas canvas(&device);
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

    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MergeBench(true); )
DEF_BENCH( return new MergeBench(false); )
