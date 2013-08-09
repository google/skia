
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU
#include "GrRenderTarget.h"
#endif
#include "SkBenchmark.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkImage.h"
#include "SkSurface.h"

class DeferredSurfaceCopyBench : public SkBenchmark {
    enum {
        N = SkBENCHLOOP(5),
        kSurfaceWidth = 1000,
        kSurfaceHeight = 1000,
    };
public:
    DeferredSurfaceCopyBench(void* param, bool discardableContents) : SkBenchmark(param) {
        fDiscardableContents = discardableContents;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fDiscardableContents ? "DeferredSurfaceCopy_discardable" :
            "DeferredSurfaceCopy_nonDiscardable";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        // The canvas is not actually used for this test except to provide
        // configuration information: gpu, multisampling, size, etc?
        SkImage::Info info;
        info.fWidth = kSurfaceWidth;
        info.fHeight = kSurfaceHeight;
        info.fColorType = SkImage::kPMColor_ColorType;
        info.fAlphaType = SkImage::kPremul_AlphaType;
        const SkRect fullCanvasRect = SkRect::MakeWH(
            SkIntToScalar(kSurfaceWidth), SkIntToScalar(kSurfaceHeight));
        SkSurface* surface;
#if SK_SUPPORT_GPU
        GrRenderTarget* rt = reinterpret_cast<GrRenderTarget*>(
            canvas->getDevice()->accessRenderTarget());
        if (NULL != rt) {
            surface = SkSurface::NewRenderTarget(rt->getContext(), info, rt->numSamples());
        } else
#endif
        {
            surface = SkSurface::NewRaster(info);
        }
        SkAutoTUnref<SkDeferredCanvas> drawingCanvas(SkDeferredCanvas::Create(surface));
        surface->unref();

        for (int iteration = 0; iteration < N; iteration++) {
            drawingCanvas->clear(0);
            SkAutoTUnref<SkImage> image(drawingCanvas->newImageSnapshot());
            SkPaint paint;
            if (!fDiscardableContents) {
                // If paint is not opaque, prior canvas contents are
                // not discardable because they are needed for compositing.
                paint.setAlpha(127);
            }
            drawingCanvas->drawRect(fullCanvasRect, paint);
            // Trigger copy on write, which should be faster in the discardable case.
            drawingCanvas->flush();
        }
    }

private:
    bool fDiscardableContents;

    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new DeferredSurfaceCopyBench(p, false); }
static SkBenchmark* Fact1(void* p) { return new DeferredSurfaceCopyBench(p, true); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
