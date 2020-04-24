/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

class PathOpsBench : public Benchmark {
    SkString    fName;
    SkPath      fPath1, fPath2;
    SkPathOp    fOp;

public:
    PathOpsBench(const char suffix[], SkPathOp op) : fOp(op) {
        fName.printf("pathops_%s", suffix);

        fPath1.addOval({-10, -20, 10, 20});
        fPath2.addOval({-20, -10, 20, 10});
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < 1000; ++j) {
                SkPath result;
                Op(fPath1, fPath2, fOp, &result);
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

class PathOpsSimplifyBench : public Benchmark {
    SkString    fName;
    SkPath      fPath;

public:
    PathOpsSimplifyBench(const char suffix[], const SkPath& path) : fPath(path) {
        fName.printf("pathops_simplify_%s", suffix);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < 100; ++j) {
                SkPath result;
                Simplify(fPath, &result);
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};


DEF_BENCH( return new PathOpsBench("sect", kIntersect_SkPathOp); )
DEF_BENCH( return new PathOpsBench("join", kUnion_SkPathOp); )

static SkPath makerects() {
    SkRandom rand;
    SkPath path;
    SkScalar scale = 100;
    for (int i = 0; i < 20; ++i) {
        SkScalar x = rand.nextUScalar1() * scale;
        SkScalar y = rand.nextUScalar1() * scale;
        path.addRect({x, y, x + scale, y + scale});
    }
    return path;
}

DEF_BENCH( return new PathOpsSimplifyBench("rects", makerects()); )
