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

class OverlappingTextureBackedImages : public Benchmark {
public:
    OverlappingTextureBackedImages() {
        fName.set("overlapping_texture_image_bench");
    }

    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto ii = SkImageInfo::Make(kImageSize, kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
        auto surf = canvas->makeSurface(ii);
        SkRandom random;
        for (int i = 0; i < kNumImages; ++i) {
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
        for (int i = 0; i < loops; i++) {
            canvas->drawImage(fImages[i % kNumImages].get(), x, y);
            x += kImageSize / 2;
            y += kImageSize / 3.5;
            if (x > 500.f) {
                x = 0.f;
            }
            if (y > 500.f) {
                y = 0.f;
            }
        }
    }

private:
    static const int kNumImages = 5;
    static const int kImageSize = 30;
    sk_sp<SkImage> fImages[kNumImages];
    SkString fName;

    typedef Benchmark INHERITED;
};


DEF_BENCH( return new Image2RasterBench; )
DEF_BENCH( return new OverlappingTextureBackedImages);
