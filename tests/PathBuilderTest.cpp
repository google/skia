/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "tests/Test.h"

static void is_empty(skiatest::Reporter* reporter, const SkPath& p) {
    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, p.countPoints() == 0);
}

DEF_TEST(pathbuilder, reporter) {
    SkPathBuilder b;

    is_empty(reporter, b.snapshot());
    is_empty(reporter, b.detach());

    b.moveTo(10, 10).lineTo(20, 20).quadTo(30, 10, 10, 20);

    SkPath p0 = b.snapshot();
    SkPath p1 = b.snapshot();
    SkPath p2 = b.detach();

    REPORTER_ASSERT(reporter, p0.getBounds() == SkRect::MakeLTRB(10, 10, 30, 20));
    REPORTER_ASSERT(reporter, p0.countPoints() == 4);

    REPORTER_ASSERT(reporter, p0 == p1);
    REPORTER_ASSERT(reporter, p0 == p2);

    is_empty(reporter, b.snapshot());
    is_empty(reporter, b.detach());
}
