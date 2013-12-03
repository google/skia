/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkRasterClip.h"

class DrawPathBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fName;
    SkPath      fPath;
    SkRasterClip fRC;
    SkBitmap    fBitmap;
    SkMatrix    fIdentity;
    SkDraw      fDraw;
    bool        fDrawCoverage;
public:
    DrawPathBench(bool drawCoverage) : fDrawCoverage(drawCoverage) {
        fPaint.setAntiAlias(true);
        fName.printf("draw_coverage_%s", drawCoverage ? "true" : "false");

        fPath.moveTo(0, 0);
        fPath.quadTo(500, 0, 500, 500);
        fPath.quadTo(250, 0, 0, 500);

        fBitmap.setConfig(SkBitmap::kA8_Config, 500, 500);
        fBitmap.allocPixels();

        fIdentity.setIdentity();
        fRC.setRect(fPath.getBounds().round());

        fDraw.fBitmap   = &fBitmap;
        fDraw.fMatrix   = &fIdentity;
        fDraw.fClip     = &fRC.bwRgn();
        fDraw.fRC       = &fRC;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DrawPathBench(false) )
DEF_BENCH( return new DrawPathBench(true) )
