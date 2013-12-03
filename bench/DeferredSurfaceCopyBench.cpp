
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
        kSurfaceWidth = 1000,
        kSurfaceHeight = 1000,
    };
public:
    DeferredSurfaceCopyBench(bool discardableContents) {
        fDiscardableContents = discardableContents;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fDiscardableContents ? "DeferredSurfaceCopy_discardable" :
            "DeferredSurfaceCopy_nonDiscardable";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        // The canvas is not actually used for this test except to provide
        // configuration information: gpu, multisampling, size, etc?
        SkImageInfo info;
        info.fWidth = kSurfaceWidth;
        info.fHeight = kSurfaceHeight;
        info.fColorType = kPMColor_SkColorType;
        info.fAlphaType = kPremul_SkAlphaType;
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

        for (int iteration = 0; iteration < loops; iteration++) {
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

DEF_BENCH( return new DeferredSurfaceCopyBench(false); )
DEF_BENCH( return new DeferredSurfaceCopyBench(true); )
