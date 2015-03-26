/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkMergeImageFilter.h"

enum { kNumInputs = 5 };

// Exercise a blur filter connected to 5 inputs of the same merge filter.
// This bench shows an improvement in performance once cacheing of re-used
// nodes is implemented, since the DAG is no longer flattened to a tree.

class ImageFilterDAGBench : public Benchmark {
public:
    ImageFilterDAGBench() {}

protected:
    const char* onGetName() override {
        return "image_filter_dag";
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        for (int j = 0; j < loops; j++) {
            SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(20.0f, 20.0f));
            SkImageFilter* inputs[kNumInputs];
            for (int i = 0; i < kNumInputs; ++i) {
                inputs[i] = blur.get();
            }
            SkAutoTUnref<SkImageFilter> merge(SkMergeImageFilter::Create(inputs, kNumInputs));
            SkPaint paint;
            paint.setImageFilter(merge);
            SkRect rect = SkRect::Make(SkIRect::MakeWH(400, 400));
            canvas->drawRect(rect, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ImageFilterDAGBench;)
