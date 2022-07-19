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
    SkGlyphRect16 r{1, 1, 10, 10};
    REPORTER_ASSERT(reporter, !r.empty());
    SkGlyphRect16 a = rect16_union(r, empty_rect16());
    REPORTER_ASSERT(reporter, a.iRect() == SkIRect::MakeLTRB(1, 1, 10, 10));
    auto widthHeight = a.widthHeight();
    REPORTER_ASSERT(reporter, widthHeight[0] == 9 && widthHeight[1] == 9);

    a = rect16_intersection(r, full_rect16());
    REPORTER_ASSERT(reporter, a.iRect() == SkIRect::MakeLTRB(1, 1, 10, 10));

    SkGlyphRect16 acc = full_rect16();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect16_intersection(acc, SkGlyphRect16(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.iRect() == SkIRect::MakeLTRB(9, 9, 10, 10));

    acc = empty_rect16();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect16_union(acc, SkGlyphRect16(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.iRect() == SkIRect::MakeLTRB(-10, -10, 29, 29));
}
