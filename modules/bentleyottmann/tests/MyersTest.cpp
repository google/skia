// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "tests/Test.h"

namespace myers {
extern bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1);
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
