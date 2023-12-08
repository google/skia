/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "tools/fonts/FontToolUtils.h"

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
        auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h));
        SkPaint paint;
        paint.setColor(0xFF884422);

        SkFont font = ToolUtils::DefaultFont();
        font.setSize(SkIntToScalar(96));
        surf->getCanvas()->drawSimpleText("g", 1, SkTextEncoding::kUTF8, 15, 55, font, paint);
        fImage = surf->makeImageSnapshot();
    }

    void makeCheckerboard() {
        const int w = this->isSmall() ? FILTER_WIDTH_SMALL : FILTER_WIDTH_LARGE;
        const int h = this->isSmall() ? FILTER_HEIGHT_SMALL : FILTER_HEIGHT_LARGE;
        auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h)));
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
        canvas->clipIRect(fImage->bounds().makeOffset(x, y));
        canvas->drawImage(fImage, SkIntToScalar(x), SkIntToScalar(y), SkSamplingOptions(), &paint);
        canvas->restore();
    }

    inline bool isSmall() const { return fIsSmall; }

    sk_sp<SkImage> fImage, fCheckerboard;

private:
    bool fInitialized;
    bool fIsSmall;
    using INHERITED = Benchmark;
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
        sk_sp<SkImageFilter> displ(SkImageFilters::Image(fCheckerboard, SkFilterMode::kLinear));
        // No displacement effect
        paint.setImageFilter(SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kG,
                                                             0.0f, std::move(displ), nullptr));

        for (int i = 0; i < loops; i++) {
            this->drawClippedBitmap(canvas, 0, 0, paint);
        }
    }

private:
    using INHERITED = DisplacementBaseBench;
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
        sk_sp<SkImageFilter> displ(SkImageFilters::Image(fCheckerboard, SkFilterMode::kLinear));
        // Displacement, with 1 alpha component (which isn't pre-multiplied)
        paint.setImageFilter(SkImageFilters::DisplacementMap(SkColorChannel::kB, SkColorChannel::kA,
                                                             16.0f, std::move(displ), nullptr));
        for (int i = 0; i < loops; i++) {
            this->drawClippedBitmap(canvas, 100, 0, paint);
        }
    }

private:
    using INHERITED = DisplacementBaseBench;
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
        sk_sp<SkImageFilter> displ(SkImageFilters::Image(fCheckerboard, SkFilterMode::kLinear));
        // Displacement, with 2 non-alpha components
        paint.setImageFilter(SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kB,
                                                             32.0f, std::move(displ), nullptr));
        for (int i = 0; i < loops; ++i) {
            this->drawClippedBitmap(canvas, 200, 0, paint);
        }
    }

private:
    using INHERITED = DisplacementBaseBench;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DisplacementZeroBench(true); )
DEF_BENCH( return new DisplacementAlphaBench(true); )
DEF_BENCH( return new DisplacementFullBench(true); )
DEF_BENCH( return new DisplacementZeroBench(false); )
DEF_BENCH( return new DisplacementAlphaBench(false); )
DEF_BENCH( return new DisplacementFullBench(false); )
