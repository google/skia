/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkPathOpsCubic.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

static const CubicPts hullTests[] = {
{{{2.6250000819563866, 2.3750000223517418}, {2.833333432674408, 2.3333333432674408}, {3.1111112236976624, 2.3333333134651184}, {3.4074075222015381, 2.3333332538604736}}},
};

static const size_t hullTests_count = SK_ARRAY_COUNT(hullTests);

DEF_TEST(PathOpsCubicHull, reporter) {
    for (size_t index = 0; index < hullTests_count; ++index) {
        const CubicPts& c = hullTests[index];
        SkDCubic cubic;
        cubic.debugSet(c.fPts);
        char order[4];
        cubic.convexHull(order);
    }
}
