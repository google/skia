/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/gpu/GrContext.h"
#include "include/utils/SkRandom.h"

#include "src/gpu/GrClip.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/SkGr.h"

// Benchmarks that exercise the bulk image and solid color quad APIs, under a variety of patterns:
//  1. Solid color
//  2. Single image
//  3. Unique image per quad
// Also includes reference benchmarks that use the conventional draw calls to quantify the benefits
// of the bulk API.

// Abstract parent benchmark that configures the rectangle geometry to draw
class BulkRectBench : public Benchmark {
public:
    static constexpr int      kRectCount   = 1000;
    static constexpr SkScalar kMinRectSize = 0.2f;
    static constexpr SkScalar kMaxRectSize = 300.f;
    static constexpr int      kWidth       = 800;
    static constexpr int      kHeight      = 600;

    SkRect fRects[kRectCount];

protected:
    virtual void doBulkDraw(SkCanvas* canvas) = 0;

    void onDelayedSetup() override {
        SkRandom rand;
        for (int i = 0; i < kRectCount; i++) {
            SkScalar w = rand.nextF() * (kMaxRectSize - kMinRectSize) + kMinRectSize;
            SkScalar h = rand.nextF() * (kMaxRectSize - kMinRectSize) + kMinRectSize;

            SkScalar x = rand.nextF() * (kWidth - w);
            SkScalar y = rand.nextF() * (kHeight - h);

            fRects[i].setXYWH(x, y, w, h);
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            this->doBulkDraw(canvas);
        }
    }

    SkIPoint onGetSize() override {
        return { kWidth, kHeight };
    }

    typedef Benchmark INHERITED;
};

// Abstract benchmark that creates the images to reference, but performs no drawing yet.
template<bool kSingleImage>
class BulkImageBench : public BulkRectBench {
public:
    // There will either be 1 image, or 1 image per rect
    static constexpr int kImageCount = kSingleImage ? 1 : kRectCount;

    sk_sp<SkImage> fImages[kImageCount];

protected:

    void onDelayedSetup() override {
        this->INHERITED::onDelayedSetup();

        for (int i = 0; i < kImageCount; ++i) {
            SkImageInfo ii = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            SkBitmap bm;
            bm.allocN32Pixels(256, 256);
            bm.eraseColor(SK_ColorRED);

            fImages[i] = SkImage::MakeFromBitmap(bm);
        }
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // Push the skimages to the GPU when using the GPU backend so that the texture creation is
        // not part of the bench measurements
        GrContext* context = canvas->getGrContext();
        if (context) {
            for (int i = 0; i < kImageCount; ++i) {
                fImages[i] = fImages[i]->makeTextureImage(context);
            }
        }
    }

    // Utility that draws the image rects one at a time, doing the right thing depending on
    // kSingleImage's value
    void doReferenceDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(kLow_SkFilterQuality);

        for (int i = 0; i < kRectCount; ++i) {
            int imageIndex = kSingleImage ? 0 : i;
            SkIRect srcRect = SkIRect::MakeWH(fImages[imageIndex]->width(),
                                              fImages[imageIndex]->height());
            canvas->drawImageRect(fImages[imageIndex].get(), srcRect, fRects[i], &paint,
                                  SkCanvas::kFast_SrcRectConstraint);
        }
    }

    // Utility that draws the image rects in a batch, doing the right thing depending on
    // kSingleImage's value
    void doBatchDraw(SkCanvas* canvas) {
        SkCanvas::ImageSetEntry batch[kRectCount];
        for (int i = 0; i < kRectCount; ++i) {
            int imageIndex = kSingleImage ? 0 : i;
            batch[i].fImage = fImages[imageIndex];
            batch[i].fSrcRect = SkRect::MakeIWH(fImages[imageIndex]->width(),
                                                fImages[imageIndex]->height());
            batch[i].fDstRect = fRects[i];
            batch[i].fAAFlags = SkCanvas::kAll_QuadAAFlags;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(kLow_SkFilterQuality);

        canvas->experimental_DrawEdgeAAImageSet(batch, kRectCount, nullptr, nullptr, &paint,
                                                SkCanvas::kFast_SrcRectConstraint);
    }

    typedef BulkRectBench INHERITED;
};

// Concrete benchmark for solid color rectangles, submitted with SkCanvas::drawRect as a reference
class BulkSolidColorReference : public BulkRectBench {
protected:
    const char* onGetName() override { return "bulkrect_solidcolor_ref"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // As a reference benchmark, this isn't actually using bulk APIs
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);
        for (int i = 0; i < kRectCount; ++i) {
            canvas->drawRect(fRects[i], paint);
        }
    }
};

// Concrete benchmark for solid color rectangles, submitted with
// SkCanvas::experimental_DrawEdgeAAQuad (to compare relative overhead of SkPaint and drawRect).
class BulkSolidColorQuad : public BulkRectBench {
protected:
    const char* onGetName() override { return "bulkrect_solidcolor_quad"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // As a reference benchmark, this isn't actually using bulk APIs
        for (int i = 0; i < kRectCount; ++i) {
            canvas->experimental_DrawEdgeAAQuad(fRects[i], nullptr, SkCanvas::kAll_QuadAAFlags,
                                                SK_ColorRED, SkBlendMode::kSrcOver);
        }
    }
};

// Concrete benchmark for solid color rectangles, submitted as a batch using GrRTC::drawQuadSet()
class BulkSolidColorBatch : public BulkRectBench {
public:
    // Currently the bulk color quad API is only available on GrRenderTargetContext
    bool isSuitableFor(Backend backend) override {
        return backend == kGPU_Backend;
    }

protected:
    const char* onGetName() override { return "bulkrect_solidcolor_batch"; }

    void doBulkDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();
        SkASSERT(context);

        GrRenderTargetContext::QuadSetEntry batch[kRectCount];
        for (int i = 0; i < kRectCount; ++i) {
            batch[i].fRect = fRects[i];
            batch[i].fColor = {1.f, 0.f, 0.f, 1.f};
            batch[i].fLocalMatrix = SkMatrix::I();
            batch[i].fAAFlags = GrQuadAAFlags::kAll;
        }

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);

        GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
        SkMatrix view = canvas->getTotalMatrix();
        GrPaint grPaint;
        SkPaintToGrPaint(context, rtc->colorInfo(), paint, view, &grPaint);
        rtc->drawQuadSet(GrNoClip(), std::move(grPaint), GrAA::kYes, view, batch, kRectCount);
    }
};

// Concrete benchmark for repeatedly drawing the same image, submitted as SkCanvas::drawImageRect
// as a reference
class BulkSingleImageReference : public BulkImageBench<true> {
protected:
    const char* onGetName() override { return "bulkrect_singleimage_ref"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // Submit one at a time
        this->doReferenceDraw(canvas);
    }
};

// Concrete benchmark for repeatedly drawing the same image, submitted as
// SkCanvas::experimental_drawEdgeAAImageSet
class BulkSingleImageBatch : public BulkImageBench<true> {
protected:
    const char* onGetName() override { return "bulkrect_singleimage_batch"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // Submit as a batch
        this->doBatchDraw(canvas);
    }
};

// Concrete benchmark for drawing unique images, submitted as SkCanvas::drawImageRect as a reference
class BulkManyImageReference : public BulkImageBench<false> {
protected:
    const char* onGetName() override { return "bulkrect_manyimage_ref"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // Submit one at a time
        this->doReferenceDraw(canvas);
    }
};

// Concrete benchmark for drawing unique images, submitted as
// SkCanvas::experimental_drawEdgeAAImageSet
class BulkManyImageBatch : public BulkImageBench<false> {
protected:
    const char* onGetName() override { return "bulkrect_manyimage_batch"; }

    void doBulkDraw(SkCanvas* canvas) override {
        // Submit as a batch
        this->doBatchDraw(canvas);
    }
};

DEF_BENCH( return new BulkSolidColorReference(); )
DEF_BENCH( return new BulkSolidColorQuad(); )
DEF_BENCH( return new BulkSolidColorBatch(); )

DEF_BENCH( return new BulkSingleImageReference(); )
DEF_BENCH( return new BulkSingleImageBatch(); )

DEF_BENCH( return new BulkManyImageReference(); )
DEF_BENCH( return new BulkManyImageBatch(); )
