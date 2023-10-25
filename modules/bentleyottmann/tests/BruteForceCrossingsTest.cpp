// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BruteForceCrossings.h"

#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"
#include "tests/Test.h"

#include <vector>

using namespace bentleyottmann;

DEF_TEST(BO_bruteForceCrossingsBasic, reporter) {

    {
        Segment s0 = {{-1, 0}, {1,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        std::vector<Segment> segments;
        segments.push_back(s0);
        segments.push_back(s1);

        auto possibleCrossings = brute_force_crossings(segments);

        REPORTER_ASSERT(reporter, possibleCrossings.has_value());

        if (possibleCrossings) {
            auto crossings = possibleCrossings.value();
            REPORTER_ASSERT(reporter, crossings.size() == 1);
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, crossings[0].crossing == p);
            REPORTER_ASSERT(reporter, (crossings[0].s0 == s0 && crossings[0].s1 == s1) ||
                                      (crossings[0].s0 == s1 && crossings[0].s1 == s0));
        }
    }
    {
        Point p0 = {-50, -100},
              p1 = { 50, -100},
              p2 = {  0, 100};
        Segment s0 = {p0, p1},
                s1 = {p1, p2},
                s2 = {p2, p0};

        std::vector<Segment> segments{s0, s1, s2};
        auto possibleCrossings = brute_force_crossings(segments);

        REPORTER_ASSERT(reporter, possibleCrossings.has_value());
        if (possibleCrossings) {
            auto crossings = possibleCrossings.value();
            REPORTER_ASSERT(reporter, crossings.size() == 0);
        }
    }
    {
        Point p0 = {-50, 100},
              p1 = { 50, 100},
              p2 = {  0, -100};
        Segment s0 = {p0, p1},
                s1 = {p1, p2},
                s2 = {p2, p0};

        std::vector<Segment> segments{s0, s1, s2};
        auto possibleCrossings = brute_force_crossings(segments);

        REPORTER_ASSERT(reporter, possibleCrossings.has_value());
        if (possibleCrossings) {
            auto crossings = possibleCrossings.value();
            REPORTER_ASSERT(reporter, crossings.size() == 0);
        }
    }

}
