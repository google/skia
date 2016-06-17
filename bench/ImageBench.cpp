/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImage.h"
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
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        auto surface(canvas->makeSurface(info));
        canvas->drawColor(SK_ColorRED);
        fImage = surface->makeImageSnapshot();

        // create a cpu-backed Surface
        fRasterSurface = SkSurface::MakeRaster(info);
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
DEF_BENCH( return new Image2RasterBench; )
