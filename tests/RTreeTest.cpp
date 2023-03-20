/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/core/SkRTree.h"
#include "tests/Test.h"

#include <cmath>
#include <cstddef>
#include <vector>

using namespace skia_private;

static const int NUM_RECTS = 200;
static const size_t NUM_ITERATIONS = 100;
static const size_t NUM_QUERIES = 50;

static SkRect random_rect(SkRandom& rand) {
    SkRect rect = {0,0,0,0};
    while (rect.isEmpty()) {
        rect.fLeft   = rand.nextRangeF(0, 1000);
        rect.fRight  = rand.nextRangeF(0, 1000);
        rect.fTop    = rand.nextRangeF(0, 1000);
        rect.fBottom = rand.nextRangeF(0, 1000);
        rect.sort();
    }
    return rect;
}

static bool verify_query(SkRect query, SkRect rects[], const std::vector<int>& found) {
    std::vector<int> expected;
    // manually intersect with every rectangle
    for (int i = 0; i < NUM_RECTS; ++i) {
        if (SkRect::Intersects(query, rects[i])) {
            expected.push_back(i);
        }
    }

    if (expected.size() != found.size()) {
        return false;
    }
    if (0 == expected.size()) {
        return true;
    }
    return found == expected;
}

static void run_queries(skiatest::Reporter* reporter, SkRandom& rand, SkRect rects[],
                        const SkRTree& tree) {
    for (size_t i = 0; i < NUM_QUERIES; ++i) {
        std::vector<int> hits;
        SkRect query = random_rect(rand);
        tree.search(query, &hits);
        REPORTER_ASSERT(reporter, verify_query(query, rects, hits));
    }
}

DEF_TEST(RTree, reporter) {
    int expectedDepthMin = -1;
    int tmp = NUM_RECTS;
    while (tmp > 0) {
        tmp -= static_cast<int>(pow(static_cast<double>(SkRTree::kMaxChildren),
                                    static_cast<double>(expectedDepthMin + 1)));
        ++expectedDepthMin;
    }

    int expectedDepthMax = -1;
    tmp = NUM_RECTS;
    while (tmp > 0) {
        tmp -= static_cast<int>(pow(static_cast<double>(SkRTree::kMinChildren),
                                    static_cast<double>(expectedDepthMax + 1)));
        ++expectedDepthMax;
    }

    SkRandom rand;
    AutoTMalloc<SkRect> rects(NUM_RECTS);
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        SkRTree rtree;
        REPORTER_ASSERT(reporter, 0 == rtree.getCount());

        for (int j = 0; j < NUM_RECTS; j++) {
            rects[j] = random_rect(rand);
        }

        rtree.insert(rects.get(), NUM_RECTS);
        SkASSERT(rects);  // SkRTree doesn't take ownership of rects.

        run_queries(reporter, rand, rects, rtree);
        REPORTER_ASSERT(reporter, NUM_RECTS == rtree.getCount());
        REPORTER_ASSERT(reporter, expectedDepthMin <= rtree.getDepth() &&
                                  expectedDepthMax >= rtree.getDepth());
    }
}
