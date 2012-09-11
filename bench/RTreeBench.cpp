
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkRTree.h"
#include "SkRandom.h"
#include "SkString.h"

// confine rectangles to a smallish area, so queries generally hit something, and overlap occurs:
static const int GENERATE_EXTENTS = 1000;
static const int NUM_BUILD_RECTS = 500;
static const int NUM_QUERY_RECTS = 5000;
static const int NUM_QUERIES = 1000;

typedef SkIRect (*MakeRectProc)(SkRandom&, int, int);

// Time how long it takes to build an R-Tree either bulk-loaded or not
class BBoxBuildBench : public SkBenchmark {
public:
    BBoxBuildBench(void* param, const char* name, MakeRectProc proc, bool bulkLoad,
                    SkBBoxHierarchy* tree)
        : INHERITED(param)
        , fTree(tree)
        , fProc(proc)
        , fBulkLoad(bulkLoad) {
        fName.append("rtree_");
        fName.append(name);
        fName.append("_build");
        if (fBulkLoad) {
            fName.append("_bulk");
        }
    }
    virtual ~BBoxBuildBench() {
        fTree->unref();
    }
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }
    virtual void onDraw(SkCanvas* canvas) {
        SkRandom rand;
        for (int i = 0; i < SkBENCHLOOP(100); ++i) {
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
    typedef SkBenchmark INHERITED;
};

// Time how long it takes to perform queries on an R-Tree, bulk-loaded or not
class BBoxQueryBench : public SkBenchmark {
public:
    enum QueryType {
        kSmall_QueryType, // small queries
        kLarge_QueryType, // large queries
        kRandom_QueryType,// randomly sized queries
        kFull_QueryType   // queries that cover everything
    };

    BBoxQueryBench(void* param, const char* name, MakeRectProc proc, bool bulkLoad,
                    QueryType q, SkBBoxHierarchy* tree)
        : INHERITED(param)
        , fTree(tree)
        , fProc(proc)
        , fBulkLoad(bulkLoad)
        , fQuery(q) {
        fName.append("rtree_");
        fName.append(name);
        fName.append("_query");
        if (fBulkLoad) {
            fName.append("_bulk");
        }
        SkRandom rand;
        for (int j = 0; j < SkBENCHLOOP(NUM_QUERY_RECTS); ++j) {
            fTree->insert(reinterpret_cast<void*>(j), fProc(rand, j,
                           SkBENCHLOOP(NUM_QUERY_RECTS)), fBulkLoad);
        }
        fTree->flushDeferredInserts();
    }
    virtual ~BBoxQueryBench() {
        fTree->unref();
    }
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }
    virtual void onDraw(SkCanvas* canvas) {
        SkRandom rand;
        for (int i = 0; i < SkBENCHLOOP(NUM_QUERIES); ++i) {
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
    typedef SkBenchmark INHERITED;
};

static SkIRect make_simple_rect(SkRandom&, int index, int numRects) {
    SkIRect out = {0, 0, GENERATE_EXTENTS, GENERATE_EXTENTS};
    return out;
}

static SkIRect make_concentric_rects_increasing(SkRandom&, int index, int numRects) {
    SkIRect out = {0, 0, index + 1, index + 1};
    return out;
}

static SkIRect make_concentric_rects_decreasing(SkRandom&, int index, int numRects) {
    SkIRect out = {0, 0, numRects - index, numRects - index};
    return out;
}

static SkIRect make_point_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft   = rand.nextU() % GENERATE_EXTENTS;
    out.fTop    = rand.nextU() % GENERATE_EXTENTS;
    out.fRight  = out.fLeft + (GENERATE_EXTENTS / 200);
    out.fBottom = out.fTop + (GENERATE_EXTENTS / 200);
    return out;
}

static SkIRect make_random_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft   = rand.nextS() % GENERATE_EXTENTS;
    out.fTop    = rand.nextS() % GENERATE_EXTENTS;
    out.fRight  = out.fLeft + 1 + rand.nextU() % (GENERATE_EXTENTS / 5);
    out.fBottom = out.fTop  + 1 + rand.nextU() % (GENERATE_EXTENTS / 5);
    return out;
}

static SkIRect make_large_rects(SkRandom& rand, int index, int numRects) {
    SkIRect out;
    out.fLeft   = rand.nextU() % GENERATE_EXTENTS;
    out.fTop    = rand.nextU() % GENERATE_EXTENTS;
    out.fRight  = out.fLeft + (GENERATE_EXTENTS / 3);
    out.fBottom = out.fTop  + (GENERATE_EXTENTS / 3);
    return out;
}

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) {
    return SkNEW_ARGS(BBoxBuildBench, (p, "random", &make_random_rects, true,
                      SkRTree::Create(5, 16)));
}
static SkBenchmark* Fact1(void* p) {
    return SkNEW_ARGS(BBoxBuildBench, (p, "random", &make_random_rects, false,
                      SkRTree::Create(5, 16)));
}
static SkBenchmark* Fact2(void* p) {
    return SkNEW_ARGS(BBoxBuildBench, (p, "concentric",
                      &make_concentric_rects_increasing, true, SkRTree::Create(5, 16)));
}
static SkBenchmark* Fact3(void* p) {
    return SkNEW_ARGS(BBoxQueryBench, (p, "random", &make_random_rects, true,
                      BBoxQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
}
static SkBenchmark* Fact4(void* p) {
    return SkNEW_ARGS(BBoxQueryBench, (p, "random", &make_random_rects, false,
                      BBoxQueryBench::kRandom_QueryType, SkRTree::Create(5, 16)));
}

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg4(Fact4);

