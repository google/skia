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

class GPU2RasterImageBench : public Benchmark {
public:
    GPU2RasterImageBench() {
        fName.set("gpu2raster_image");
    }

    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // create a gpu-backed Image
        SkImageInfo info = SkImageInfo::MakeN32Premul(500, 500);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));
        canvas->drawColor(SK_ColorRED);
        fGpuImage.reset(surface->newImageSnapshot());

        // create a cpu-backed Surface
        fRasterSurface.reset(SkSurface::NewRaster(info));
    }

    void onDraw(const int loops, SkCanvas*) override {
        // Time drawing a gpu-image into a raster-surface
        for (int i = 0; i < loops; i++) {
            for (int inner = 0; inner < 10; ++inner) {
                fRasterSurface->getCanvas()->drawImage(fGpuImage, 0, 0);
            }
        }
    }

private:
    SkString                fName;
    SkAutoTUnref<SkImage>   fGpuImage;
    SkAutoTUnref<SkSurface> fRasterSurface;

    typedef Benchmark INHERITED;
};


DEF_BENCH( return new GPU2RasterImageBench; )
