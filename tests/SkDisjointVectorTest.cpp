/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDisjointVector.h"
#include "tests/Test.h"


DEF_TEST(SkDisjointVectorBasic, reporter) {
    SkDisjointVector<int> v;
    REPORTER_ASSERT(reporter, v.begin() == v.end());
    REPORTER_ASSERT(reporter, v.empty());
    REPORTER_ASSERT(reporter, v.size() == 0);
    v.emplace(4);
    REPORTER_ASSERT(reporter, !v.empty());
    REPORTER_ASSERT(reporter, v.size() == 1);
    REPORTER_ASSERT(reporter, v[0] == 4);
    v.emplace(5);
    REPORTER_ASSERT(reporter, v.size() == 2);
    REPORTER_ASSERT(reporter, v[0] == 4);
    REPORTER_ASSERT(reporter, v[1] == 5);
}
