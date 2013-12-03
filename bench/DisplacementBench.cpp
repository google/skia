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
#include "SkDisplacementMapEffect.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256

class DisplacementBaseBench : public SkBenchmark {
public:
    DisplacementBaseBench(bool small) :
        fInitialized(false), fIsSmall(small) {
    }

protected:
    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            this->makeBitmap();
            this->makeCheckerboard();
            fInitialized = true;
        }
    }

    void makeBitmap() {
        const int w = this->isSmall() ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = this->isSmall() ? FILTER_HEIGHT_LARGE : FILTER_HEIGHT_LARGE;
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
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

    void makeCheckerboard() {
        const int w = this->isSmall() ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = this->isSmall() ? FILTER_HEIGHT_LARGE : FILTER_HEIGHT_LARGE;
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

    void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                          SkIntToScalar(fBitmap.width()),
                                          SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, SkIntToScalar(x), SkIntToScalar(y), &paint);
        canvas->restore();
    }

    inline bool isSmall() const { return fIsSmall; }

    SkBitmap fBitmap, fCheckerboard;
private:
    bool fInitialized;
    bool fIsSmall;
    typedef SkBenchmark INHERITED;
};

class DisplacementZeroBench : public DisplacementBaseBench {
public:
    DisplacementZeroBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return this->isSmall() ? "displacement_zero_small" : "displacement_zero_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> displ(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        // No displacement effect
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kG_ChannelSelectorType, 0.0f, displ)))->unref();

        for (int i = 0; i < loops; i++) {
            this->drawClippedBitmap(canvas, 0, 0, paint);
        }
    }

private:
    typedef DisplacementBaseBench INHERITED;
};

class DisplacementAlphaBench : public DisplacementBaseBench {
public:
    DisplacementAlphaBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "displacement_alpha_small" : "displacement_alpha_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> displ(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        // Displacement, with 1 alpha component (which isn't pre-multiplied)
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kB_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 16.0f, displ)))->unref();
        for (int i = 0; i < loops; i++) {
            drawClippedBitmap(canvas, 100, 0, paint);
        }
    }

private:
    typedef DisplacementBaseBench INHERITED;
};

class DisplacementFullBench : public DisplacementBaseBench {
public:
    DisplacementFullBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "displacement_full_small" : "displacement_full_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> displ(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        // Displacement, with 2 non-alpha components
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kB_ChannelSelectorType, 32.0f, displ)))->unref();
        for (int i = 0; i < loops; ++i) {
            this->drawClippedBitmap(canvas, 200, 0, paint);
        }
    }

private:
    typedef DisplacementBaseBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DisplacementZeroBench(true); )
DEF_BENCH( return new DisplacementAlphaBench(true); )
DEF_BENCH( return new DisplacementFullBench(true); )
DEF_BENCH( return new DisplacementZeroBench(false); )
DEF_BENCH( return new DisplacementAlphaBench(false); )
DEF_BENCH( return new DisplacementFullBench(false); )
