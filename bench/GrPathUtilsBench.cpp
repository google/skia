/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "src/gpu/geometry/GrPathUtils.h"

class FindCubicConvex180Chops : public Benchmark {
public:
    FindCubicConvex180Chops(const std::array<SkPoint,4>& pts, const char* suffix) : fPts(pts) {
        fName.printf("GrPathUtils_findCubicConvex180Chops%s", suffix);
    }

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) final { return backend == kNonRendering_Backend; }
    void onDraw(int loops, SkCanvas*) final {
        float T[2] = {0};
        bool areCusps;
        int iters = 50000 * loops;
        for (int i = 0; i < iters; ++i) {
            int count = GrPathUtils::findCubicConvex180Chops(fPts.data(), T, &areCusps);
            if (T[0] == 200.7f) {
                // This will never happen. Pretend to use the result to keep the compiler honest.
                SkDebugf("%i%f%f", count, T[0], T[1]);
            }
        }
    }

    SkString fName;
    std::array<SkPoint,4> fPts;
};

DEF_BENCH(return new FindCubicConvex180Chops({{{0,0}, {100,0}, {50,100}, {100,100}}}, "_inflect1");)
DEF_BENCH(return new FindCubicConvex180Chops({{{0,0}, {50,0}, {100,50}, {100,100}}}, "_loop");)
