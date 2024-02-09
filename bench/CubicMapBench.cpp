/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCubicMap.h"

class CubicMapBench : public Benchmark {
public:
    CubicMapBench(SkPoint p1, SkPoint p2) : fCMap(p1, p2) {
        fName.printf("cubicmap_%g_%g_%g_%g", p1.fX, p1.fY, p2.fX, p2.fY);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int outer = 0; outer < 100; ++outer) {
            for (int i = 0; i < loops; ++i) {
                for (SkScalar x = 0; x <= 1; x += 1.0f / 512) {
                    fCMap.computeYFromX(x);
                }
            }
        }
    }

private:
    SkCubicMap  fCMap;
    SkString    fName;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new CubicMapBench({1, 0}, {0,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {0,1}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 1}, {0,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {0,1}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 0}, {1,1}); )
DEF_BENCH( return new CubicMapBench({1, 1}, {0,0}); )
