/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkQuadTree.h"
#include "SkRandom.h"
#include "SkString.h"

// confine rectangles to a smallish area, so queries generally hit something, and overlap occurs:
static const int GENERATE_EXTENTS = 1000;
static const int NUM_BUILD_RECTS = 500;
static const int NUM_QUERY_RECTS = 5000;
static const int GRID_WIDTH = 100;
static const SkIRect QUAD_TREE_BOUNDS = SkIRect::MakeLTRB(
    -GENERATE_EXTENTS, -GENERATE_EXTENTS, 2 * GENERATE_EXTENTS, 2 * GENERATE_EXTENTS);

typedef SkIRect (*MakeRectProc)(SkRandom&, int, int);

// Time how long it takes to build an QuadTree
class QuadTreeBuildBench : public Benchmark {
public:
    QuadTreeBuildBench(const char* name, MakeRectProc proc, SkBBoxHierarchy* tree)
        : fTree(tree)
        , fProc(proc) {
        fName.append("quadtree_");
        fName.append(name);
        fName.append("_build");
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual ~QuadTreeBuildBench() {
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
                              false);
            }
            fTree->clear();
        }
    }
private:
    SkBBoxHierarchy* fTree;
    MakeRectProc fProc;
    SkString fName;
    typedef Benchmark INHERITED;
};

// Time how long it takes to perform queries on an QuadTree
class QuadTreeQueryBench : public Benchmark {
public:
    enum QueryType {
        kSmall_QueryType, // small queries
        kLarge_QueryType, // large queries
        kRandom_QueryType,// randomly sized queries
        kFull_QueryType   // queries that cover everything
    };

    QuadTreeQueryBench(const char* name, MakeRectProc proc,
                    QueryType q, SkBBoxHierarchy* tree)
        : fTree(tree)
        , fProc(proc)
        , fQuery(q) {
        fName.append("quadtree_");
        fName.append(name);
        fName.append("_query");
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual ~QuadTreeQueryBench() {
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
                          false);
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
    return SkNEW_ARGS(QuadTreeBuildBench, ("XYordered", &make_XYordered_rects,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeQueryBench, ("XYordered", &make_XYordered_rects,
                      QuadTreeQueryBench::kRandom_QueryType,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeBuildBench, ("YXordered", &make_YXordered_rects,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeQueryBench, ("YXordered", &make_YXordered_rects,
                      QuadTreeQueryBench::kRandom_QueryType,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeBuildBench, ("random", &make_random_rects,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeQueryBench, ("random", &make_random_rects,
                      QuadTreeQueryBench::kRandom_QueryType,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeBuildBench, ("concentric", &make_concentric_rects_increasing,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
DEF_BENCH(
    return SkNEW_ARGS(QuadTreeQueryBench, ("concentric", &make_concentric_rects_increasing,
                      QuadTreeQueryBench::kRandom_QueryType,
                      SkNEW_ARGS(SkQuadTree, (QUAD_TREE_BOUNDS))));
)
