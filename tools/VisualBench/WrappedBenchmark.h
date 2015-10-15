/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WrappedBenchmark_DEFINED
#define WrappedBenchmark_DEFINED

#include "Benchmark.h"
#include "SkSurface.h"

// Wrap some other benchmark to allow specialization to either
// cpu or gpu backends. The derived class will override 'setupOffScreen'
// to create an offscreen surface in which the actual rendering will occur.
class WrappedBenchmark : public Benchmark {
public:
    // Takes ownership of caller's ref on `bench`.
    explicit WrappedBenchmark(Benchmark* bench) : fBench(bench) {}

    const char* onGetName()       override { return fBench->getName(); }
    const char* onGetUniqueName() override { return fBench->getUniqueName(); }

    void onDelayedSetup() override { fBench->delayedSetup(); }
    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        fOffScreen.reset(this->setupOffScreen(canvas));
        fBench->perCanvasPreDraw(fOffScreen->getCanvas());
    }
    void onPreDraw(SkCanvas* canvas) override {
        SkASSERT(fOffScreen.get());
        fBench->preDraw(fOffScreen->getCanvas());
    }
    void onPostDraw(SkCanvas* canvas) override {
        SkASSERT(fOffScreen.get());
        fBench->postDraw(fOffScreen->getCanvas());
    }
    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        SkASSERT(fOffScreen.get());
        fBench->perCanvasPostDraw(fOffScreen->getCanvas());
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkASSERT(fOffScreen.get());
        fBench->draw(loops, fOffScreen->getCanvas());
        SkAutoTUnref<SkImage> image(fOffScreen->newImageSnapshot());
        canvas->drawImage(image, 0,0);
    }

    virtual SkIPoint onGetSize() override { return fBench->getSize(); }

private:
    virtual SkSurface* setupOffScreen(SkCanvas*)=0;

    SkAutoTUnref<SkSurface> fOffScreen;
    SkAutoTUnref<Benchmark> fBench;
};

// Create a raster surface for off screen rendering
class CpuWrappedBenchmark : public WrappedBenchmark {
public:
    explicit CpuWrappedBenchmark(Benchmark* bench) : INHERITED(bench) {}

private:
    SkSurface* setupOffScreen(SkCanvas* canvas) override {
        return SkSurface::NewRaster(canvas->imageInfo());
    }

    typedef WrappedBenchmark INHERITED;
};

// Create an MSAA & NVPR-enabled GPU backend
class NvprWrappedBenchmark : public WrappedBenchmark {
public:
    explicit NvprWrappedBenchmark(Benchmark* bench, int numSamples)
        : INHERITED(bench)
        , fNumSamples(numSamples) {}

private:
    SkSurface* setupOffScreen(SkCanvas* canvas) override {
        return SkSurface::NewRenderTarget(canvas->getGrContext(),
                                          SkSurface::kNo_Budgeted,
                                          canvas->imageInfo(),
                                          fNumSamples);
    }

    int fNumSamples;
    typedef WrappedBenchmark INHERITED;
};

#endif //WrappedBenchmark_DEFINED
