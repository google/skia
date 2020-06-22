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

#include "include/core/SkPathBuilder.h"

template <typename T> void run_builder(T& b, bool useReserve) {
    const int N = 100;
    float x = 0, y = 0;
    b.moveTo(x, y);

    if (useReserve) {
        b.incReserve(N * 12);
    }
    for (int i = 0; i < N; ++i) {
        b.lineTo(x, y);
        b.quadTo(x, y, x, y);
        b.cubicTo(x, y, x, y, x, y);
    }
}

enum class MakeType {
    kPath,
    kSnapshot,
    kDetach,
};

class PathBuilderBench : public Benchmark {
    SkString    fName;
    MakeType    fMakeType;
    bool        fUseReserve;

public:
    PathBuilderBench(MakeType mt, bool reserve) : fMakeType(mt), fUseReserve(reserve) {
        const char* typenames[] = { "path", "snapshot", "detach" };

        fName.printf("makepath_%s_%s", typenames[(int)mt], reserve ? "reserve" : "noreserve");
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkPath build() {
        switch (fMakeType) {
            case MakeType::kSnapshot:
            case MakeType::kDetach: {
                SkPathBuilder b;
                run_builder(b, fUseReserve);
                return MakeType::kSnapshot == fMakeType ? b.snapshot() : b.detach();
            }
            case MakeType::kPath: {
                SkPath p;
                run_builder(p, fUseReserve);
                return p;
            }
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < 100; ++j) {
                SkPath result = this->build();
                // force bounds calc as part of the test
                if (!result.getBounds().isFinite()) {
                    SkDebugf("should never get here!\n");
                    return;
                }
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};
DEF_BENCH( return new PathBuilderBench(MakeType::kPath, false); )
DEF_BENCH( return new PathBuilderBench(MakeType::kSnapshot, false); )
DEF_BENCH( return new PathBuilderBench(MakeType::kDetach, false); )
DEF_BENCH( return new PathBuilderBench(MakeType::kPath, true); )
DEF_BENCH( return new PathBuilderBench(MakeType::kSnapshot, true); )
DEF_BENCH( return new PathBuilderBench(MakeType::kDetach, true); )
