/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkRandom.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "tests/Test.h"

namespace skgpu::graphite {

class SimpleIntersectionTree {
public:
    bool add(SkRect rect) {
        for (const SkRect& r : fRects) {
            if (r.intersects(rect)) {
                return false;
            }
        }
        fRects.push_back(rect);
        return true;
    }

private:
    std::vector<SkRect> fRects;
};

#define CHECK(A) REPORTER_ASSERT(reporter, A)

DEF_GRAPHITE_TEST(skgpu_IntersectionTree, reporter, CtsEnforcement::kNextRelease) {
    SkRandom rand;
    {
        SimpleIntersectionTree simpleTree;
        IntersectionTree tree;
        for (int i = 0; i < 1000; ++i) {
            Rect rect = Rect::XYWH(rand.nextRangeF(0, 500),
                                                       rand.nextRangeF(0, 500),
                                                       rand.nextRangeF(0, 70),
                                                       rand.nextRangeF(0, 70));
            CHECK(tree.add(rect) == simpleTree.add({rect.left(),
                                                   rect.top(),
                                                   rect.right(),
                                                   rect.bot()}));
        }
    }
    {
        SimpleIntersectionTree simpleTree;
        IntersectionTree tree;
        for (int i = 0; i < 100; ++i) {
            Rect rect = Rect::XYWH(rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 200),
                                   rand.nextRangeF(0, 200));
            CHECK(tree.add(rect) == simpleTree.add({rect.left(),
                                                   rect.top(),
                                                   rect.right(),
                                                   rect.bot()}));
        }
    }
    {
        SimpleIntersectionTree simpleTree;
        IntersectionTree tree;
        CHECK(tree.add(Rect::Infinite()));
        CHECK(!tree.add(Rect::WH(1,1)));
        CHECK(!tree.add(Rect::WH(1,std::numeric_limits<float>::infinity())));
        CHECK(tree.add(Rect::WH(0, 0)));
        CHECK(tree.add(Rect::WH(-1, 1)));
        CHECK(tree.add(Rect::WH(1, std::numeric_limits<float>::quiet_NaN())));
    }
}

}  // namespace skgpu::graphite
