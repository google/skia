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
        LINES = 500,
    };
    SkPoint     fStartPts[LINES];
    SkPoint     fEndPts[LINES];

public:
    enum LineType {
        SH, // Straight + horizontally
        SV, // Straight + vertically
        RAND,
    };
    LineBench(SkScalar width, bool doAA, LineType type)  {
        fStrokeWidth = width;
        fDoAA = doAA;
        fName.printf("lines_%g_%s_%s", width, doAA ? "AA" : "BW",
                     type == SH ? "SH" : (type == SV ? "SV" : "RAND"));

        SkRandom rand;
        for (int i = 0; i < LINES; ++i) {
            fStartPts[i].set(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
            if (type == SH) {
                fEndPts[i].set(rand.nextUScalar1() * 640, fStartPts[i].y());
            } else if (type == SV) {
                fEndPts[i].set(fStartPts[i].x(), rand.nextUScalar1() * 480);
            } else {
                fEndPts[i].set(rand.nextUScalar1() * 640, rand.nextUScalar1() * 480);
            }
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(fDoAA);
        paint.setStrokeWidth(fStrokeWidth);

        for (int i = 0; i < loops; i++) {
            canvas->drawLine(fStartPts[i].x(), fStartPts[i].y(), fEndPts[i].x(), fEndPts[i].y(), paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new LineBench(0,            false, LineBench::SH);)
DEF_BENCH(return new LineBench(SK_Scalar1,   false, LineBench::SH);)
DEF_BENCH(return new LineBench(0,            true, LineBench::SH);)
DEF_BENCH(return new LineBench(SK_Scalar1/2, true, LineBench::SH);)
DEF_BENCH(return new LineBench(SK_Scalar1,   true, LineBench::SH);)
DEF_BENCH(return new LineBench(SK_Scalar1*10,true, LineBench::SH);)
DEF_BENCH(return new LineBench(0,            false, LineBench::SV);)
DEF_BENCH(return new LineBench(SK_Scalar1,   false, LineBench::SV);)
DEF_BENCH(return new LineBench(0,            true, LineBench::SV);)
DEF_BENCH(return new LineBench(SK_Scalar1/2, true, LineBench::SV);)
DEF_BENCH(return new LineBench(SK_Scalar1,   true, LineBench::SV);)
DEF_BENCH(return new LineBench(SK_Scalar1*10,true, LineBench::SV);)
DEF_BENCH(return new LineBench(0,            false, LineBench::RAND);)
DEF_BENCH(return new LineBench(SK_Scalar1,   false, LineBench::RAND);)
DEF_BENCH(return new LineBench(0,            true, LineBench::RAND);)
DEF_BENCH(return new LineBench(SK_Scalar1/2, true, LineBench::RAND);)
DEF_BENCH(return new LineBench(SK_Scalar1,   true, LineBench::RAND);)
DEF_BENCH(return new LineBench(SK_Scalar1*10,true, LineBench::RAND);)
