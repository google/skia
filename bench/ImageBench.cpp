/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSurface.h"

class Image2RasterBench : public Benchmark {
public:
    Image2RasterBench() {
        fName.set("native_image_to_raster_surface");
    }

    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend || kRaster_Backend == backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    // We explicitly want to bench drawing a Image [cpu or gpu backed] into a raster target,
    // to ensure that we can cache the read-back in the case of gpu -> raster
    //
    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // create an Image reflecting the canvas (gpu or cpu)
        SkImageInfo info = canvas->imageInfo().makeWH(100, 100);
        auto surface(canvas->makeSurface(info));
        canvas->drawColor(SK_ColorRED);
        fImage = surface->makeImageSnapshot();

        // create a cpu-backed Surface
        SkImageInfo n32Info = SkImageInfo::MakeN32Premul(100, 100);
        fRasterSurface = SkSurface::MakeRaster(n32Info);
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        // Release the image and raster surface here to prevent out of order destruction
        // between these and the gpu interface.
        fRasterSurface.reset(nullptr);
        fImage.reset(nullptr);
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            for (int inner = 0; inner < 10; ++inner) {
                fRasterSurface->getCanvas()->drawImage(fImage.get(), 0, 0);
            }
        }
    }

private:
    SkString                fName;
    sk_sp<SkImage>   fImage;
    sk_sp<SkSurface> fRasterSurface;

    typedef Benchmark INHERITED;
};

class TextureImages : public Benchmark {
public:
    enum class ImageSize {
        kSmall,
        kMedium,
        kLarge,
        kExtraLarge,
    };
    enum class DstRect {
        kSmallSparse,
        kSmallDense,
        kMedium,
    };
    enum class Overlap {
        kNo,
        kYes
    };

    TextureImages(ImageSize imageSize, DstRect dstRect, Overlap overlap)
            : fImageSize(imageSize)
            , fDstRect(dstRect)
            , fOverlap(overlap) {
        fName.set("texture_images");
        switch (fImageSize) {
            case ImageSize::kExtraLarge:
                fName.append("_extra_large_image");
                break;
            case ImageSize::kLarge:
                fName.append("_large_image");
                break;
            case ImageSize::kMedium:
                fName.append("_medium_image");
                break;
            case ImageSize::kSmall:
                fName.append("_small_image");
                break;
        }
        switch (fDstRect) {
            case DstRect::kMedium:
                fName.append("_medium_rects");
                break;
            case DstRect::kSmallSparse:
                fName.append("_small_sparse_rects");
                break;
            case DstRect::kSmallDense:
                fName.append("_small_dense_rects");
                break;
        }
        if (Overlap::kYes == overlap) {
            fName.append("_overlap");
        }
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        SkISize size = this->imageSize();
        auto ii = SkImageInfo::Make(size.fWidth, size.fHeight, kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType, nullptr);
        SkRandom random;
        for (int i = 0; i < kNumImages; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            surf->getCanvas()->drawRect(SkRect::MakeLTRB(3, 3, size.fWidth - 3, size.fHeight - 3), paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        for (int i = 0; i < kNumImages; ++i) {
            fImages[i].reset();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect rect;
        switch (fDstRect) {
            case DstRect::kMedium:
                rect = SkRect::MakeWH(kMediumImageSize, kMediumImageSize);
                break;
            case DstRect::kSmallSparse:
            case DstRect::kSmallDense:
                rect = SkRect::MakeWH(kSmallImageSize, kSmallImageSize);
                break;
        }
        SkPaint paint;
        paint.setAlpha(0x40);
        paint.setFilterQuality(kLow_SkFilterQuality);
        for (int i = 0; i < loops * kNumImages; i++) {
            auto offset = this->updateTranslate(i);
            // Prevent any batching except without multitexturing since we're trying to measure
            // drawing distinct images and just repeating images here to increase the workload for
            // timing reasons.
            if (0 == i % kNumImages) {
                canvas->flush();
            }
            canvas->drawImageRect(fImages[i % kNumImages].get(),
                                  rect.makeOffset(offset.fX, offset.fY), &paint);
        }
    }

private:
    SkISize imageSize() const {
        switch (fImageSize) {
            case ImageSize::kSmall:
                return SkISize{kSmallImageSize, kSmallImageSize};
            case ImageSize::kMedium:
                return SkISize{kMediumImageSize, kMediumImageSize};
            case ImageSize::kLarge:
                return SkISize{kLargeImageSize, kLargeImageSize};
            case ImageSize::kExtraLarge:
                return SkISize{kExtraLargeImageSize, kExtraLargeImageSize};
        }
        return SkISize{0, 0};
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(kMediumImageSize * kOutputRectMulX, kMediumImageSize * kOutputRectMulY);
    }

    SkVector updateTranslate(int i) const {
        if (Overlap::kYes == fOverlap) {
            i = i / kNumImages;
        }
        SkVector offset;
        SkScalar t = this->translateAmount();
        offset.fX = i % kOutputRectMulX * t;
        offset.fY = (i / kOutputRectMulX) % kOutputRectMulY * t;
        return offset;
    }

    SkScalar translateAmount() const {
        return SkIntToScalar((DstRect::kSmallDense == fDstRect) ? kSmallImageSize
                                                                : kMediumImageSize);
    }

    static const int kSmallImageSize = 10;
    static const int kMediumImageSize = 250;
    static const int kLargeImageSize = 1024;
    static const int kExtraLargeImageSize = 2048;
    static const int kOutputRectMulX = 8;
    static const int kOutputRectMulY = 5;
    static const int kNumImages = 8;

    sk_sp<SkImage> fImages[kNumImages];
    SkString fName;
    ImageSize fImageSize;
    DstRect fDstRect;
    Overlap fOverlap;

    typedef Benchmark INHERITED;
};

using ImageSize = TextureImages::ImageSize;
using DstRect = TextureImages::DstRect;
using Overlap = TextureImages::Overlap;

DEF_BENCH( return new Image2RasterBench; )
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kMedium, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kMedium, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kMedium, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kMedium, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kMedium, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kMedium, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kMedium, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kMedium, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kSmallSparse, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kSmallSparse, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kSmallSparse, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kSmallSparse, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kSmallSparse, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kSmallSparse, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kSmallSparse, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kSmallSparse, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kSmallDense, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kExtraLarge, DstRect::kSmallDense, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kSmallDense, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kLarge, DstRect::kSmallDense, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kSmallDense, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kMedium, DstRect::kSmallDense, Overlap::kYes));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kSmallDense, Overlap::kNo));
DEF_BENCH( return new TextureImages(ImageSize::kSmall, DstRect::kSmallDense, Overlap::kYes));
