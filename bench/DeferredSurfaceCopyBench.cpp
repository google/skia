
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkImage.h"
#include "SkSurface.h"
#if SK_SUPPORT_GPU
#include "GrRenderTarget.h"
#endif

class DeferredSurfaceCopyBench : public Benchmark {
    enum {
        kSurfaceWidth = 1000,
        kSurfaceHeight = 1000,
    };
public:
    DeferredSurfaceCopyBench(bool discardableContents) {
        fDiscardableContents = discardableContents;
    }

protected:
    const char* onGetName() override {
        return fDiscardableContents ? "DeferredSurfaceCopy_discardable" :
            "DeferredSurfaceCopy_nonDiscardable";
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        // The canvas is not actually used for this test except to provide
        // configuration information: gpu, multisampling, size, etc?
        SkImageInfo info = SkImageInfo::MakeN32Premul(kSurfaceWidth, kSurfaceHeight);
        const SkRect fullCanvasRect = SkRect::MakeWH(
            SkIntToScalar(kSurfaceWidth), SkIntToScalar(kSurfaceHeight));
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));

        // newSurface() can return NULL for several reasons, so we need to check
        if (NULL == surface.get()) {
            SkDebugf("DeferredSurfaceCopyBench newSurface failed, bench results are meaningless\n");
            return; // should we signal the caller that we hit an error?
        }

        SkAutoTUnref<SkDeferredCanvas> drawingCanvas(SkDeferredCanvas::Create(surface));

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

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DeferredSurfaceCopyBench(false); )
DEF_BENCH( return new DeferredSurfaceCopyBench(true); )
