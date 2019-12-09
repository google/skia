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
    Base(size_t n, char i) : SkFlexibleArrays{n}, fI{i} {}
    ~Base() { total += fI; }

    // If you want to allow this in your public interface.
    using SkFlexibleArrays::arrays;

    static int total;
private:
    const char fI;
};
int Base::total = 0;

DEF_TEST(SkFlexibleArrays_Basic, reporter) {
    {
        auto base = Base::Make(5, (char) 3);
        SkZip<double, SkPoint> zip = base->arrays();
        REPORTER_ASSERT(reporter, zip.size() == 5);
        for (auto[i, aDouble, aSkPoint] : SkMakeEnumerate(zip)) {
            aDouble = i;
            aSkPoint = SkPoint::Make(i, i);
        }

        for (auto[i, aDouble, aSkPoint] :  SkMakeEnumerate(zip)) {
            REPORTER_ASSERT(reporter, aDouble == i);
            REPORTER_ASSERT(reporter, aSkPoint == SkPoint::Make(i, i));
        }

        {
            uintptr_t p = (uintptr_t) &zip.get<0>()[0];
            uintptr_t mask = alignof(double) - 1;
            REPORTER_ASSERT(reporter, (p & mask) == 0);
        }

        {
            uintptr_t p = (uintptr_t) &zip.get<1>()[0];
            uintptr_t mask = alignof(SkPoint) - 1;
            REPORTER_ASSERT(reporter, (p & mask) == 0);
        }
    }
}
