/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkMergeImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkXfermodeImageFilter.h"
#include "tools/Resources.h"

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

class ImageMakeWithFilterDAGBench : public Benchmark {
public:
    ImageMakeWithFilterDAGBench() {}

protected:
    const char* onGetName() override {
        return "image_make_with_filter_dag";
    }

    void onDelayedSetup() override {
        fImage = GetResourceAsImage("images/mandrill_512.png");
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkIRect subset = SkIRect::MakeSize(fImage->dimensions());
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkIRect discardSubset;
        sk_sp<SkImage> image = fImage;

        for (int j = 0; j < loops; j++) {
            sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(20.0f, 20.0f, nullptr));
            sk_sp<SkImageFilter> inputs[kNumInputs];
            for (int i = 0; i < kNumInputs; ++i) {
                inputs[i] = blur;
            }
            sk_sp<SkImageFilter> mergeFilter = SkMergeImageFilter::Make(inputs, kNumInputs);
            image = image->makeWithFilter(mergeFilter.get(), subset, subset, &discardSubset,
                                          &offset);
            SkASSERT(image && image->dimensions() == fImage->dimensions());
        }
    }

private:
    static const int kNumInputs = 5;
    sk_sp<SkImage> fImage;

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

// Exercise an Xfermode kSrcIn filter compositing two inputs which have a small intersection.
class ImageFilterXfermodeIn : public Benchmark {
public:
    ImageFilterXfermodeIn() {}

protected:
    const char* onGetName() override { return "image_filter_xfermode_in"; }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int j = 0; j < loops; j++) {
            auto blur = SkBlurImageFilter::Make(20.0f, 20.0f, nullptr);
            auto offset1 = SkOffsetImageFilter::Make(100.0f, 100.0f, blur);
            auto offset2 = SkOffsetImageFilter::Make(-100.0f, -100.0f, blur);
            auto xfermode =
                    SkXfermodeImageFilter::Make(SkBlendMode::kSrcIn, offset1, offset2, nullptr);

            SkPaint paint;
            paint.setImageFilter(xfermode);
            canvas->drawRect(SkRect::MakeWH(200.0f, 200.0f), paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ImageFilterDAGBench;)
DEF_BENCH(return new ImageMakeWithFilterDAGBench;)
DEF_BENCH(return new ImageFilterDisplacedBlur;)
DEF_BENCH(return new ImageFilterXfermodeIn;)
