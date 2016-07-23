/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlurImageFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkCanvas.h"
#include "SkMergeImageFilter.h"


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

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect rect = SkRect::Make(SkIRect::MakeWH(400, 400));

        for (int j = 0; j < loops; j++) {
            sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(20.0f, 20.0f, nullptr));
            sk_sp<SkImageFilter> inputs[kNumInputs];
            for (int i = 0; i < kNumInputs; ++i) {
                inputs[i] = blur;
            }
            SkPaint paint;
            paint.setImageFilter(SkMergeImageFilter::Make(inputs, kNumInputs));
            canvas->drawRect(rect, paint);
        }
    }

private:
    static const int kNumInputs = 5;

    typedef Benchmark INHERITED;
};

// Exercise a blur filter connected to both inputs of an SkDisplacementMapEffect.

class ImageFilterDisplacedBlur : public Benchmark {
public:
    ImageFilterDisplacedBlur() {}

protected:
    const char* onGetName() override {
        return "image_filter_displaced_blur";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int j = 0; j < loops; j++) {
            sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(4.0f, 4.0f, nullptr));
            auto xSelector = SkDisplacementMapEffect::kR_ChannelSelectorType;
            auto ySelector = SkDisplacementMapEffect::kB_ChannelSelectorType;
            SkScalar scale = 2;

            SkPaint paint;
            paint.setImageFilter(SkDisplacementMapEffect::Make(xSelector, ySelector, scale,
                                                               blur, blur));

            SkRect rect = SkRect::Make(SkIRect::MakeWH(400, 400));
            canvas->drawRect(rect, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ImageFilterDAGBench;)
DEF_BENCH(return new ImageFilterDisplacedBlur;)
