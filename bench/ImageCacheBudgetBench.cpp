/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "tools/ToolUtils.h"


#include <utility>

/** These benchmarks were designed to measure changes to GrResourceCache's replacement policy */

//////////////////////////////////////////////////////////////////////////////

// The width/height of the images to draw. The small size underestimates the value of a good
// replacement strategy since the texture uploads are quite small. However, the effects are still
// significant and this lets the benchmarks complete a lot faster, especially on mobile.
static constexpr int kS = 25;

static void make_images(sk_sp<SkImage> imgs[], int cnt) {
    for (int i = 0; i < cnt; ++i) {
        imgs[i] = ToolUtils::create_checkerboard_image(kS, kS, SK_ColorBLACK, SK_ColorCYAN, 10);
    }
}

static void draw_image(SkCanvas* canvas, SkImage* img) {
    // Make the paint transparent to avoid any issues of deferred tiler blending
    // optmizations
    SkPaint paint;
    paint.setAlpha(0x10);
    canvas->drawImage(img, 0, 0, SkSamplingOptions(), &paint);
}

void set_cache_budget(SkCanvas* canvas, int approxImagesInBudget) {
    // This is inexact but we attempt to figure out a baseline number of resources GrContext needs
    // to render an SkImage and add one additional resource for each image we'd like to fit.
    auto context =  canvas->recordingContext()->asDirectContext();
    SkASSERT(context);
    context->flushAndSubmit();
    context->priv().getResourceCache()->purgeUnlockedResources(
            GrPurgeResourceOptions::kAllResources);
    sk_sp<SkImage> image;
    make_images(&image, 1);
    draw_image(canvas, image.get());
    context->flushAndSubmit();
    int baselineCount;
    context->getResourceCacheUsage(&baselineCount, nullptr);
    baselineCount -= 1; // for the image's textures.
    context->setResourceCacheLimits(baselineCount + approxImagesInBudget, 1 << 30);
    context->priv().getResourceCache()->purgeUnlockedResources(
            GrPurgeResourceOptions::kAllResources);
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Tests repeatedly drawing the same set of images in each frame. Different instances of the bench
 * run with different cache sizes and either repeat the image order each frame or use a random
 * order. Every variation of this bench draws the same image set, only the budget and order of
 * images differs. Since the total fill is the same they can be cross-compared.
 */
class ImageCacheBudgetBench : public Benchmark {
public:
    /** budgetSize is the number of images that can fit in the cache. 100 images will be drawn. */
    ImageCacheBudgetBench(int budgetSize, bool shuffle)
            : fBudgetSize(budgetSize)
            , fShuffle(shuffle)
            , fIndices(nullptr) {
        float imagesOverBudget = float(kImagesToDraw) / budgetSize;
        // Make the benchmark name contain the percentage of the budget that is used in each
        // simulated frame.
        fName.printf("image_cache_budget_%.0f%s", imagesOverBudget * 100,
                     (shuffle ? "_shuffle" : ""));
    }

    bool isSuitableFor(Backend backend) override { return Backend::kGanesh == backend; }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();
        SkASSERT(context);
        fOldBytes = context->getResourceCacheLimit();
        set_cache_budget(canvas, fBudgetSize);
        make_images(fImages, kImagesToDraw);
        if (fShuffle) {
            SkRandom random;
            fIndices.reset(new int[kSimulatedFrames * kImagesToDraw]);
            for (int frame = 0; frame < kSimulatedFrames; ++frame) {
                int* base = fIndices.get() + frame * kImagesToDraw;
                for (int i = 0; i < kImagesToDraw; ++i) {
                    base[i] = i;
                }
                for (int i = 0; i < kImagesToDraw - 1; ++i) {
                    int other = random.nextULessThan(kImagesToDraw - i) + i;
                    using std::swap;
                    swap(base[i], base[other]);
                }
            }
        }
    }

    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        auto context =  canvas->recordingContext()->asDirectContext();
        SkASSERT(context);
        context->setResourceCacheLimit(fOldBytes);
        for (int i = 0; i < kImagesToDraw; ++i) {
            fImages[i].reset();
        }
        fIndices.reset(nullptr);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());

        for (int i = 0; i < loops; ++i) {
            for (int frame = 0; frame < kSimulatedFrames; ++frame) {
                for (int j = 0; j < kImagesToDraw; ++j) {
                    int idx;
                    if (fShuffle) {
                        idx = fIndices[frame * kImagesToDraw + j];
                    } else {
                        idx = j;
                    }
                    draw_image(canvas, fImages[idx].get());
                }
                // Simulate a frame boundary by flushing. This should notify GrResourceCache.
                if (dContext) {
                    dContext->flush();
                }
           }
        }
    }

private:
    inline static constexpr int kImagesToDraw = 100;
    inline static constexpr int kSimulatedFrames = 5;

    int                         fBudgetSize;
    bool                        fShuffle;
    SkString                    fName;
    sk_sp<SkImage>              fImages[kImagesToDraw];
    std::unique_ptr<int[]>      fIndices;
    size_t                      fOldBytes;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new ImageCacheBudgetBench(105, false); )

DEF_BENCH( return new ImageCacheBudgetBench(90, false); )

DEF_BENCH( return new ImageCacheBudgetBench(80, false); )

DEF_BENCH( return new ImageCacheBudgetBench(50, false); )

DEF_BENCH( return new ImageCacheBudgetBench(105, true); )

DEF_BENCH( return new ImageCacheBudgetBench(90, true); )

DEF_BENCH( return new ImageCacheBudgetBench(80, true); )

DEF_BENCH( return new ImageCacheBudgetBench(50, true); )

//////////////////////////////////////////////////////////////////////////////

/**
 * Similar to above but changes between being over and under budget by varying the number of images
 * rendered. This is not directly comparable to the non-dynamic benchmarks.
 */
class ImageCacheBudgetDynamicBench : public Benchmark {
public:
    enum class Mode {
        // Increase from min to max images drawn gradually over simulated frames and then back.
        kPingPong,
        // Alternate between under and over budget every other simulated frame.
        kFlipFlop
    };

    ImageCacheBudgetDynamicBench(Mode mode) : fMode(mode) {}

    bool isSuitableFor(Backend backend) override { return Backend::kGanesh == backend; }

protected:
    const char* onGetName() override {
        switch (fMode) {
            case Mode::kPingPong:
                return "image_cache_budget_dynamic_ping_pong";
            case Mode::kFlipFlop:
                return "image_cache_budget_dynamic_flip_flop";
        }
        return "";
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();
        SkASSERT(context);
        context->getResourceCacheLimits(&fOldCount, &fOldBytes);
        make_images(fImages, kMaxImagesToDraw);
        set_cache_budget(canvas, kImagesInBudget);
    }

    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();
        SkASSERT(context);
        context->setResourceCacheLimits(fOldCount, fOldBytes);
        for (int i = 0; i < kMaxImagesToDraw; ++i) {
            fImages[i].reset();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());

        int delta = 0;
        switch (fMode) {
            case Mode::kPingPong:
                delta = 1;
                break;
            case Mode::kFlipFlop:
                delta = kMaxImagesToDraw - kMinImagesToDraw;
                break;
        }
        for (int i = 0; i < loops; ++i) {
            int imgsToDraw = kMinImagesToDraw;
            for (int frame = 0; frame < kSimulatedFrames; ++frame) {
                for (int j = 0; j < imgsToDraw; ++j) {
                    draw_image(canvas, fImages[j].get());
                }
                imgsToDraw += delta;
                if (imgsToDraw > kMaxImagesToDraw || imgsToDraw < kMinImagesToDraw) {
                    delta = -delta;
                    imgsToDraw += 2 * delta;
                }
                // Simulate a frame boundary by flushing. This should notify GrResourceCache.
                if (dContext) {
                    dContext->flush();
                }
            }
        }
    }

private:
    inline static constexpr int kImagesInBudget  = 25;
    inline static constexpr int kMinImagesToDraw = 15;
    inline static constexpr int kMaxImagesToDraw = 35;
    inline static constexpr int kSimulatedFrames = 80;

    Mode                        fMode;
    sk_sp<SkImage>              fImages[kMaxImagesToDraw];
    size_t                      fOldBytes;
    int                         fOldCount;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new ImageCacheBudgetDynamicBench(ImageCacheBudgetDynamicBench::Mode::kPingPong); )
DEF_BENCH( return new ImageCacheBudgetDynamicBench(ImageCacheBudgetDynamicBench::Mode::kFlipFlop); )
