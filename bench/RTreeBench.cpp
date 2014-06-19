
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
static const int GENERATE_EXTENTS = 1000;
static const int NUM_BUILD_RECTS = 500;
static const int NUM_QUERY_RECTS = 5000;
static const int GRID_WIDTH = 100;

typedef SkIRect (*MakeRectProc)(SkRandom&, int, int);

// Time how long it takes to build an R-Tree either bulk-loaded or not
class RTreeBuildBench : public Benchmark {
public:
    RTreeBuildBench(const char* name, MakeRectProc proc, bool bulkLoad,
                    SkBBoxHierarchy* tree)
        : fTree(tree)
        , fProc(proc)
        , fBulkLoad(bulkLoad) {
        fName.append("rtree_");
        fName.append(name);
        fName.append("_build");
        if (fBulkLoad) {
            fName.append("_bulk");
        }
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
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < NUM_BUILD_RECTS; ++j) {
                fTree->insert(reinterpret_cast<void*>(j), fProc(rand, j, NUM_BUILD_RECTS),
                              fBulkLoad);
            }
            fTree->flushDeferredInserts();
            fTree->clear();
        }
    }
private:
    SkBBoxHierarchy* fTree;
    MakeRectProc fProc;
    SkString fName;
    bool fBulkLoad;
    typedef Benchmark INHERITED;
};

// Time how long it takes to perform queries on an R-Tree, bulk-loaded or not
class RTreeQueryBench : public Benchmark {
public:
    enum QueryType {
        kSmall_QueryType, // small queries
        kLarge_QueryType, // large queries
        kRandom_QueryType,// randomly sized queries
        kFull_QueryType   // queries that cover everything
    };

    RTreeQueryBench(const char* name, MakeRectProc proc, bool bulkLoad,
                    QueryType q, SkBBoxHierarchy* tree)
        : fTree(tree)
        , fProc(proc)
        , fBulkLoad(bulkLoad)
        , fQuery(q) {
        fName.append("rtree_");
        fName.append(name);
        fName.append("_query");
        if (fBulkLoad) {
            fName.append("_bulk");
        }
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
        for (int j = 0; j < NUM_QUERY_RECTS; ++j) {
            fTree->insert(reinterpret_cast<void*>(j),
                          fProc(rand, j, NUM_QUERY_RECTS),
                          fBulkLoad);
        }
        fTree->flushDeferredInserts();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom rand;
        for (int i = 0; i < loops; ++i) {
            SkTDArray<void*> hits;
            SkIRect query;
            switch(fQuery) {
                case kSmall_QueryType:
                    query.fLeft = rand.nextU() % GENERATE_EXTENTS;
                    query.fTop = rand.nextU() % GENERATE_EXTENTS;
                    query.fRight = query.fLeft + (GENERATE_EXTENTS / 20);
                    query.fBottom = query.fTop + (GENERATE_EXTENTS / 20);
                    break;
                case kLarge_QueryType:
                    query.fLeft = rand.nextU() % GENERATE_EXTENTS;
                    query.fTop = rand.nextU() % GENERATE_EXTENTS;
                    query.fRight = query.fLeft + (GENERATE_EXTENTS / 2);
                    query.fBottom = query.fTop + (GENERATE_EXTENTS / 2);
                    break;
                case kFull_QueryType:
                    query.fLeft = -GENERATE_EXTENTS;
                    query.fTop = -GENERATE_EXTENTS;
                    query.fRight = 2 * GENERATE_EXTENTS;
                    query.fBottom = 2 * GENERATE_EXTENTS;
                    break;
                default: // fallthrough
                case kRandom_QueryType:
                    query.fLeft = rand.nextU() % GENERATE_EXTENTS;
                    query.fTop = rand.nextU() % GENERATE_EXTENTS;
                    query.fRight = query.fLeft + 1 + rand.nextU() % (GENERATE_EXTENTS / 2);
                    query.fBottom = query.fTop + 1 + rand.nextU() % (GENERATE_EXTENTS / 2);
                    break;
            };
            fTree->search(query, &hits);
        }
    }
private:
    SkBBoxHierarchy* fTree;
    MakeRectProc fProc;
    SkString fName;
    bool fBulkLoad;
    QueryType fQuery;
    typedef Benchmark INHERITED;
};

static inline SkIRect make_concentric_rects_increasing(SkRandom&, int index, int numRects) {
    SkIRect out = {0, 0, index + 1, index + 1};
    return out;
}

static inline SkIRect make_XYordered_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft = index % GRID_WIDTH;
    out.fTop = index / GRID_WIDTH;
    out.fRight  = out.fLeft + 1 + rand.nextU() % (GENERATE_EXTENTS / 3);
    out.fBottom = out.fTop + 1 + rand.nextU() % (GENERATE_EXTENTS / 3);
    return out;
}
static inline SkIRect make_YXordered_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft = index / GRID_WIDTH;
    out.fTop = index % GRID_WIDTH;
    out.fRight  = out.fLeft + 1 + rand.nextU() % (GENERATE_EXTENTS / 3);
    out.fBottom = out.fTop + 1 + rand.nextU() % (GENERATE_EXTENTS / 3);
    return out;
}

static inline SkIRect make_random_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft   = rand.nextS() % GENERATE_EXTENTS;
    out.fTop    = rand.nextS() % GENERATE_EXTENTS;
    out.fRight  = out.fLeft + 1 + rand.nextU() % (GENERATE_EXTENTS / 5);
    out.fBottom = out.fTop  + 1 + rand.nextU() % (GENERATE_EXTENTS / 5);
    return out;
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("XYordered", &make_XYordered_rects, false,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("XYordered", &make_XYordered_rects, true,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("(unsorted)XYordered", &make_XYordered_rects, true,
                      SkRTree::Create(5, 16, 1, false)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("XYordered", &make_XYordered_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("(unsorted)XYordered", &make_XYordered_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16, 1, false)));
)

DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("YXordered", &make_YXordered_rects, false,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("YXordered", &make_YXordered_rects, true,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("(unsorted)YXordered", &make_YXordered_rects, true,
                      SkRTree::Create(5, 16, 1, false)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("YXordered", &make_YXordered_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("(unsorted)YXordered", &make_YXordered_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16, 1, false)));
)

DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("random", &make_random_rects, false,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("random", &make_random_rects, true,
                      SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("(unsorted)random", &make_random_rects, true,
                      SkRTree::Create(5, 16, 1, false)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("random", &make_random_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("(unsorted)random", &make_random_rects, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16, 1, false)));
)

DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("concentric",
                      &make_concentric_rects_increasing, true, SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeBuildBench, ("(unsorted)concentric",
                      &make_concentric_rects_increasing, true, SkRTree::Create(5, 16, 1, false)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("concentric", &make_concentric_rects_increasing, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
)
DEF_BENCH(
    return SkNEW_ARGS(RTreeQueryBench, ("(unsorted)concentric", &make_concentric_rects_increasing, true,
                      RTreeQueryBench::kRandom_QueryType, SkRTree::Create(5, 16, 1, false)));
)
