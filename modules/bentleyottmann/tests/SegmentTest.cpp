// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Segment.h"
#include "tests/Test.h"

using namespace bentleyottmann;

DEF_TEST(BO_SegmentBasic, reporter) {
    {
        Segment s = {{0, 0}, {1, 1}};
        REPORTER_ASSERT(reporter, s.upper() == s.p0);
        REPORTER_ASSERT(reporter, s.lower() == s.p1);
    }

    {
        Segment s = {{1, 0}, {0, 1}};
        REPORTER_ASSERT(reporter, s.upper() == s.p0);
        REPORTER_ASSERT(reporter, s.lower() == s.p1);
    }

    {
        Segment s = {{1, 1}, {0, 0}};
        REPORTER_ASSERT(reporter, s.upper() == s.p1);
        REPORTER_ASSERT(reporter, s.lower() == s.p0);
    }

    {
        Segment s = {{0, 1}, {1, 0}};
        REPORTER_ASSERT(reporter, s.upper() == s.p1);
        REPORTER_ASSERT(reporter, s.lower() == s.p0);
    }
}

static Segment swap_ends(const Segment& s) {
    return {s.p1, s.p0};
}

DEF_TEST(BO_no_intersection_bounding_box, reporter) {
    Segment interesting[] = {{Point::Smallest(),  Point::Smallest()+ Point{10, 5}},
                             {Point::Largest(), Point::Largest() - Point{10, 5}},
                             {{-10, -5}, {10, 5}}};

    // Intersection
    for (auto& s0 : interesting) {
        auto [l, t, r, b] = s0.bounds();

        // Points in the interior of interesting rectangles
        for(Point p : {Point {l + 1, t + 1},
                       Point {r - 1, t + 1},
                       Point {r - 1, b - 1},
                       Point {l + 1, b - 1}}) {
            Segment s1 = {p, {0, 0}};
            REPORTER_ASSERT(reporter, !no_intersection_by_bounding_box(s0, s1));
            REPORTER_ASSERT(reporter, !no_intersection_by_bounding_box(s1, s0));
            REPORTER_ASSERT(reporter,
            !no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
            REPORTER_ASSERT(reporter,
            !no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
        }
    }

    int32_t small = Point::Smallest().x,
            big = Point::Largest().x;

    // No Intersection
    for (auto& s0 : interesting) {
        auto [l, t, r, b] = s0.bounds();

        Segment outside[] = {{{r, t}, {big, b}},
                             {{r, b}, {big, big}},
                             {{l, b}, {r, big}},
                             {{l, b}, {small, big}},
                             {{l, t}, {small, b}},
                             {{l, t}, {small, small}},
                             {{l, t}, {r, small}},
                             {{r, t}, {small, small}}};

        for (auto& s1 : outside) {
            REPORTER_ASSERT(reporter, no_intersection_by_bounding_box(s0, s1));
            REPORTER_ASSERT(reporter, no_intersection_by_bounding_box(s1, s0));
            REPORTER_ASSERT(reporter,
                    no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
            REPORTER_ASSERT(reporter,
                    no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
        }
    }
}
