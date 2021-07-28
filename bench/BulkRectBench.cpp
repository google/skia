/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/gpu/GrDirectContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrOpsTypes.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

// Benchmarks that exercise the bulk image and solid color quad APIs, under a variety of patterns:
enum class ImageMode {
    kShared, // 1. One shared image referenced by every rectangle
    kUnique, // 2. Unique image for every rectangle
    kNone    // 3. No image, solid color shading per rectangle
};
//   X
enum class DrawMode {
    kBatch,  // Bulk API submission, one call to draw every rectangle
    kRef,    // One standard SkCanvas draw call per rectangle
    kQuad    // One experimental draw call per rectangle, only for solid color draws
};
//   X
enum class RectangleLayout {
    kRandom,  // Random overlapping rectangles
    kGrid     // Small, non-overlapping rectangles in a grid covering the output surface
};

// Benchmark runner that can be configured by template arguments.
template<int kRectCount, RectangleLayout kLayout, ImageMode kImageMode, DrawMode kDrawMode>
class BulkRectBench : public Benchmark {
public:
    static_assert(kImageMode == ImageMode::kNone || kDrawMode != DrawMode::kQuad,
                  "kQuad only supported for solid color draws");

    static constexpr int kWidth      = 1024;
    static constexpr int kHeight     = 1024;

    // There will either be 0 images, 1 image, or 1 image per rect
    static constexpr int kImageCount = kImageMode == ImageMode::kShared ?
            1 : (kImageMode == ImageMode::kNone ? 0 : kRectCount);

    bool isSuitableFor(Backend backend) override {
        if (kDrawMode == DrawMode::kBatch && kImageMode == ImageMode::kNone) {
            // Currently the bulk color quad API is only available on skgpu::v1::SurfaceDrawContext
            return backend == kGPU_Backend;
        } else {
            return this->INHERITED::isSuitableFor(backend);
        }
    }

protected:
    SkRect         fRects[kRectCount];
    sk_sp<SkImage> fImages[kImageCount];
    SkColor4f      fColors[kRectCount];
    SkString       fName;

    void computeName()  {
        fName = "bulkrect";
        fName.appendf("_%d", kRectCount);
        if (kLayout == RectangleLayout::kRandom) {
            fName.append("_random");
        } else {
            fName.append("_grid");
        }
        if (kImageMode == ImageMode::kShared) {
            fName.append("_sharedimage");
        } else if (kImageMode == ImageMode::kUnique) {
            fName.append("_uniqueimages");
        } else {
            fName.append("_solidcolor");
        }
        if (kDrawMode == DrawMode::kBatch) {
            fName.append("_batch");
        } else if (kDrawMode == DrawMode::kRef) {
            fName.append("_ref");
        } else {
            fName.append("_quad");
        }
    }

    void drawImagesBatch(SkCanvas* canvas) const {
        SkASSERT(kImageMode != ImageMode::kNone);
        SkASSERT(kDrawMode == DrawMode::kBatch);

        SkCanvas::ImageSetEntry batch[kRectCount];
        for (int i = 0; i < kRectCount; ++i) {
            int imageIndex = kImageMode == ImageMode::kShared ? 0 : i;
            batch[i].fImage = fImages[imageIndex];
            batch[i].fSrcRect = SkRect::MakeIWH(fImages[imageIndex]->width(),
                                                fImages[imageIndex]->height());
            batch[i].fDstRect = fRects[i];
            batch[i].fAAFlags = SkCanvas::kAll_QuadAAFlags;
        }

        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->experimental_DrawEdgeAAImageSet(batch, kRectCount, nullptr, nullptr,
                                                SkSamplingOptions(SkFilterMode::kLinear), &paint,
                                                SkCanvas::kFast_SrcRectConstraint);
    }

    void drawImagesRef(SkCanvas* canvas) const {
        SkASSERT(kImageMode != ImageMode::kNone);
        SkASSERT(kDrawMode == DrawMode::kRef);

        SkPaint paint;
        paint.setAntiAlias(true);

        for (int i = 0; i < kRectCount; ++i) {
            int imageIndex = kImageMode == ImageMode::kShared ? 0 : i;
            SkRect srcRect = SkRect::MakeIWH(fImages[imageIndex]->width(),
                                             fImages[imageIndex]->height());
            canvas->drawImageRect(fImages[imageIndex].get(), srcRect, fRects[i],
                                  SkSamplingOptions(SkFilterMode::kLinear), &paint,
                                  SkCanvas::kFast_SrcRectConstraint);
        }
    }

    void drawSolidColorsBatch(SkCanvas* canvas) const {
        SkASSERT(kImageMode == ImageMode::kNone);
        SkASSERT(kDrawMode == DrawMode::kBatch);

        auto context = canvas->recordingContext();
        SkASSERT(context);

        GrQuadSetEntry batch[kRectCount];
        for (int i = 0; i < kRectCount; ++i) {
            batch[i].fRect = fRects[i];
            batch[i].fColor = fColors[i].premul();
            batch[i].fLocalMatrix = SkMatrix::I();
            batch[i].fAAFlags = GrQuadAAFlags::kAll;
        }

        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAntiAlias(true);

        auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
        SkMatrix view = canvas->getLocalToDeviceAs3x3();
        SkSimpleMatrixProvider matrixProvider(view);
        GrPaint grPaint;
        SkPaintToGrPaint(context, sdc->colorInfo(), paint, matrixProvider, &grPaint);
        sdc->drawQuadSet(nullptr, std::move(grPaint), GrAA::kYes, view, batch, kRectCount);
    }

    void drawSolidColorsRef(SkCanvas* canvas) const {
        SkASSERT(kImageMode == ImageMode::kNone);
        SkASSERT(kDrawMode == DrawMode::kRef || kDrawMode == DrawMode::kQuad);

        SkPaint paint;
        paint.setAntiAlias(true);
        for (int i = 0; i < kRectCount; ++i) {
            if (kDrawMode == DrawMode::kRef) {
                paint.setColor4f(fColors[i]);
                canvas->drawRect(fRects[i], paint);
            } else {
                canvas->experimental_DrawEdgeAAQuad(fRects[i], nullptr, SkCanvas::kAll_QuadAAFlags,
                                                    fColors[i], SkBlendMode::kSrcOver);
            }
        }
    }

    const char* onGetName() override {
        if (fName.isEmpty()) {
            this->computeName();
        }
        return fName.c_str();
    }

    void onDelayedSetup() override {
        static constexpr SkScalar kMinRectSize = 0.2f;
        static constexpr SkScalar kMaxRectSize = 300.f;

        SkRandom rand;
        for (int i = 0; i < kRectCount; i++) {
            if (kLayout == RectangleLayout::kRandom) {
                SkScalar w = rand.nextF() * (kMaxRectSize - kMinRectSize) + kMinRectSize;
                SkScalar h = rand.nextF() * (kMaxRectSize - kMinRectSize) + kMinRectSize;

                SkScalar x = rand.nextF() * (kWidth - w);
                SkScalar y = rand.nextF() * (kHeight - h);

                fRects[i].setXYWH(x, y, w, h);
            } else {
                int gridSize = SkScalarCeilToInt(SkScalarSqrt(kRectCount));
                SkASSERT(gridSize * gridSize >= kRectCount);

                SkScalar w = (kWidth - 1.f) / gridSize;
                SkScalar h = (kHeight - 1.f) / gridSize;

                SkScalar x = (i % gridSize) * w + 0.5f; // Offset to ensure AA doesn't get disabled
                SkScalar y = (i / gridSize) * h + 0.5f;

                fRects[i].setXYWH(x, y, w, h);
            }

            // Make sure we don't extend outside the render target, don't want to include clipping
            // in the benchmark.
            SkASSERT(SkRect::MakeWH(kWidth, kHeight).contains(fRects[i]));

            fColors[i] = {rand.nextF(), rand.nextF(), rand.nextF(), 1.f};
        }
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // Push the skimages to the GPU when using the GPU backend so that the texture creation is
        // not part of the bench measurements. Always remake the images since they are so simple,
        // and since they are context-specific, this works when the bench runs multiple GPU backends
        auto direct = GrAsDirectContext(canvas->recordingContext());
        for (int i = 0; i < kImageCount; ++i) {
            SkBitmap bm;
            bm.allocN32Pixels(256, 256);
            bm.eraseColor(fColors[i].toSkColor());
            auto image = bm.asImage();

            fImages[i] = direct ? image->makeTextureImage(direct) : std::move(image);
        }
    }

    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        for (int i = 0; i < kImageCount; ++i) {
            // For Vulkan we need to make sure the bench isn't holding onto any refs to the
            // GrContext when we go to delete the vulkan context (which happens before the bench is
            // deleted). So reset all the images here so they aren't holding GrContext refs.
            fImages[i].reset();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            if (kImageMode == ImageMode::kNone) {
                if (kDrawMode == DrawMode::kBatch) {
                    this->drawSolidColorsBatch(canvas);
                } else {
                    this->drawSolidColorsRef(canvas);
                }
            } else {
                if (kDrawMode == DrawMode::kBatch) {
                    this->drawImagesBatch(canvas);
                } else {
                    this->drawImagesRef(canvas);
                }
            }
        }
    }

    SkIPoint onGetSize() override {
        return { kWidth, kHeight };
    }

    using INHERITED = Benchmark;
};

// constructor call is wrapped in () so the macro doesn't break parsing the commas in the template
#define ADD_BENCH(n, layout, imageMode, drawMode)                              \
    DEF_BENCH( return (new BulkRectBench<n, layout, imageMode, drawMode>()); )

#define ADD_BENCH_FAMILY(n, layout)                                            \
    ADD_BENCH(n, layout, ImageMode::kShared, DrawMode::kBatch)                 \
    ADD_BENCH(n, layout, ImageMode::kShared, DrawMode::kRef)                   \
    ADD_BENCH(n, layout, ImageMode::kUnique, DrawMode::kBatch)                 \
    ADD_BENCH(n, layout, ImageMode::kUnique, DrawMode::kRef)                   \
    ADD_BENCH(n, layout, ImageMode::kNone,   DrawMode::kBatch)                 \
    ADD_BENCH(n, layout, ImageMode::kNone,   DrawMode::kRef)                   \
    ADD_BENCH(n, layout, ImageMode::kNone,   DrawMode::kQuad)

ADD_BENCH_FAMILY(1000,  RectangleLayout::kRandom)
ADD_BENCH_FAMILY(1000,  RectangleLayout::kGrid)

#undef ADD_BENCH_FAMILY
#undef ADD_BENCH
