/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkRTree.h"
#include "SkRandom.h"
#include "SkString.h"

// confine rectangles to a smallish area, so queries generally hit something, and overlap occurs:
static const SkScalar GENERATE_EXTENTS = 1000.0f;
static const int NUM_BUILD_RECTS = 500;
static const int NUM_QUERY_RECTS = 5000;
static const int GRID_WIDTH = 100;

typedef SkRect (*MakeRectProc)(SkRandom&, int, int);

// Time how long it takes to build an R-Tree either bulk-loaded or not
class RTreeBuildBench : public Benchmark {
public:
    RTreeBuildBench(const char* name, MakeRectProc proc, SkRTree* tree)
        : fTree(tree)
        , fProc(proc) {
        fName.printf("rtree_%s_build", name);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual ~RTreeBuildBench() {
        fTree->unref();
    }
protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom rand;
        SkAutoTMalloc<SkRect> rects(NUM_BUILD_RECTS);
        for (int i = 0; i < NUM_BUILD_RECTS; ++i) {
            rects[i] = fProc(rand, i, NUM_BUILD_RECTS);
        }

        for (int i = 0; i < loops; ++i) {
            fTree->insert(&rects, NUM_BUILD_RECTS);
            SkASSERT(rects != NULL);  // It'd break this bench if the tree took ownership of rects.
            fTree->clear();
        }
    }
private:
    SkRTree* fTree;
    MakeRectProc fProc;
    SkString fName;
    typedef Benchmark INHERITED;
};

// Time how long it takes to perform queries on an R-Tree, bulk-loaded or not
class RTreeQueryBench : public Benchmark {
public:
    RTreeQueryBench(const char* name, MakeRectProc proc, SkRTree* tree)
        : fTree(tree)
        , fProc(proc) {
        fName.printf("rtree_%s_query", name);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual ~RTreeQueryBench() {
        fTree->unref();
    }
protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }
    virtual void onPreDraw() SK_OVERRIDE {
        SkRandom rand;
        SkAutoTMalloc<SkRect> rects(NUM_QUERY_RECTS);
        for (int i = 0; i < NUM_QUERY_RECTS; ++i) {
            rects[i] = fProc(rand, i, NUM_QUERY_RECTS);
        }
        fTree->insert(&rects, NUM_QUERY_RECTS);
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom rand;
        for (int i = 0; i < loops; ++i) {
            SkTDArray<unsigned> hits;
            SkRect query;
            query.fLeft   = rand.nextRangeF(0, GENERATE_EXTENTS);
            query.fTop    = rand.nextRangeF(0, GENERATE_EXTENTS);
            query.fRight  = query.fLeft + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/2);
            query.fBottom = query.fTop  + 1 + rand.nextRangeF(0, GENERATE_EXTENTS/2);
            fTree->search(query, &hits);
        }
    }
private:
    SkBBoxHierarchy* fTree;
    MakeRectProc fProc;
    SkString fName;
    typedef Benchmark INHERITED;
};

static inline SkRect make_concentric_rects_increasing(SkRandom&, int index, int numRects) {
    SkRect out = SkRect::MakeWH(SkIntToScalar(index+1), SkIntToScalar(index+1));
    return out;
}

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

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("XY_sorted", &make_XYordered_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("XY_unsorted", &make_XYordered_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("YX_sorted", &make_YXordered_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("YX_unsorted", &make_YXordered_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("random_sorted", &make_random_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("random_unsorted", &make_random_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("concentric_sorted", &make_concentric_rects_increasing, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeBuildBench,
            ("concentric_unsorted",
             &make_concentric_rects_increasing,
             SkRTree::Create(5, 16, 1, false)));)

DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("XY_sorted", &make_XYordered_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("XY_unsorted", &make_XYordered_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("YX_sorted", &make_YXordered_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("YX_unsorted", &make_YXordered_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("random_sorted", &make_random_rects, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("random_unsorted", &make_random_rects, SkRTree::Create(5, 16, 1, false)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("concentric_sorted", &make_concentric_rects_increasing, SkRTree::Create(5, 16)));)
DEF_BENCH(return SkNEW_ARGS(RTreeQueryBench,
            ("concentric_unsorted",
             &make_concentric_rects_increasing,
             SkRTree::Create(5, 16, 1, false)));)
