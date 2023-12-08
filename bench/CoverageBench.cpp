/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"

class DrawPathBench : public Benchmark {
    SkPaint     fPaint;
    SkString    fName;
    SkPath      fPath;
    SkRasterClip fRC;
    SkAutoPixmapStorage fPixmap;
    SkDraw      fDraw;
    bool        fDrawCoverage;
public:
    DrawPathBench(bool drawCoverage) : fDrawCoverage(drawCoverage) {
        fName.printf("draw_coverage_%s", drawCoverage ? "true" : "false");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        fPaint.setAntiAlias(true);

        fPath.moveTo(0, 0);
        fPath.quadTo(500, 0, 500, 500);
        fPath.quadTo(250, 0, 0, 500);

        fPixmap.alloc(SkImageInfo::MakeA8(500, 500));
        if (!fDrawCoverage) {
            // drawPathCoverage() goes out of its way to work fine with an uninitialized
            // dst buffer, even in "SrcOver" mode, but ordinary drawing sure doesn't.
            fPixmap.erase(0);
        }

        fRC.setRect(fPath.getBounds().round());

        fDraw.fDst = fPixmap;
        fDraw.fCTM = &SkMatrix::I();
        fDraw.fRC = &fRC;
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        if (fDrawCoverage) {
            for (int i = 0; i < loops; ++i) {
                fDraw.drawPathCoverage(fPath, fPaint);
            }
        } else {
            for (int i = 0; i < loops; ++i) {
                fDraw.drawPath(fPath, fPaint);
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DrawPathBench(false) )
DEF_BENCH( return new DrawPathBench(true) )
