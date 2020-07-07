/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"

class RasterTileBench : public Benchmark {
    sk_sp<SkSurface> fSurf;
    SkPath           fPath;
    SkString       fName;
public:
    RasterTileBench() : fName("rastertile") {
        int W = 2014 * 20;
        int H = 20;
        fSurf = SkSurface::MakeRasterN32Premul(W, H);

        fPath.moveTo(0, 0);
        fPath.cubicTo(20, 10, 10, 15, 30, 5);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1.1f);
        paint.setAntiAlias(true);

        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < 1000; ++j) {
                fSurf->getCanvas()->drawPath(fPath, paint);
            }
        }
    }

private:
};
DEF_BENCH(return new RasterTileBench;)
