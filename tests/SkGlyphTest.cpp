/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyph.h"
#include "tests/Test.h"

DEF_TEST(SkGlyphRectBasic, reporter) {
    using namespace skglyph;
    SkGlyphRect r{1, 1, 10, 10};
    REPORTER_ASSERT(reporter, !r.empty());
    SkGlyphRect a = rect_union(r, empty_rect());
    REPORTER_ASSERT(reporter, a.iRect() == SkIRect::MakeLTRB(1, 1, 10, 10));

    a = rect_intersection(r, full_rect());
    REPORTER_ASSERT(reporter, a.iRect() == SkIRect::MakeLTRB(1, 1, 10, 10));

    SkGlyphRect acc = full_rect();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect_intersection(acc, SkGlyphRect(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.iRect() == SkIRect::MakeLTRB(9, 9, 10, 10));

    acc = empty_rect();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect_union(acc, SkGlyphRect(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.iRect() == SkIRect::MakeLTRB(-10, -10, 29, 29));
}
