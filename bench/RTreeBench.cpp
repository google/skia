/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkRTree.h"

// confine rectangles to a smallish area, so queries generally hit something, and overlap occurs:
static const SkScalar GENERATE_EXTENTS = 1000.0f;
static const int NUM_BUILD_RECTS = 500;
static const int NUM_QUERY_RECTS = 5000;
static const int GRID_WIDTH = 100;

typedef SkRect (*MakeRectProc)(SkRandom&, int, int);

// Time how long it takes to build an R-Tree.
class RTreeBuildBench : public Benchmark {
public:
    RTreeBuildBench(const char* name, MakeRectProc proc) : fProc(proc) {
        fName.printf("rtree_%s_build", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }
    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        SkAutoTMalloc<SkRect> rects(NUM_BUILD_RECTS);
        for (int i = 0; i < NUM_BUILD_RECTS; ++i) {
            rects[i] = fProc(rand, i, NUM_BUILD_RECTS);
        }

        for (int i = 0; i < loops; ++i) {
            SkRTree tree;
            tree.insert(rects.get(), NUM_BUILD_RECTS);
            SkASSERT(rects != nullptr);  // It'd break this bench if the tree took ownership of rects.
        }
    }
private:
    MakeRectProc fProc;
    SkString fName;
    typedef Benchmark INHERITED;
};

// Time how long it takes to perform queries on an R-Tree.
class RTreeQueryBench : public Benchmark {
public:
    RTreeQueryBench(const char* name, MakeRectProc proc) : fProc(proc) {
        fName.printf("rtree_%s_query", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
protected:
    const char* onGetName() override {
        return fName.c_str();
    }
    void onDelayedSetup() override {
        SkRandom rand;
        SkAutoTMalloc<SkRect> rects(NUM_QUERY_RECTS);
        for (int i = 0; i < NUM_QUERY_RECTS; ++i) {
            rects[i] = fProc(rand, i, NUM_QUERY_RECTS);
        }
        fTree.insert(rects.get(), NUM_QUERY_RECTS);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRandom rand;
        for (int i = 0; i < loops; ++i) {
            std::vector<int> hits;
            SkRect query;
            query.fLeft   = rand.nextRangeF(0, GENERATE_EXTENTS);
            query.fTop    = rand.nextRangeF(0, GENERATE_EXTENTS);
            query.fRight  = query.fLeft + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/2);
            query.fBottom = query.fTop  + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/2);
            fTree.search(query, &hits);
        }
    }
private:
    SkRTree fTree;
    MakeRectProc fProc;
    SkString fName;
    typedef Benchmark INHERITED;
};

static inline SkRect make_XYordered_rects(SkRandom& rand, int index, int numRects) {
    SkRect out;
    out.fLeft   = SkIntToScalar(index % GRID_WIDTH);
    out.fTop    = SkIntToScalar(index / GRID_WIDTH);
    out.fRight  = out.fLeft + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/3);
    out.fBottom = out.fTop  + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/3);
    return out;
}
static inline SkRect make_YXordered_rects(SkRandom& rand, int index, int numRects) {
    SkRect out;
    out.fLeft   = SkIntToScalar(index / GRID_WIDTH);
    out.fTop    = SkIntToScalar(index % GRID_WIDTH);
    out.fRight  = out.fLeft + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/3);
    out.fBottom = out.fTop  + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/3);
    return out;
}

static inline SkRect make_random_rects(SkRandom& rand, int index, int numRects) {
    SkRect out;
    out.fLeft   = rand.nextRangeF(0, GENERATE_EXTENTS);
    out.fTop    = rand.nextRangeF(0, GENERATE_EXTENTS);
    out.fRight  = out.fLeft + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/5);
    out.fBottom = out.fTop  + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/5);
    return out;
}

static inline SkRect make_concentric_rects(SkRandom&, int index, int numRects) {
    return SkRect::MakeWH(SkIntToScalar(index+1), SkIntToScalar(index+1));
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new RTreeBuildBench("XY", &make_XYordered_rects));
DEF_BENCH(return new RTreeBuildBench("YX", &make_YXordered_rects));
DEF_BENCH(return new RTreeBuildBench("random", &make_random_rects));
DEF_BENCH(return new RTreeBuildBench("concentric", &make_concentric_rects));

DEF_BENCH(return new RTreeQueryBench("XY", &make_XYordered_rects));
DEF_BENCH(return new RTreeQueryBench("YX", &make_YXordered_rects));
DEF_BENCH(return new RTreeQueryBench("random", &make_random_rects));
DEF_BENCH(return new RTreeQueryBench("concentric", &make_concentric_rects));
