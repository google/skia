/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkString.h"

struct BezierRec {
    SkCanvas*   fCanvas;
    SkPaint     fPaint;
    SkPath      fQuad;
    SkPath      fCubic;
};

typedef const char* (*DrawProc)(const BezierRec*, int);

static const char* draw_quad(const BezierRec* rec, int count) {
    if (rec) {
        SkCanvas* canvas = rec->fCanvas;
        const SkPaint& paint = rec->fPaint;
        const SkPath& path = rec->fQuad;
        for (int i = 0; i < count; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    return "quad";
}

static const char* draw_cubic(const BezierRec* rec, int count) {
    if (rec) {
        SkCanvas* canvas = rec->fCanvas;
        const SkPaint& paint = rec->fPaint;
        const SkPath& path = rec->fCubic;
        for (int i = 0; i < count; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    return "cubic";
}

class BezierBench : public Benchmark {
    SkString fName;
    SkPaint::Cap fCap;
    SkPaint::Join fJoin;
    BezierRec fRec;
    DrawProc fProc;
    SkScalar fWidth;
public:
    BezierBench(SkPaint::Cap c, SkPaint::Join j, SkScalar w, DrawProc proc) {
        static const char* gCapName[] = {
            "butt", "round", "square"
        };
        static const char* gJoinName[] = {
            "miter", "round", "bevel"
        };

        fCap = c;
        fJoin = j;
        fProc = proc;
        fWidth = SkIntToScalar(w);
        fName.printf("draw_stroke_bezier_%s_%s_%s_%g", proc(nullptr, 0), gCapName[c], gJoinName[j], w);

        SkPathBuilder builder;

        builder.moveTo(20, 20).quadTo(60, 20, 60, 60).quadTo(20, 60, 20, 100);
        fRec.fQuad = builder.detach();

        builder.moveTo(20, 20).cubicTo(40, 20, 60, 40, 60, 60).cubicTo(40, 60, 20, 80, 20, 100);
        fRec.fCubic = builder.detach();
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        fRec.fCanvas = canvas;
        this->setupPaint(&fRec.fPaint);
        fRec.fPaint.setStyle(SkPaint::kStroke_Style);
        fRec.fPaint.setStrokeCap(fCap);
        fRec.fPaint.setStrokeJoin(fJoin);
        fRec.fPaint.setStrokeWidth(fWidth);
        fProc(&fRec, loops);
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH( return new BezierBench(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2, draw_quad); )
DEF_BENCH( return new BezierBench(SkPaint::kSquare_Cap, SkPaint::kBevel_Join, 10, draw_quad); )
DEF_BENCH( return new BezierBench(SkPaint::kRound_Cap, SkPaint::kMiter_Join, 50, draw_quad); )

DEF_BENCH( return new BezierBench(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2, draw_cubic); )
DEF_BENCH( return new BezierBench(SkPaint::kSquare_Cap, SkPaint::kBevel_Join, 10, draw_cubic); )
DEF_BENCH( return new BezierBench(SkPaint::kRound_Cap, SkPaint::kMiter_Join, 50, draw_cubic); )
