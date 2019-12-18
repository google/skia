/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDisjointVector.h"
#include "src/core/SkEnumerate.h"
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
    REPORTER_ASSERT(reporter, v.begin() != v.end());
    v.emplace(5);
    REPORTER_ASSERT(reporter, v.size() == 2);
    REPORTER_ASSERT(reporter, v[0] == 4);
    REPORTER_ASSERT(reporter, v[1] == 5);
    REPORTER_ASSERT(reporter, v.begin() != v.end());
    v.emplace(6);
    REPORTER_ASSERT(reporter, v.size() == 3);
    REPORTER_ASSERT(reporter, v[0] == 4);
    REPORTER_ASSERT(reporter, v[1] == 5);
    REPORTER_ASSERT(reporter, v[2] == 6);

    for (size_t i = 0; i < v.size(); i++) {
        auto t = v[i];
        REPORTER_ASSERT(reporter, i + 4 == t);
    }

    int index = 0;
    for (int i : v) {
        REPORTER_ASSERT(reporter, index + 4 == i);
        index++;
    }

}
