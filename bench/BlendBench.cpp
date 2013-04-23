/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmapSource.h"
#include "SkBlendImageFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"

#define FILTER_WIDTH  256
#define FILTER_HEIGHT 256

class BlendBaseBench : public SkBenchmark {
public:
    BlendBaseBench(void* param) : INHERITED(param), fInitialized(false) {
    }

protected:
    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), 0,
            SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, SkIntToScalar(x), 0, &paint);
        canvas->restore();
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, FILTER_WIDTH, FILTER_HEIGHT);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, FILTER_WIDTH, FILTER_HEIGHT);
        fCheckerboard.allocPixels();
        SkDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < FILTER_HEIGHT; y += 16) {
          for (int x = 0; x < FILTER_WIDTH; x += 16) {
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

    typedef SkBenchmark INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

class BlendNormalBench : public BlendBaseBench {
public:
    BlendNormalBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "blend_normal";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter,
                (SkBlendImageFilter::kNormal_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 0);
    }

private:
    typedef BlendBaseBench INHERITED;
};

class BlendMultiplyBench : public BlendBaseBench {
public:
    BlendMultiplyBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "blend_multiply";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter,
                (SkBlendImageFilter::kMultiply_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 100);
    }

private:
    typedef BlendBaseBench INHERITED;
};

class BlendScreenBench : public BlendBaseBench {
public:
    BlendScreenBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "blend_screen";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter,
                (SkBlendImageFilter::kScreen_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 200);
    }

private:
    typedef BlendBaseBench INHERITED;
};

class BlendDarkenBench : public BlendBaseBench {
public:
    BlendDarkenBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "blend_darken";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter,
                (SkBlendImageFilter::kDarken_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 300);
    }

private:
    typedef BlendBaseBench INHERITED;
};

class BlendLightenBench : public BlendBaseBench {
public:
    BlendLightenBench(void* param) : INHERITED(param) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "blend_lighten";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter,
                (SkBlendImageFilter::kLighten_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 400);
    }

private:
    typedef BlendBaseBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new BlendNormalBench(p); )
DEF_BENCH( return new BlendMultiplyBench(p); )
DEF_BENCH( return new BlendScreenBench(p); )
DEF_BENCH( return new BlendDarkenBench(p); )
DEF_BENCH( return new BlendLightenBench(p); )

