/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkColorFilterImageFilter.h"

// Just need an interesting filter, nothing to special about colormatrix
static sk_sp<SkColorFilter> make_grayscale() {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::Matrix(matrix);
}

/**
 *  Different ways to draw the same thing (a red rect)
 *  All of their timings should be about the same
 *  (we allow for slight overhead to figure out that we can undo the presence of the filters)
 */
class FilteredRectBench : public Benchmark {
public:
    enum Type {
        kNoFilter_Type,
        kColorFilter_Type,
        kImageFilter_Type,
    };

    FilteredRectBench(Type t) : fType(t) {
        static const char* suffix[] = { "nofilter", "colorfilter", "imagefilter" };
        fName.printf("filteredrect_%s", suffix[t]);
        fPaint.setColor(SK_ColorRED);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        switch (fType) {
            case kNoFilter_Type:
                break;
            case kColorFilter_Type:
                fPaint.setColorFilter(make_grayscale());
                break;
            case kImageFilter_Type:
                fPaint.setImageFilter(SkColorFilterImageFilter::Make(make_grayscale(), nullptr));
            break;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect r = { 0, 0, 256, 256 };
        for (int i = 0; i < loops; ++i) {
            canvas->drawRect(r, fPaint);
        }
    }

private:
    SkPaint  fPaint;
    SkString fName;
    Type     fType;

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kNoFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kColorFilter_Type); )
DEF_BENCH( return new FilteredRectBench(FilteredRectBench::kImageFilter_Type); )
