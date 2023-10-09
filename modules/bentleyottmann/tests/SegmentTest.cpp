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

DEF_TEST(BO_intersectBasic, reporter) {

    auto checkIntersection = [reporter](Segment s0, Segment s1, Point expected) {
        {
            auto answer = intersect(s0, s1);
            REPORTER_ASSERT(reporter, answer.has_value());
            REPORTER_ASSERT(reporter, answer.value() == expected);
        }
        {
            auto answer = intersect(s1, s0);
            REPORTER_ASSERT(reporter, answer.has_value());
            REPORTER_ASSERT(reporter, answer.value() == expected);
        }
        {
            auto answer = intersect(swap_ends(s0), swap_ends(s1));
            REPORTER_ASSERT(reporter, answer.has_value());
            REPORTER_ASSERT(reporter, answer.value() == expected);
        }
        {
            auto answer = intersect(swap_ends(s1), swap_ends(s0));
            REPORTER_ASSERT(reporter, answer.has_value());
            REPORTER_ASSERT(reporter, answer.value() == expected);
        }
    };

    {
        Segment s0 = {{-1, 0}, {1,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1, Point{0, 0});
    }
    {
        Segment s0 = {{-1, 0}, {5,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1, Point{0, 0});
    }

    {
        Segment s0 = {{5, 0}, {-1,  0}},
                s1 = {{ 0, -1}, {0, 1}};

        checkIntersection(s0, s1, Point{0, 0});
    }

    {
        Segment s0 = {{-5, -5}, {5, 5}},
                s1 = {{-5, 5}, {5, -5}};

        checkIntersection(s0, s1, Point{0, 0});
    }

    // Test very close segments (x0, 0) -> (x1, 1) & (x2, 0) -> (x3, 1)
    for (int32_t x0 = -10; x0 <= 10; x0++) {
        for (int32_t x1 = -10; x1 <= 10; x1++) {
            for (int32_t x2 = -10; x2 <= 10; x2++) {
                for (int32_t x3 = -10; x3 <= 10; x3++) {
                    Point P0 = {x0, 0},
                          P1 = {x1, 1},
                          P2 = {x2, 0},
                          P3 = {x3, 1};
                    auto actual = intersect({P0, P1}, {P2, P3});
                    bool expected = (x0 < x2 && x3 < x1) || (x2 < x0 && x1 < x3);
                    REPORTER_ASSERT(reporter, actual.has_value() == expected);
                    if (actual) {
                        int32_t y = std::abs(x2 - x0) >= std::abs(x3 - x1);
                        REPORTER_ASSERT(reporter, actual.value().y == y);
                    }
                }
            }
        }
    }
}

DEF_TEST(BO_lessAtBasic, reporter) {
    { // Parallel lines
        Segment s0 = {{-1, 1}, {-1, -1}},
                s1 = {{1, 1}, {1, -1}};

        REPORTER_ASSERT(reporter, lessThanAt(s0, s1, -1));
        REPORTER_ASSERT(reporter, !lessThanAt(s1, s0, -1));
        REPORTER_ASSERT(reporter, lessThanAt(s0, s1, 0));
        REPORTER_ASSERT(reporter, !lessThanAt(s1, s0, 0));
        REPORTER_ASSERT(reporter, lessThanAt(s0, s1, 1));
        REPORTER_ASSERT(reporter, !lessThanAt(s1, s0, 1));
    }
    { // Crossed lines
        Segment s0 = {{-1, -1}, {1, 1}},
                s1 = {{1, -1}, {-1, 1}};

        REPORTER_ASSERT(reporter, lessThanAt(s0, s1, -1));
        REPORTER_ASSERT(reporter, !lessThanAt(s1, s0, -1));

        // When they are == neither is less.
        REPORTER_ASSERT(reporter, !lessThanAt(s0, s1, 0));
        REPORTER_ASSERT(reporter, !lessThanAt(s1, s0, 0));

        REPORTER_ASSERT(reporter, !lessThanAt(s0, s1, 1));
        REPORTER_ASSERT(reporter, lessThanAt(s1, s0, 1));
    }
    { // Near crossing
        Segment s0 = {{0, -100}, {0, 100}},
                s1 = {{-3, 98}, {3, 104}};

        REPORTER_ASSERT(reporter, !lessThanAt(s0, s1, 98));
        REPORTER_ASSERT(reporter, lessThanAt(s1, s0, 98));

        REPORTER_ASSERT(reporter, !lessThanAt(s0, s1, 99));
        REPORTER_ASSERT(reporter, lessThanAt(s1, s0, 99));

        REPORTER_ASSERT(reporter, !lessThanAt(s0, s1, 100));
        REPORTER_ASSERT(reporter, lessThanAt(s1, s0, 100));
    }
}

DEF_TEST(BO_compareSlopesBasic, reporter) {
    { // Both horizontal
        Segment s0 = {{-1, 1}, {0, 1}},
                s1 = {{-2, 1}, {1, 1}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == 0);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == 0);
    }
    { // One horizontal
        Segment s0 = {{-1, 1}, {0, 0}},
                s1 = {{-2, 1}, {1, 1}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == -1);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == 1);
    }
    { // One vertical
        Segment s0 = {{-1, 1}, {-1, 0}}, // Vertical
                s1 = {{-2, 1}, {-1, 0}},
                s2 = {{2, 1}, {-1, 0}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == 1);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == -1);
        REPORTER_ASSERT(reporter, compareSlopes(s0, s2) == -1);
        REPORTER_ASSERT(reporter, compareSlopes(s2, s0) == 1);
    }

    { // Equal slope
        Segment s0 = {{-2, 1}, {0, 0}},
                s1 = {{-4, 2}, {0, 0}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == 0);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == 0);
    }

    { // Equal slope
        Segment s0 = {{2, 1}, {0, 0}},
                s1 = {{4, 2}, {0, 0}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == 0);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == 0);
    }

    {
        Segment s0 = {{-2, 1}, {0, 0}},
                s1 = {{4, 2}, {0, 0}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == -1);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == 1);
    }

    {
        Segment s0 = {{-2, 1}, {0, 0}},
                s1 = {{-3, 1}, {0, 0}};
        REPORTER_ASSERT(reporter, compareSlopes(s0, s1) == 1);
        REPORTER_ASSERT(reporter, compareSlopes(s1, s0) == -1);
    }
}
