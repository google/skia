/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFlexibleArrays.h"


#include "include/core/SkPoint.h"
#include "tests/Test.h"

class Base : public SkFlexibleArrays<Base, double, SkPoint> {
public:
    Base(size_t n, int i) : SkFlexibleArrays{n}, fI{i} {}
    ~Base() { total += fI; }

    static int total;
private:
    const int fI;
};
int Base::total = 0;

DEF_TEST(SkFlexibleArrays_Basic, reporter) {
    auto base = Base::Make(5, 3);
    auto zip = base->arrays();
    for (auto [i, aDouble, aSkPoint] : SkMakeEnumerate(zip)) {
        aDouble = i;
        aSkPoint = SkPoint::Make(i, i);
    }

    for (auto [i, aDouble, aSkPoint] :  SkMakeEnumerate(zip)) {
        REPORTER_ASSERT(reporter, aDouble == i);
        REPORTER_ASSERT(reporter, aSkPoint == SkPoint::Make(i, i));
    }

}
