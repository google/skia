/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkLumaColorFilter.h"
#include "SkTableColorFilter.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

class ColorFilterBaseBench : public SkBenchmark {

public:
    ColorFilterBaseBench(bool small) : fIsSmall(small) { }

protected:
    SkRect getFilterRect() const {
        return isSmall() ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                           SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
    }

    static SkImageFilter* make_brightness(float amount, SkImageFilter* input = NULL) {
        SkScalar amount255 = SkScalarMul(amount, SkIntToScalar(255));
        SkScalar matrix[20] = { 1, 0, 0, 0, amount255,
                                0, 1, 0, 0, amount255,
                                0, 0, 1, 0, amount255,
                                0, 0, 0, 1, 0 };
        SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
        return SkColorFilterImageFilter::Create(filter, input);
    }

    static SkImageFilter* make_grayscale(SkImageFilter* input = NULL) {
        SkScalar matrix[20];
        memset(matrix, 0, 20 * sizeof(SkScalar));
        matrix[0] = matrix[5] = matrix[10] = 0.2126f;
        matrix[1] = matrix[6] = matrix[11] = 0.7152f;
        matrix[2] = matrix[7] = matrix[12] = 0.0722f;
        matrix[18] = 1.0f;
        SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
        return SkColorFilterImageFilter::Create(filter, input);
    }

    static SkImageFilter* make_mode_blue(SkImageFilter* input = NULL) {
        SkAutoTUnref<SkColorFilter> filter(
            SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
        return SkColorFilterImageFilter::Create(filter, input);
    }

    inline bool isSmall() const { return fIsSmall; }
private:
    bool fIsSmall;

    typedef SkBenchmark INHERITED;
};

class ColorFilterDimBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterDimBrightBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_dim_bright_small" : "colorfilter_dim_bright_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        for (int i = 0; i < loops; i++) {
            for (float brightness = -1.0f; brightness <= 1.0f; brightness += 0.4f) {
                SkAutoTUnref<SkImageFilter> dim(make_brightness(-brightness));
                SkAutoTUnref<SkImageFilter> bright(make_brightness(brightness, dim));
                paint.setImageFilter(bright);
                canvas->drawRect(r, paint);
            }
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightGrayBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightGrayBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_bright_gray_small" : "colorfilter_bright_gray_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(0.9f));
            SkAutoTUnref<SkImageFilter> grayscale(make_grayscale(brightness));
            paint.setImageFilter(grayscale);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterGrayBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterGrayBrightBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_gray_bright_small" : "colorfilter_gray_bright_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> grayscale(make_grayscale());
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(0.9f, grayscale));
            paint.setImageFilter(brightness);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBlueBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterBlueBrightBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_blue_bright_small" : "colorfilter_blue_bright_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue());
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(1.0f, blue));
            paint.setImageFilter(brightness);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightBlueBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightBlueBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_bright_blue_small" : "colorfilter_bright_blue_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(1.0f));
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue(brightness));
            paint.setImageFilter(blue);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBrightBench : public ColorFilterBaseBench {

public:
    ColorFilterBrightBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_bright_small" : "colorfilter_bright_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(1.0f));
            paint.setImageFilter(brightness);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterBlueBench : public ColorFilterBaseBench {

public:
    ColorFilterBlueBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_blue_small" : "colorfilter_blue_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue());
            paint.setImageFilter(blue);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class ColorFilterGrayBench : public ColorFilterBaseBench {

public:
    ColorFilterGrayBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "colorfilter_gray_small" : "colorfilter_gray_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkImageFilter> grayscale(make_grayscale());
            paint.setImageFilter(grayscale);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef ColorFilterBaseBench INHERITED;
};

class TableColorFilterBench : public ColorFilterBaseBench {

public:
    TableColorFilterBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "table_colorfilter_small" : "table_colorfilter_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkColorFilter> table_filter(make_table_filter());
            paint.setColorFilter(table_filter);
            canvas->drawRect(r, paint);
        }
    }

private:
    static void fill_table_data(uint8_t table[]) {
        for (int i = 0; i < 256; ++i) {
            int n = i >> 5;
            table[i] = (n << 5) | (n << 2) | (n >> 1);
        }
    }

    static SkColorFilter* make_table_filter() {
        uint8_t table[256]; fill_table_data(table);
        return SkTableColorFilter::Create(table);
    }

    typedef ColorFilterBaseBench INHERITED;
};

class LumaColorFilterBench : public ColorFilterBaseBench {

public:
    LumaColorFilterBench(bool small) : INHERITED(small) {
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return isSmall() ? "luma_colorfilter_small" : "luma_colorfilter_large";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRect r = getFilterRect();
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkColorFilter> luma_filter(SkLumaColorFilter::Create());
            paint.setColorFilter(luma_filter);
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
DEF_BENCH( return new TableColorFilterBench(true); )
DEF_BENCH( return new LumaColorFilterBench(true); )

DEF_BENCH( return new ColorFilterDimBrightBench(false); )
DEF_BENCH( return new ColorFilterBrightGrayBench(false); )
DEF_BENCH( return new ColorFilterGrayBrightBench(false); )
DEF_BENCH( return new ColorFilterBlueBrightBench(false); )
DEF_BENCH( return new ColorFilterBrightBlueBench(false); )
DEF_BENCH( return new ColorFilterBrightBench(false); )
DEF_BENCH( return new ColorFilterBlueBench(false); )
DEF_BENCH( return new ColorFilterGrayBench(false); )
DEF_BENCH( return new TableColorFilterBench(false); )
DEF_BENCH( return new LumaColorFilterBench(false); )
