// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BruteForceCrossings.h"
#include "tests/Test.h"

#include <vector>

using namespace bentleyottmann;

DEF_TEST(BO_bruteForceCrossingsBasic, reporter) {

    Segment s0 = {{-1, 0}, {1,  0}},
            s1 = {{ 0, 1}, {0, -1}};

    std::vector<Segment> segments;
    segments.push_back(s0);
    segments.push_back(s1);

    auto possiblePoints = brute_force_crossings(segments);

    REPORTER_ASSERT(reporter, possiblePoints.has_value());

    if (possiblePoints) {
        auto points = possiblePoints.value();
        REPORTER_ASSERT(reporter, points.size() == 1);
        Point p = {0, 0};
        REPORTER_ASSERT(reporter, points[0] == p);
    }

}
