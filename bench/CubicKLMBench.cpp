/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "src/core/SkGeometry.h"
#include "src/gpu/geometry/GrPathUtils.h"

class CubicKLMBench : public Benchmark {
public:
    CubicKLMBench(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1,
                  SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3)  {
        fPoints[0].set(x0, y0);
        fPoints[1].set(x1, y1);
        fPoints[2].set(x2, y2);
        fPoints[3].set(x3, y3);

        fName = "cubic_klm_";
        switch (SkClassifyCubic(fPoints)) {
            case SkCubicType::kSerpentine:
                fName.append("serp");
                break;
            case SkCubicType::kLoop:
                fName.append("loop");
                break;
            default:
                SK_ABORT("Unexpected cubic type");
                break;
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        SkPoint dst[10];
        SkMatrix klm;
        int loopIdx;
        for (int i = 0; i < loops * 50000; ++i) {
            GrPathUtils::chopCubicAtLoopIntersection(fPoints, dst, &klm, &loopIdx);
        }
    }

private:
    SkPoint     fPoints[4];
    SkString    fName;

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new CubicKLMBench(285.625f, 499.687f, 411.625f, 808.188f,
                                    1064.62f, 135.688f, 1042.63f, 585.187f); )
DEF_BENCH( return new CubicKLMBench(635.625f, 614.687f, 171.625f, 236.188f,
                                    1064.62f, 135.688f, 516.625f, 570.187f); )
