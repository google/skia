/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkImageSource.h"

#define FILTER_WIDTH_SMALL  32
#define FILTER_HEIGHT_SMALL 32
#define FILTER_WIDTH_LARGE  256
#define FILTER_HEIGHT_LARGE 256

class DisplacementBaseBench : public Benchmark {
public:
    DisplacementBaseBench(bool small) : fInitialized(false), fIsSmall(small) { }

protected:
    void onDelayedSetup() override {
        if (!fInitialized) {
            this->makeBitmap();
            this->makeCheckerboard();
            fInitialized = true;
        }
    }

    void makeBitmap() {
        const int w = this->isSmall() ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = this->isSmall() ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
        fBitmap.allocN32Pixels(w, h);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setColor(0xFF884422);

        SkFont font;
        font.setSize(SkIntToScalar(96));
        canvas.drawSimpleText("g", 1, SkTextEncoding::kUTF8, SkIntToScalar(15), SkIntToScalar(55), font, paint);
    }

    void makeCheckerboard() {
        const int w = this->isSmall() ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = this->isSmall() ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
        auto surface(SkSurface::MakeRasterN32Premul(w, h));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF804020);
        SkPaint lightPaint;
        lightPaint.setColor(0xFF244484);
        for (int y = 0; y < h; y += 16) {
            for (int x = 0; x < w; x += 16) {
                canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas->drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas->restore();
            }
        }

        fCheckerboard = surface->makeImageSnapshot();
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

    SkBitmap fBitmap;
    sk_sp<SkImage> fCheckerboard;

private:
    bool fInitialized;
    bool fIsSmall;
    typedef Benchmark INHERITED;
};

class DisplacementZeroBench : public DisplacementBaseBench {
public:
    DisplacementZeroBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "displacement_zero_small" : "displacement_zero_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        sk_sp<SkImageFilter> displ(SkImageSource::Make(fCheckerboard));
        // No displacement effect
        paint.setImageFilter(SkDisplacementMapEffect::Make(
                                                SkDisplacementMapEffect::kR_ChannelSelectorType,
                                                SkDisplacementMapEffect::kG_ChannelSelectorType,
                                                0.0f, std::move(displ), nullptr));

        for (int i = 0; i < loops; i++) {
            this->drawClippedBitmap(canvas, 0, 0, paint);
        }
    }

private:
    typedef DisplacementBaseBench INHERITED;
};

class DisplacementAlphaBench : public DisplacementBaseBench {
public:
    DisplacementAlphaBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return isSmall() ? "displacement_alpha_small" : "displacement_alpha_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        sk_sp<SkImageFilter> displ(SkImageSource::Make(fCheckerboard));
        // Displacement, with 1 alpha component (which isn't pre-multiplied)
        paint.setImageFilter(SkDisplacementMapEffect::Make(
                                                SkDisplacementMapEffect::kB_ChannelSelectorType,
                                                SkDisplacementMapEffect::kA_ChannelSelectorType,
                                                16.0f, std::move(displ), nullptr));
        for (int i = 0; i < loops; i++) {
            this->drawClippedBitmap(canvas, 100, 0, paint);
        }
    }

private:
    typedef DisplacementBaseBench INHERITED;
};

class DisplacementFullBench : public DisplacementBaseBench {
public:
    DisplacementFullBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return isSmall() ? "displacement_full_small" : "displacement_full_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        sk_sp<SkImageFilter> displ(SkImageSource::Make(fCheckerboard));
        // Displacement, with 2 non-alpha components
        paint.setImageFilter(SkDisplacementMapEffect::Make(
                                                SkDisplacementMapEffect::kR_ChannelSelectorType,
                                                SkDisplacementMapEffect::kB_ChannelSelectorType,
                                                32.0f, std::move(displ), nullptr));
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
