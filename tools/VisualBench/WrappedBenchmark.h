/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WrappedBenchmark_DEFINED
#define WrappedBenchmark_DEFINED

#include "Benchmark.h"
#include "SkDevice.h"
#include "SkSurface.h"
#include "GrContext.h"
#include "GrRenderTarget.h"

// Wrap some other benchmark to allow specialization to either
// cpu or gpu backends. The derived class will override 'setupOffScreen'
// to create an offscreen surface in which the actual rendering will occur.
class WrappedBenchmark : public Benchmark {
public:
    // Takes ownership of caller's ref on `bench`.
    explicit WrappedBenchmark(const SkSurfaceProps& surfaceProps, Benchmark* bench)
        : fSurfaceProps(surfaceProps)
        , fBench(bench) {}

    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }

    const char* onGetName()       override { return fBench->getName(); }
    const char* onGetUniqueName() override { return fBench->getUniqueName(); }

    void onDelayedSetup() override { fBench->delayedSetup(); }
    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        this->setupOffScreen(canvas);
        fOffScreen->getCanvas()->clear(SK_ColorWHITE);
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
        this->blitToScreen(canvas);
    }

    virtual SkIPoint onGetSize() override { return fBench->getSize(); }

protected:
    virtual void setupOffScreen(SkCanvas*)=0;

    void blitToScreen(SkCanvas* canvas) {
        int w = SkTMin(fBench->getSize().fX, fOffScreen->width());
        int h = SkTMin(fBench->getSize().fY, fOffScreen->width());
        this->onBlitToScreen(canvas, w, h);
    }

    virtual void onBlitToScreen(SkCanvas* canvas, int w, int h) = 0;

    SkSurfaceProps          fSurfaceProps;
    SkAutoTUnref<SkSurface> fOffScreen;
    SkAutoTUnref<Benchmark> fBench;
};

// Create a raster surface for off screen rendering
class CpuWrappedBenchmark : public WrappedBenchmark {
public:
    explicit CpuWrappedBenchmark(const SkSurfaceProps& surfaceProps, Benchmark* bench)
        : INHERITED(surfaceProps, bench) {}

private:
    void setupOffScreen(SkCanvas* canvas) override {
        fOffScreen.reset(SkSurface::NewRaster(canvas->imageInfo(), &this->surfaceProps()));
    }

    void onBlitToScreen(SkCanvas* canvas, int w, int h) override {
        SkAutoTUnref<SkImage> image(fOffScreen->newImageSnapshot());
        SkPaint blitPaint;
        blitPaint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas->drawImageRect(image, SkIRect::MakeWH(w, h),
                              SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h)), &blitPaint);
    }

    typedef WrappedBenchmark INHERITED;
};

// Create an MSAA & NVPR-enabled GPU backend
class GpuWrappedBenchmark : public WrappedBenchmark {
public:
    explicit GpuWrappedBenchmark(const SkSurfaceProps& surfaceProps, Benchmark* bench,
                                 int numSamples)
        : INHERITED(surfaceProps, bench)
        , fNumSamples(numSamples) {}

private:
    void setupOffScreen(SkCanvas* canvas) override {
        fOffScreen.reset(SkSurface::NewRenderTarget(canvas->getGrContext(),
                                                    SkSurface::kNo_Budgeted,
                                                    canvas->imageInfo(),
                                                    fNumSamples,
                                                    &this->surfaceProps()));
    }

    void onBlitToScreen(SkCanvas* canvas, int w, int h) override {
        // We call copySurface directly on the underlying GPU surfaces for a more efficient blit.
        GrRenderTarget* dst, *src;

        SkCanvas::LayerIter canvasIter(canvas, false);
        SkAssertResult((dst = canvasIter.device()->accessRenderTarget()));

        SkCanvas::LayerIter offscreenIter(fOffScreen->getCanvas(), false);
        SkAssertResult((src = offscreenIter.device()->accessRenderTarget()));

        SkASSERT(dst->getContext() == src->getContext());

        dst->getContext()->copySurface(dst, src, SkIRect::MakeWH(w, h), SkIPoint::Make(0, 0));

#ifdef SK_DEBUG
        // This method should not be called while layers are saved.
        canvasIter.next();
        SkASSERT(canvasIter.done());

        offscreenIter.next();
        SkASSERT(offscreenIter.done());
#endif
    }

    int fNumSamples;
    typedef WrappedBenchmark INHERITED;
};

#endif //WrappedBenchmark_DEFINED
