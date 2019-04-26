/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkColorMatrixFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

static sk_sp<SkImageFilter> make_brightness(float amount, sk_sp<SkImageFilter> input) {
    float matrix[20] = { 1, 0, 0, 0, amount,
                         0, 1, 0, 0, amount,
                         0, 0, 1, 0, amount,
                         0, 0, 0, 1, 0 };
    sk_sp<SkColorFilter> filter(SkColorFilters::Matrix(matrix));
    return SkColorFilterImageFilter::Make(std::move(filter), std::move(input));
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input) {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    sk_sp<SkColorFilter> filter(SkColorFilters::Matrix(matrix));
    return SkColorFilterImageFilter::Make(std::move(filter), std::move(input));
}

static sk_sp<SkImageFilter> make_mode_blue(sk_sp<SkImageFilter> input) {
    sk_sp<SkColorFilter> filter(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kSrcIn));
    return SkColorFilterImageFilter::Make(std::move(filter), std::move(input));
}

class ColorFilterBaseBench : public Benchmark {

public:
    ColorFilterBaseBench(bool small) : fIsSmall(small) { }

protected:
    SkRect getFilterRect() const {
        return this->isSmall() ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                                 SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
    }

    inline bool isSmall() const { return fIsSmall; }

private:
    bool fIsSmall;

    typedef Benchmark INHERITED;
};

class ColorFilterDimBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterDimBrightBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_dim_bright_small" : "colorfilter_dim_bright_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        for (int i = 0; i < loops; i++) {
            for (float brightness = -1.0f; brightness <= 1.0f; brightness += 0.4f) {
                sk_sp<SkImageFilter> dim(make_brightness(-brightness, nullptr));
                paint.setImageFilter(make_brightness(brightness, std::move(dim)));
                canvas->drawRect(r, paint);
            }
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightGrayBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightGrayBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_bright_gray_small" : "colorfilter_bright_gray_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            sk_sp<SkImageFilter> brightness(make_brightness(0.9f, nullptr));
            paint.setImageFilter(make_grayscale(std::move(brightness)));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterGrayBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterGrayBrightBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_gray_bright_small" : "colorfilter_gray_bright_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            sk_sp<SkImageFilter> grayscale(make_grayscale(nullptr));
            paint.setImageFilter(make_brightness(0.9f, std::move(grayscale)));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBlueBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterBlueBrightBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_blue_bright_small" : "colorfilter_blue_bright_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            sk_sp<SkImageFilter> blue(make_mode_blue(nullptr));
            paint.setImageFilter(make_brightness(1.0f, std::move(blue)));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightBlueBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightBlueBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_bright_blue_small" : "colorfilter_bright_blue_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            sk_sp<SkImageFilter> brightness(make_brightness(1.0f, nullptr));
            paint.setImageFilter(make_mode_blue(std::move(brightness)));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_bright_small" : "colorfilter_bright_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            paint.setImageFilter(make_brightness(1.0f, nullptr));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBlueBench : public ColorFilterBaseBench {

public:
    ColorFilterBlueBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_blue_small" : "colorfilter_blue_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            paint.setImageFilter(make_mode_blue(nullptr));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterGrayBench : public ColorFilterBaseBench {

public:
    ColorFilterGrayBench(bool small) : INHERITED(small) { }

protected:
    const char* onGetName() override {
        return this->isSmall() ? "colorfilter_gray_small" : "colorfilter_gray_large";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = this->getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            paint.setImageFilter(make_grayscale(nullptr));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ColorFilterDimBrightBench(true); )
DEF_BENCH( return new ColorFilterBrightGrayBench(true); )
DEF_BENCH( return new ColorFilterGrayBrightBench(true); )
DEF_BENCH( return new ColorFilterBlueBrightBench(true); )
DEF_BENCH( return new ColorFilterBrightBlueBench(true); )
DEF_BENCH( return new ColorFilterBrightBench(true); )
DEF_BENCH( return new ColorFilterBlueBench(true); )
DEF_BENCH( return new ColorFilterGrayBench(true); )

DEF_BENCH( return new ColorFilterDimBrightBench(false); )
DEF_BENCH( return new ColorFilterBrightGrayBench(false); )
DEF_BENCH( return new ColorFilterGrayBrightBench(false); )
DEF_BENCH( return new ColorFilterBlueBrightBench(false); )
DEF_BENCH( return new ColorFilterBrightBlueBench(false); )
DEF_BENCH( return new ColorFilterBrightBench(false); )
DEF_BENCH( return new ColorFilterBlueBench(false); )
DEF_BENCH( return new ColorFilterGrayBench(false); )
