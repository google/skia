/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/effects/SkImageFilters.h"
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

        // Set up the filters once, we're not interested in measuring allocation time here
        sk_sp<SkImageFilter> blur(SkImageFilters::Blur(20.0f, 20.0f, nullptr));
        sk_sp<SkImageFilter> inputs[kNumInputs];
        for (int i = 0; i < kNumInputs; ++i) {
            inputs[i] = blur;
        }
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Merge(inputs, kNumInputs));

        // Only measure the filter computations done in drawRect()
        // TODO (michaelludwig) - This benchmark, and the ones defined below, allocate their filters
        // outside of the loop. This means that repeatedly drawing with the same filter will hit
        // the global image filter cache inside the loop. Raster backend uses this cache so will see
        // artificially improved performance. Ganesh will not because it uses a cache per filter
        // call, so only within-DAG cache hits are measured (as desired). skbug:9297 wants to move
        // raster backend to the same pattern, which will make the benchmark executions fair again.
        for (int j = 0; j < loops; j++) {
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
        // makeWithFilter will only use the GPU backend if the image is already a texture
        sk_sp<SkImage> image = fImage->makeTextureImage(canvas->getGrContext());
        if (!image) {
            image = fImage;
        }

        // Set up the filters once so the allocation cost isn't included per-loop
        sk_sp<SkImageFilter> blur(SkImageFilters::Blur(20.0f, 20.0f, nullptr));
        sk_sp<SkImageFilter> inputs[kNumInputs];
        for (int i = 0; i < kNumInputs; ++i) {
            inputs[i] = blur;
        }
        sk_sp<SkImageFilter> mergeFilter = SkImageFilters::Merge(inputs, kNumInputs);

        // But measure makeWithFilter() per loop since that's the focus of this benchmark
        for (int j = 0; j < loops; j++) {
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
        // Setup filter once
        sk_sp<SkImageFilter> blur(SkImageFilters::Blur(4.0f, 4.0f, nullptr));
        SkScalar scale = 2;

        SkPaint paint;
        paint.setImageFilter(SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kR,
                                                             scale, blur, blur));

        SkRect rect = SkRect::Make(SkIRect::MakeWH(400, 400));

        // As before, measure just the filter computation time inside the loops
        for (int j = 0; j < loops; j++) {
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
        // Allocate filters once to avoid measuring instantiation time
        auto blur = SkImageFilters::Blur(20.0f, 20.0f, nullptr);
        auto offset1 = SkImageFilters::Offset(100.0f, 100.0f, blur);
        auto offset2 = SkImageFilters::Offset(-100.0f, -100.0f, blur);
        auto xfermode =
                SkImageFilters::Xfermode(SkBlendMode::kSrcIn, offset1, offset2, nullptr);

        SkPaint paint;
        paint.setImageFilter(xfermode);

        // Measure only the filter time
        for (int j = 0; j < loops; j++) {
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
