
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkString.h"

static bool union_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kUnion_Op);
}

static bool sect_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kIntersect_Op);
}

static bool diff_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kDifference_Op);
}

static bool contains_proc(SkRegion& a, SkRegion& b) {
    SkIRect r = b.getBounds();
    r.inset(r.width()/4, r.height()/4);
    return a.contains(r);
}

static bool sects_proc(SkRegion& a, SkRegion& b) {
    return a.intersects(b);
}

class RegionBench : public SkBenchmark {
public:
    typedef bool (*Proc)(SkRegion& a, SkRegion& b);

    SkRegion fA, fB;
    Proc     fProc;
    SkString fName;
    
    enum {
        W = 1024,
        H = 768,
        N = SkBENCHLOOP(5000)
    };

    SkIRect randrect(SkRandom& rand) {
        int x = rand.nextU() % W;
        int y = rand.nextU() % H;
        int w = rand.nextU() % W;
        int h = rand.nextU() % H;
        return SkIRect::MakeXYWH(x, y, w >> 1, h >> 1);
    }

    RegionBench(void* param, int count, Proc proc, const char name[]) : INHERITED(param) {
        fProc = proc;
        fName.printf("Region_%s_%d", name, count);

        SkRandom rand;
        for (int i = 0; i < N; i++) {
            fA.op(randrect(rand), SkRegion::kUnion_Op);
            fB.op(randrect(rand), SkRegion::kUnion_Op);
        }
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }

    virtual void onDraw(SkCanvas* canvas) {
        Proc proc = fProc;
        for (int i = 0; i < N; ++i) {
            proc(fA, fB);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

#define SMALL   64

static SkBenchmark* gF0(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, union_proc, "union")); }
static SkBenchmark* gF1(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, sect_proc, "intersect")); }
static SkBenchmark* gF2(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, diff_proc, "difference")); }
static SkBenchmark* gF3(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, contains_proc, "contains")); }
static SkBenchmark* gF4(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, sects_proc, "intersects")); }

static BenchRegistry gR0(gF0);
static BenchRegistry gR1(gF1);
static BenchRegistry gR2(gF2);
static BenchRegistry gR3(gF3);
static BenchRegistry gR4(gF4);
