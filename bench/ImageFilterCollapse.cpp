/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkTableColorFilter.h"

// Chains several matrix color filters image filter or several
// table filter image filters and draws a bitmap.
// This bench shows an improvement in performance and memory
// when collapsing matrices or tables is implemented since all
// the passes are collapsed in one.

class BaseImageFilterCollapseBench : public Benchmark {
public:
    BaseImageFilterCollapseBench() {}

protected:
    void doPreDraw(sk_sp<SkColorFilter> colorFilters[], int nFilters) {
        SkASSERT(!fImageFilter);

        // Create a chain of ImageFilters from colorFilters
        for(int i = nFilters; i --> 0;) {
            fImageFilter = SkImageFilters::ColorFilter(colorFilters[i], fImageFilter);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        makeBitmap();

        for(int i = 0; i < loops; i++) {
            SkPaint paint;
            paint.setImageFilter(fImageFilter);
            canvas->drawBitmap(fBitmap, 0, 0, &paint);
        }
    }

private:
    sk_sp<SkImageFilter> fImageFilter;
    SkBitmap fBitmap;

    void makeBitmap() {
        int W = 400;
        int H = 400;
        fBitmap.allocN32Pixels(W, H);
        fBitmap.eraseColor(SK_ColorTRANSPARENT);

        SkCanvas canvas(fBitmap);
        SkPaint paint;
        SkPoint pts[] = { {0, 0}, {SkIntToScalar(W), SkIntToScalar(H)} };
        SkColor colors[] = {
            SK_ColorBLACK, SK_ColorGREEN, SK_ColorCYAN,
            SK_ColorRED, 0, SK_ColorBLUE, SK_ColorWHITE
        };
        paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                     SkTileMode::kClamp));
        canvas.drawPaint(paint);
    }
};

class TableCollapseBench: public BaseImageFilterCollapseBench {
protected:
    const char* onGetName() override {
        return "image_filter_collapse_table";
    }

    void onDelayedSetup() override {
        for (int i = 0; i < 256; ++i) {
            int n = i >> 5;
            table1[i] = (n << 5) | (n << 2) | (n >> 1);

            table2[i] = i * i / 255;

            float fi = i / 255.0f;
            table3[i] = static_cast<uint8_t>(sqrtf(fi) * 255);
        }

        sk_sp<SkColorFilter> colorFilters[] = {
            SkTableColorFilter::Make(table1),
            SkTableColorFilter::Make(table2),
            SkTableColorFilter::Make(table3),
        };

        this->doPreDraw(colorFilters, SK_ARRAY_COUNT(colorFilters));
    }

private:
    uint8_t table1[256], table2[256], table3[256];
};

static sk_sp<SkColorFilter> make_brightness(float amount) {
    SkScalar matrix[20] = { 1, 0, 0, 0, amount,
                            0, 1, 0, 0, amount,
                            0, 0, 1, 0, amount,
                            0, 0, 0, 1, 0 };
    return SkColorFilters::Matrix(matrix);
}

static sk_sp<SkColorFilter> make_grayscale() {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::Matrix(matrix);
}

class MatrixCollapseBench: public BaseImageFilterCollapseBench {
protected:
    const char* onGetName() override {
        return "image_filter_collapse_matrix";
    }

    void onDelayedSetup() override {
        sk_sp<SkColorFilter> colorFilters[] = {
            make_brightness(0.1f),
            make_grayscale(),
            make_brightness(-0.1f),
        };

        this->doPreDraw(colorFilters, SK_ARRAY_COUNT(colorFilters));
    }
};

DEF_BENCH(return new TableCollapseBench;)
DEF_BENCH(return new MatrixCollapseBench;)
