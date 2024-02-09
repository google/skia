/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkRegion.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

static bool sect_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kIntersect_Op);
}

class RegionContainBench : public Benchmark {
public:
    typedef bool (*Proc)(SkRegion& a, SkRegion& b);
    SkRegion fA, fB;
    Proc     fProc;
    SkString fName;

    enum {
        W = 200,
        H = 200,
        COUNT = 10,
    };

    SkIRect randrect(SkRandom& rand, int i) {
        int w = rand.nextU() % W;
        return SkIRect::MakeXYWH(0, i*H/COUNT, w, H/COUNT);
    }

    RegionContainBench(Proc proc, const char name[])  {
        fProc = proc;
        fName.printf("region_contains_%s", name);

        SkRandom rand;
        for (int i = 0; i < COUNT; i++) {
            fA.op(randrect(rand, i), SkRegion::kXOR_Op);
        }

        fB.setRect({0, 0, H, W});
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        Proc proc = fProc;

        for (int i = 0; i < loops; ++i) {
           proc(fA, fB);
        }
    }

private:
    using INHERITED = Benchmark;
};

DEF_BENCH(return new RegionContainBench(sect_proc, "sect");)
