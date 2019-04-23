/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "src/core/SkGeometry.h"
#include "src/gpu/ccpr/GrCCFillGeometry.h"

static int kNumBaseLoops = 50000;

class GrCCGeometryBench : public Benchmark {
public:
    GrCCGeometryBench(float x0, float y0, float x1, float y1,
                      float x2, float y2, float x3, float y3, const char* extraName)  {
        fPoints[0].set(x0, y0);
        fPoints[1].set(x1, y1);
        fPoints[2].set(x2, y2);
        fPoints[3].set(x3, y3);
        fPoints[4].set(x0, y0); // Flat closing edge.

        fName = "ccprgeometry";
        switch (SkClassifyCubic(fPoints)) {
            case SkCubicType::kSerpentine:
                fName.append("_serp");
                break;
            case SkCubicType::kLoop:
                fName.append("_loop");
                break;
            default:
                SK_ABORT("Unexpected cubic type");
                break;
        }

        fName.appendf("_%s", extraName);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int j = 0; j < loops; ++j) {
            fGeometry.beginContour(fPoints[0]);
            for (int i = 0; i < kNumBaseLoops; ++i) {
                fGeometry.cubicTo(fPoints);
                fGeometry.lineTo(fPoints+3);
            }
            fGeometry.endContour();
            fGeometry.reset();
        }
    }

private:
    SkPoint fPoints[5];
    SkString fName;
    GrCCFillGeometry fGeometry{4*100*kNumBaseLoops, 2*100*kNumBaseLoops};

    typedef Benchmark INHERITED;
};

// Loops.
DEF_BENCH( return new GrCCGeometryBench(529.049988f, 637.050049f, 335.750000f, -135.950012f,
                                        912.750000f, 560.949951f, 59.049988f, 295.950012f,
                                        "2_roots"); )

DEF_BENCH( return new GrCCGeometryBench(182.050003f, 300.049988f, 490.750000f, 111.049988f,
                                        482.750000f, 500.950012f, 451.049988f, 553.950012f,
                                        "1_root"); )

DEF_BENCH( return new GrCCGeometryBench(498.049988f, 476.049988f, 330.750000f, 330.049988f,
                                        222.750000f, 389.950012f, 169.049988f, 542.950012f,
                                        "0_roots"); )

// Serpentines.
DEF_BENCH( return new GrCCGeometryBench(529.049988f, 714.049988f, 315.750000f, 196.049988f,
                                        484.750000f, 110.950012f, 349.049988f, 630.950012f,
                                        "2_roots"); )

DEF_BENCH( return new GrCCGeometryBench(513.049988f, 245.049988f, 73.750000f, 137.049988f,
                                        508.750000f, 657.950012f, 99.049988f, 601.950012f,
                                        "1_root"); )

DEF_BENCH( return new GrCCGeometryBench(560.049988f, 364.049988f, 217.750000f, 314.049988f,
                                        21.750000f, 364.950012f, 83.049988f, 624.950012f,
                                        "0_roots"); )
