// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Point.h"
#include "tests/Test.h"

using namespace bentleyottmann;

DEF_TEST(BO_PointBasic, reporter) {
    {
        Point p0 = {0, 0};
        Point p1 = {0, 0};
        REPORTER_ASSERT(reporter, !(p0 < p1));
        REPORTER_ASSERT(reporter, !(p1 < p0));
        REPORTER_ASSERT(reporter, p0 == p1);
    }

    {
        Point p0 = {0, 0};
        Point p1 = {1, 1};
        REPORTER_ASSERT(reporter, p0 < p1);
        REPORTER_ASSERT(reporter, !(p1 < p0));
        REPORTER_ASSERT(reporter, p0 != p1);
    }

    {
        // Same y different x.
        Point p0 = {0, 0};
        Point p1 = {1, 0};
        REPORTER_ASSERT(reporter, p0 < p1);
        REPORTER_ASSERT(reporter, !(p1 < p0));
        REPORTER_ASSERT(reporter, p0 != p1);
    }

    {
        REPORTER_ASSERT(reporter, !Point::DifferenceTooBig(Point::Smallest(), Point{0, 0}));
        REPORTER_ASSERT(reporter, !Point::DifferenceTooBig(Point::Largest(), Point{0, 0}));
        REPORTER_ASSERT(reporter, Point::DifferenceTooBig(Point{0, 0}, Point::Smallest()));
        REPORTER_ASSERT(reporter, !Point::DifferenceTooBig(Point{0, 0}, Point::Largest()));
        REPORTER_ASSERT(reporter, Point::DifferenceTooBig(Point::Smallest(), Point::Largest()));
        REPORTER_ASSERT(reporter, Point::DifferenceTooBig(Point::Largest(), Point::Smallest()));
        REPORTER_ASSERT(reporter,
                        !Point::DifferenceTooBig(Point::Smallest() + Point{1, 1}, Point{0, 0}));
        REPORTER_ASSERT(reporter,
                        !Point::DifferenceTooBig(Point{0, 0}, Point::Smallest() + Point{1, 1}));
    }
}
