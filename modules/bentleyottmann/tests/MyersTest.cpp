// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "tests/Test.h"

namespace myers {
extern bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1);
extern bool segment_less_than_upper_to_insert(const Segment& segment, const Segment& to_insert);
extern bool s0_less_than_s1_at_y(const Segment& s0, const Segment& s1, int32_t y);
extern bool s0_intersects_s1(const Segment& s0, const Segment& s1);
}  // namespace myers

using namespace myers;

static bool operator==(std::pair<const Point &, const Point &> l, std::tuple<Point, Point> r) {
    return std::get<0>(l) == std::get<0>(r) && std::get<1>(l) == std::get<1>(r);
}

DEF_TEST(MFC_order_segment_points, r) {
    {
        Point p0 = {0, 0},
              p1 = {1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {-1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {0, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
}

DEF_TEST(MFC_segment_ctor, r) {
    {
        Point p0 = {0, 0},
              p1 = {1, 1};
        Segment s = {p1, p0};
        const auto [u, l] = s;
        REPORTER_ASSERT(r, u == s.upper() && u == p0);
        REPORTER_ASSERT(r, l == s.lower() && l == p1);
    }

    {
        Point p0 = {0, 0},
              p1 = {0, 1};
        Segment s = {p1, p0};
        const auto [u, l] = s;
        REPORTER_ASSERT(r, u == s.upper() && u == p0);
        REPORTER_ASSERT(r, l == s.lower() && l == p1);
    }
}

DEF_TEST(MFC_slope_less_than, r) {
    {
        Segment s0 = {{0, 0}, {1, 1}},
                s1 = {{0, 0}, {-1, 1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, slope_s0_less_than_slope_s1(s1, s0));
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s0));
    }
    {
        Segment s = {{0, 0}, {0,1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s, s));
    }
    {  // Check slopes for horizontals.
        Segment s0 = {{-2, 0}, {1, 0}},
                s1 = {{-1, 0}, {2, 0}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s1, s0));
    }
    {  // Check slopes for horizontals.
        Segment s0 = {{-2, 0}, {1, 0}},
                s1 = {{0, 0}, {1, 1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, slope_s0_less_than_slope_s1(s1, s0));
    }
}

DEF_TEST(MFC_segment_less_than_upper_to_insert, r) {
    Segment s0 = {{-10, -10}, {10, 10}},
            s1 = {{10, -10}, {-10, 10}},
            to_insert = {{0, 0}, {0, 3}};

    // Above y = 0, the sweepLine is {s0, s1}, but at y=0 s1 and s0 swap because of their slopes.
    std::vector<Segment> sweepLine = {s1, s0};

    auto insertionPoint = std::lower_bound(sweepLine.begin(), sweepLine.end(), to_insert,
                                           segment_less_than_upper_to_insert);

    // The insertion point is between s1 and s0.
    REPORTER_ASSERT(r, *insertionPoint == s0);
    REPORTER_ASSERT(r, *(insertionPoint-1) == s1);
}

DEF_TEST(MFC_less_than_at_y, r) {
    {
        Segment s0 = {{0, 0}, {2, 2}},
                s1 = {{0, 0}, {-2, 2}};
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 1));
    }
    {  // cross at 0 use slope to break tie.
        Segment s0 = {{-2, -2}, {2, 2}},
                s1 = {{2, -2}, {-2, 2}};
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s0, s1, -1));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s1, s0, -1));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 0));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 0));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 1));
    }
    {
        Segment s0 = {{-2, -100}, {-2, 89}},
                s1 = {{6, -70}, {-2, 72}};

        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 72));
    }
}

static Segment swap_ends(const Segment& s) {
    return {s.lower(), s.upper()};
}

DEF_TEST(MFC_has_inner_intersection, r) {
    auto checkIntersection = [&](Segment s0, Segment s1) {
        REPORTER_ASSERT(r, s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, s0_intersects_s1(s1, s0));
        REPORTER_ASSERT(r, s0_intersects_s1(swap_ends(s0), swap_ends(s1)));
        REPORTER_ASSERT(r, s0_intersects_s1(swap_ends(s1), swap_ends(s0)));
    };

    {
        Segment s0 = {{-1, 0}, {1,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1);
    }
    {
        Segment s0 = {{-1, 0}, {5,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1);
    }

    {
        Segment s0 = {{5, 0}, {-1,  0}},
                s1 = {{ 0, -1}, {0, 1}};

        checkIntersection(s0, s1);
    }

    {
        Segment s0 = {{-5, -5}, {5, 5}},
                s1 = {{-5, 5}, {5, -5}};

        checkIntersection(s0, s1);
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
                    bool actual = s0_intersects_s1({P0, P1}, {P2, P3});
                    bool expected = (x0 <= x2 && x3 <= x1) || (x2 <= x0 && x1 <= x3);
                    if (actual != expected) {
                        s0_intersects_s1({P0, P1}, {P2, P3});
                        REPORTER_ASSERT(r, actual == expected);
                    }
                }
            }
        }
    }

    {
        Segment s0 = {{0, -100}, {0, -50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
    {
        Segment s0 = {{0, 100}, {0, 50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
    {
        Segment s0 = {{0, -101}, {0, -50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
}
