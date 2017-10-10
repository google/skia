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
    OverlappingImages(bool overlap, bool large) : fAllRects(ontop), fLarge(large) {
        fName.set("texture_images");
        if (overlap) {
            fName.append("_overlap");
        } else {
            fName.append("_no_overlap");
        }
        if (large) {
            fName.append("_large");
        } else {
            fName.append("_small");
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
            surf->getCanvas()->clear(random.nextU());
            SkPaint paint;
            paint.setColor(random.nextU());
            surf->getCanvas()->drawRect(SkRect::MakeLTRB(3, 3, kImageSize - 3, kImageSize - 3), paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        for (int i = 0; i < kNumImages; ++i) {
            fImages[i].reset();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkScalar x = 0, y = 0;
        SkRect rect = SkRect::MakeWH(kRectSize, kRectSize).makeOutset(0.5f, 0.5f);
        SkVector offset{0, 0};
        for (int i = 0; i < loops * kNumImages; i++) {
            if (0 == i % kNumImages) {
                canvas->flush();
            }
            canvas->drawImageRect(fImages[i % kNumImages].get(),
                                  rect.makeOffset(offset.fX, offset.fY), nullptr);
            this->updateTranslate(&offset, i);
        }
    }

private:
    SkISize imageSize() const { return fLarge ? SkISize{kRectSize, kRectSize} : SkISize{10, 10}; }

    SkIPoint onGetSize() const {
        return SkIPoint::Make(kRectSize * kOutputRectMulX, kRectSize * kOutputRectMulY);
    }

    void updateTranslate(SkVector* curr, int i) {
        if (fAllRects) {
            i = i / kNumImages;
        }
        curr->fX = i % kOutputRectMulX * kRectSize;
        curr->fY = (i / kOutputRectMulX) % kOutputRectMulY * kRectSize;
    }

    static const int kRectSize = 300;
    static const int kOutputRectMulX = 5;
    static const int kOutputRectMulY = 4;
    static const int kNumImages = 8;

    static const int kNumRectsInOutput = kOutputRectMulX * kOutputRectMulY;
    static_assert(kNumRectsInOutput / kNumImages * kNumImages == kNumRectsInOutput, "test assumes this");

    sk_sp<SkImage> fImages[kNumImages];
    SkString fName;
    bool fAllRects;
    bool fLarge;

    typedef Benchmark INHERITED;
};


DEF_BENCH( return new Image2RasterBench; )
DEF_BENCH( return new TextureImages(false, false));
DEF_BENCH( return new TextureImages(false, true));
DEF_BENCH( return new TextureImages(true, false));
DEF_BENCH( return new TextureImages(true, true));
