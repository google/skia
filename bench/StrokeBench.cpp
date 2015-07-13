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
    StrokeBench(const SkPath& path, const SkPaint& paint, const char pathType[], SkScalar res)
        : fPath(path), fPaint(paint), fRes(res)
    {
        fName.printf("build_stroke_%s_%g_%d_%d",
                     pathType, paint.getStrokeWidth(), paint.getStrokeJoin(), paint.getStrokeCap());
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        for (int outer = 0; outer < 10; ++outer) {
            for (int i = 0; i < loops; ++i) {
                SkPath result;
                paint.getFillPath(fPath, &result, NULL, fRes);
            }
        }
    }

private:
    SkPath      fPath;
    SkPaint     fPaint;
    SkString    fName;
    SkScalar    fRes;
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

DEF_BENCH( return SkNEW_ARGS(StrokeBench, (line_path_maker(), paint_maker(), "line_1", 1)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (quad_path_maker(), paint_maker(), "quad_1", 1)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (conic_path_maker(), paint_maker(), "conic_1", 1)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (cubic_path_maker(), paint_maker(), "cubic_1", 1)); )

DEF_BENCH( return SkNEW_ARGS(StrokeBench, (line_path_maker(), paint_maker(), "line_4", 4)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (quad_path_maker(), paint_maker(), "quad_4", 4)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (conic_path_maker(), paint_maker(), "conic_4", 4)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (cubic_path_maker(), paint_maker(), "cubic_4", 4)); )

DEF_BENCH( return SkNEW_ARGS(StrokeBench, (line_path_maker(), paint_maker(), "line_.25", .25f)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (quad_path_maker(), paint_maker(), "quad_.25", .25f)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (conic_path_maker(), paint_maker(), "conic_.25", .25f)); )
DEF_BENCH( return SkNEW_ARGS(StrokeBench, (cubic_path_maker(), paint_maker(), "cubic_.25", .25f)); )
