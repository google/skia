/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkGradientShader.h"
#include "SkImageFilter.h"
#include "SkTableColorFilter.h"

// Chains several matrix color filters image filter or several
// table filter image filters and draws a bitmap.
// This bench shows an improvement in performance and memory
// when collapsing matrices or tables is implemented since all
// the passes are collapsed in one.

class BaseImageFilterCollapseBench : public Benchmark {
public:
    BaseImageFilterCollapseBench(): fImageFilter(NULL) {}
    ~BaseImageFilterCollapseBench() {
        SkSafeUnref(fImageFilter);
    }

protected:
    void doPreDraw(SkColorFilter* colorFilters[], int nFilters) {
        // Create a chain of ImageFilters from colorFilters
        fImageFilter = NULL;
        for(int i = nFilters; i --> 0;) {
            SkAutoTUnref<SkImageFilter> filter(
                        SkColorFilterImageFilter::Create(colorFilters[i], fImageFilter, NULL)
            );
            SkRefCnt_SafeAssign(fImageFilter, filter.get());
        }
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        makeBitmap();

        for(int i = 0; i < loops; i++) {
            SkPaint paint;
            paint.setImageFilter(fImageFilter);
            canvas->drawBitmap(fBitmap, 0, 0, &paint);
        }
    }

private:
    SkImageFilter* fImageFilter;
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
        SkAutoTUnref<SkShader> shader(SkGradientShader::CreateLinear(
                    pts, colors, NULL, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode
        ));
        paint.setShader(shader);
        canvas.drawPaint(paint);
    }
};

class TableCollapseBench: public BaseImageFilterCollapseBench {
public:
    virtual ~TableCollapseBench() {}

protected:
    virtual const char* onGetName() override {
        return "image_filter_collapse_table";
    }

    virtual void onPreDraw() override {
        for (int i = 0; i < 256; ++i) {
            int n = i >> 5;
            table1[i] = (n << 5) | (n << 2) | (n >> 1);

            table2[i] = i * i / 255;

            float fi = i / 255.0f;
            table3[i] = static_cast<uint8_t>(sqrtf(fi) * 255);
        }

        SkColorFilter* colorFilters[] = {
            SkTableColorFilter::Create(table1),
            SkTableColorFilter::Create(table2),
            SkTableColorFilter::Create(table3),
        };

        doPreDraw(colorFilters, SK_ARRAY_COUNT(colorFilters));

        for(unsigned i = 0; i < SK_ARRAY_COUNT(colorFilters); i++) {
            colorFilters[i]->unref();
        }
    }

private:
    uint8_t table1[256], table2[256], table3[256];
};

static SkColorFilter* make_brightness(float amount) {
    SkScalar amount255 = SkScalarMul(amount, SkIntToScalar(255));
    SkScalar matrix[20] = { 1, 0, 0, 0, amount255,
                            0, 1, 0, 0, amount255,
                            0, 0, 1, 0, amount255,
                            0, 0, 0, 1, 0 };
    return SkColorMatrixFilter::Create(matrix);
}

static SkColorFilter* make_grayscale() {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorMatrixFilter::Create(matrix);
}

class MatrixCollapseBench: public BaseImageFilterCollapseBench {
public:
    virtual ~MatrixCollapseBench() {}

protected:
    virtual const char* onGetName() override {
        return "image_filter_collapse_matrix";
    }

    virtual void onPreDraw() override {
        SkColorFilter* colorFilters[] = {
            make_brightness(0.1f),
            make_grayscale(),
            make_brightness(-0.1f),
        };

        doPreDraw(colorFilters, SK_ARRAY_COUNT(colorFilters));

        for(unsigned i = 0; i < SK_ARRAY_COUNT(colorFilters); i++) {
            colorFilters[i]->unref();
        }
    }
};

DEF_BENCH(return new TableCollapseBench;)
DEF_BENCH(return new MatrixCollapseBench;)
