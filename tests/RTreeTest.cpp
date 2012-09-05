
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRandom.h"
#include "SkRTree.h"
#include "SkTSort.h"

static const size_t MIN_CHILDREN = 6;
static const size_t MAX_CHILDREN = 11;

static const size_t NUM_RECTS = 200;
static const size_t NUM_ITERATIONS = 100;
static const size_t NUM_QUERIES = 50;

struct DataRect {
    SkIRect rect;
    void* data;
};

static SkIRect random_rect(SkRandom& rand) {
    SkIRect rect = {0,0,0,0};
    while (rect.isEmpty()) {
        rect.fLeft   = rand.nextS() % 1000;
        rect.fRight  = rand.nextS() % 1000;
        rect.fTop    = rand.nextS() % 1000;
        rect.fBottom = rand.nextS() % 1000;
        rect.sort();
    }
    return rect;
}

static void random_data_rects(SkRandom& rand, DataRect out[], int n) {
    for (int i = 0; i < n; ++i) {
        out[i].rect = random_rect(rand);
        out[i].data = reinterpret_cast<void*>(i);
    }
}

static bool verify_query(SkIRect query, DataRect rects[], 
                         SkTDArray<void*>& found) {
    SkTDArray<void*> expected;
    // manually intersect with every rectangle
    for (int i = 0; i < NUM_RECTS; ++i) {
        if (SkIRect::IntersectsNoEmptyCheck(query, rects[i].rect)) {
            expected.push(rects[i].data);
        }
    }

    if (expected.count() != found.count()) { 
        return false; 
    } 
    
    if (0 == expected.count()) {
        return true;
    }

    // Just cast to long since sorting by the value of the void*'s was being problematic...
    SkTQSort(reinterpret_cast<long*>(expected.begin()), 
             reinterpret_cast<long*>(expected.end() - 1));
    SkTQSort(reinterpret_cast<long*>(found.begin()), 
             reinterpret_cast<long*>(found.end() - 1));
    return found == expected;
}

static void runQueries(skiatest::Reporter* reporter, SkRandom& rand, DataRect rects[], 
                       SkRTree& tree) {
    for (int i = 0; i < NUM_QUERIES; ++i) {
        SkTDArray<void*> hits;
        SkIRect query = random_rect(rand);
        tree.search(query, &hits);
        REPORTER_ASSERT(reporter, verify_query(query, rects, hits));
    }
}

static void TestRTree(skiatest::Reporter* reporter) {
    DataRect rects[NUM_RECTS];
    SkRandom rand;
    SkRTree* rtree = SkRTree::Create(MIN_CHILDREN, MAX_CHILDREN);
    REPORTER_ASSERT(reporter, NULL != rtree);

    int expectedDepthMin = -1;
    int expectedDepthMax = -1;

    int tmp = NUM_RECTS;
    while (tmp > 0) {
        tmp -= static_cast<int>(pow(static_cast<double>(MAX_CHILDREN), 
                                static_cast<double>(expectedDepthMin + 1)));
        ++expectedDepthMin;
    }

    tmp = NUM_RECTS;
    while (tmp > 0) {
        tmp -= static_cast<int>(pow(static_cast<double>(MIN_CHILDREN),
                                static_cast<double>(expectedDepthMax + 1)));
        ++expectedDepthMax;
    }

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        random_data_rects(rand, rects, NUM_RECTS);

        // First try bulk-loaded inserts
        for (int i = 0; i < NUM_RECTS; ++i) {
            rtree->insert(rects[i].data, rects[i].rect, true);
        }
        rtree->flushDeferredInserts();
        runQueries(reporter, rand, rects, *rtree);
        REPORTER_ASSERT(reporter, NUM_RECTS == rtree->getCount());
        REPORTER_ASSERT(reporter, expectedDepthMin <= rtree->getDepth() &&
                                  expectedDepthMax >= rtree->getDepth());
        rtree->clear();
        REPORTER_ASSERT(reporter, 0 == rtree->getCount());

        // Then try immediate inserts
        for (int i = 0; i < NUM_RECTS; ++i) {
            rtree->insert(rects[i].data, rects[i].rect);
        }
        runQueries(reporter, rand, rects, *rtree);
        REPORTER_ASSERT(reporter, NUM_RECTS == rtree->getCount());
        REPORTER_ASSERT(reporter, expectedDepthMin <= rtree->getDepth() &&
                                  expectedDepthMax >= rtree->getDepth());
        rtree->clear();
        REPORTER_ASSERT(reporter, 0 == rtree->getCount());

        // And for good measure try immediate inserts, but in reversed order
        for (int i = NUM_RECTS - 1; i >= 0; --i) {
            rtree->insert(rects[i].data, rects[i].rect);
        }
        runQueries(reporter, rand, rects, *rtree);
        REPORTER_ASSERT(reporter, NUM_RECTS == rtree->getCount());
        REPORTER_ASSERT(reporter, expectedDepthMin <= rtree->getDepth() &&
                                  expectedDepthMax >= rtree->getDepth());
        rtree->clear();
        REPORTER_ASSERT(reporter, 0 == rtree->getCount());
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("RTree", RTreeTestClass, TestRTree)
