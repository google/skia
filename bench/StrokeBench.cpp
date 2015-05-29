/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkString.h"

class StrokeBench : public Benchmark {
public:
    StrokeBench(const SkPath& path, const SkPaint& paint, const char pathType[])
        : fPath(path), fPaint(paint)
    {
        fName.printf("build_stroke_%s_%g_%d_%d",
                     pathType, paint.getStrokeWidth(), paint.getStrokeJoin(), paint.getStrokeCap());
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        for (int outer = 0; outer < 10; ++outer) {
            for (int i = 0; i < loops; ++i) {
                SkPath result;
                paint.getFillPath(fPath, &result);
            }
        }
    }

private:
    SkPath      fPath;
    SkPaint     fPaint;
    SkString    fName;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static const int N = 100;
static const SkScalar X = 100;
static const SkScalar Y = 100;

static SkPoint rand_pt(SkRandom& rand) {
    return SkPoint::Make(rand.nextSScalar1() * X, rand.nextSScalar1() * Y);
}

static SkPath line_path_maker() {
    SkPath path;
    SkRandom rand;
    path.moveTo(rand_pt(rand));
    for (int i = 0; i < N; ++i) {
        path.lineTo(rand_pt(rand));
    }
    return path;
}
static SkPath quad_path_maker() {
    SkPath path;
    SkRandom rand;
    path.moveTo(rand_pt(rand));
    for (int i = 0; i < N; ++i) {
        path.quadTo(rand_pt(rand), rand_pt(rand));
    }
    return path;
}
static SkPath conic_path_maker() {
    SkPath path;
    SkRandom rand;
    path.moveTo(rand_pt(rand));
    for (int i = 0; i < N; ++i) {
        path.conicTo(rand_pt(rand), rand_pt(rand), rand.nextUScalar1());
    }
    return path;
}
static SkPath cubic_path_maker() {
    SkPath path;
    SkRandom rand;
    path.moveTo(rand_pt(rand));
    for (int i = 0; i < N; ++i) {
        path.cubicTo(rand_pt(rand), rand_pt(rand), rand_pt(rand));
    }
    return path;
}

static SkPaint paint_maker() {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(X / 10);
    paint.setStrokeJoin(SkPaint::kMiter_Join);
    paint.setStrokeCap(SkPaint::kSquare_Cap);
    return paint;
}

DEF_BENCH( return SkNEW_ARGS(StrokeBench, (line_path_maker(), paint_maker(), "line")); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (quad_path_maker(), paint_maker(), "quad")); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (conic_path_maker(), paint_maker(), "conic")); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (cubic_path_maker(), paint_maker(), "cubic")); )
