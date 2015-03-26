/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkTArray.h"


class LineBench : public Benchmark {
    SkScalar    fStrokeWidth;
    bool        fDoAA;
    SkString    fName;
    enum {
        PTS = 500,
    };
    SkPoint fPts[PTS];

public:
    LineBench(SkScalar width, bool doAA)  {
        fStrokeWidth = width;
        fDoAA = doAA;
        fName.printf("lines_%g_%s", width, doAA ? "AA" : "BW");

        SkRandom rand;
        for (int i = 0; i < PTS; ++i) {
            fPts[i].set(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(fDoAA);
        paint.setStrokeWidth(fStrokeWidth);

        for (int i = 0; i < loops; i++) {
            canvas->drawPoints(SkCanvas::kLines_PointMode, PTS, fPts, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new LineBench(0,            false);)
DEF_BENCH(return new LineBench(SK_Scalar1,   false);)
DEF_BENCH(return new LineBench(0,            true);)
DEF_BENCH(return new LineBench(SK_Scalar1/2, true);)
DEF_BENCH(return new LineBench(SK_Scalar1,   true);)
