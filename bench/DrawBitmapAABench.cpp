/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"

/**
 * This bench measures the rendering time of SkCanvas::drawBitmap with different anti-aliasing /
 * matrix combinations.
 */

class DrawBitmapAABench : public Benchmark {
public:
    DrawBitmapAABench(bool doAA, const SkMatrix& matrix, const char name[])
        : fMatrix(matrix)
        , fName("draw_bitmap_") {

        fPaint.setAntiAlias(doAA);
        fName.appendf("%s_%s", doAA ? "aa" : "noaa", name);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        auto surf = SkSurface::MakeRasterN32Premul(200, 200);
        surf->getCanvas()->clear(0xFF00FF00);
        fImage = surf->makeImageSnapshot();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        canvas->concat(fMatrix);
        for (int i = 0; i < loops; i++) {
            canvas->drawImage(fImage.get(), 0, 0, sampling, &fPaint);
        }
    }

private:
    SkPaint  fPaint;
    SkMatrix fMatrix;
    SkString fName;
    sk_sp<SkImage> fImage;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new DrawBitmapAABench(false, SkMatrix::I(), "ident"); )

DEF_BENCH( return new DrawBitmapAABench(false, SkMatrix::Scale(1.17f, 1.17f), "scale"); )

DEF_BENCH( return new DrawBitmapAABench(false, SkMatrix::Translate(17.5f, 17.5f), "translate"); )

DEF_BENCH(
    SkMatrix m;
    m.reset();
    m.preRotate(15);
    return new DrawBitmapAABench(false, m, "rotate");
)

DEF_BENCH( return new DrawBitmapAABench(true, SkMatrix::I(), "ident"); )

DEF_BENCH( return new DrawBitmapAABench(true, SkMatrix::Scale(1.17f, 1.17f), "scale"); )

DEF_BENCH( return new DrawBitmapAABench(true, SkMatrix::Translate(17.5f, 17.5f), "translate"); )

DEF_BENCH(
    SkMatrix m;
    m.reset();
    m.preRotate(15);
    return new DrawBitmapAABench(true, m, "rotate");
)
