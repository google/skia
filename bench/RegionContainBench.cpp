/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkString.h"

static bool sect_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kIntersect_Op);
}

class RegionContainBench : public SkBenchmark {
public:
    typedef bool (*Proc)(SkRegion& a, SkRegion& b);
    SkRegion fA, fB;
    Proc     fProc;
    SkString fName;

    enum {
        W = 200,
        H = 200,
        COUNT = 10,
        N = SkBENCHLOOP(20000)
    };

    SkIRect randrect(SkRandom& rand, int i) {
        int w = rand.nextU() % W;
        return SkIRect::MakeXYWH(0, i*H/COUNT, w, H/COUNT);
    }

    RegionContainBench(void* param, Proc proc, const char name[]) : INHERITED(param) {
        fProc = proc;
        fName.printf("region_contains_%s", name);

        SkRandom rand;
        for (int i = 0; i < COUNT; i++) {
            fA.op(randrect(rand, i), SkRegion::kXOR_Op);
        }

        fB.setRect(0, 0, H, W);

        fIsRendering = false;
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }

    virtual void onDraw(SkCanvas*) {
        Proc proc = fProc;

        for (int i = 0; i < N; ++i) {
           proc(fA, fB);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* gF0(void* p) { return SkNEW_ARGS(RegionContainBench, (p, sect_proc, "sect")); }

static BenchRegistry gR0(gF0);
