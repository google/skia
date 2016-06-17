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
#include "GrContext.h"
#include "GrDrawContext.h"
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
        SkASSERT(fOffScreen);
        fBench->preDraw(fOffScreen->getCanvas());
    }
    void onPostDraw(SkCanvas* canvas) override {
        SkASSERT(fOffScreen);
        fBench->postDraw(fOffScreen->getCanvas());
    }
    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        SkASSERT(fOffScreen);
        fBench->perCanvasPostDraw(fOffScreen->getCanvas());
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkASSERT(fOffScreen);
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
    sk_sp<SkSurface>        fOffScreen;
    SkAutoTUnref<Benchmark> fBench;
};

// Create a raster surface for off screen rendering
class CpuWrappedBenchmark : public WrappedBenchmark {
public:
    explicit CpuWrappedBenchmark(const SkSurfaceProps& surfaceProps, Benchmark* bench)
        : INHERITED(surfaceProps, bench) {}

private:
    void setupOffScreen(SkCanvas* canvas) override {
        fOffScreen = SkSurface::MakeRaster(canvas->imageInfo(), &this->surfaceProps());
    }

    void onBlitToScreen(SkCanvas* canvas, int w, int h) override {
        sk_sp<SkImage> image(fOffScreen->makeImageSnapshot());
        SkPaint blitPaint;
        blitPaint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas->drawImageRect(image.get(), SkIRect::MakeWH(w, h),
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
        fOffScreen = SkSurface::MakeRenderTarget(canvas->getGrContext(),
                                                    SkBudgeted::kNo,
                                                    canvas->imageInfo(),
                                                    fNumSamples,
                                                    &this->surfaceProps());
    }

    void onBlitToScreen(SkCanvas* canvas, int w, int h) override {
        // We call copySurface directly on the underlying GPU surfaces for a more efficient blit.
        GrDrawContext* dstDC = canvas->internal_private_accessTopLayerDrawContext();
        SkASSERT(dstDC);

        GrDrawContext* srcDC = 
                            fOffScreen->getCanvas()->internal_private_accessTopLayerDrawContext();
        SkASSERT(srcDC);
        GrRenderTarget* src = srcDC->accessRenderTarget();
        SkASSERT(src);

        SkASSERT(canvas->getGrContext() == fOffScreen->getCanvas()->getGrContext());

        dstDC->copySurface(src, SkIRect::MakeWH(w, h), SkIPoint::Make(0, 0));
    }

    int fNumSamples;
    typedef WrappedBenchmark INHERITED;
};

#endif //WrappedBenchmark_DEFINED
