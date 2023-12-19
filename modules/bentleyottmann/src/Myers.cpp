// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <tuple>

namespace myers {

// -- Point ----------------------------------------------------------------------------------------
Point operator-(const Point& p0, const Point& p1) {
    return {p0.x - p1.x, p0.y - p1.y};
}

std::tuple<int64_t, int64_t> point_to_s64(Point p) {
    return std::make_tuple(SkToS64(p.x), SkToS64(p.y));
}

// -- Segment --------------------------------------------------------------------------------------
Point Segment::upper() const {
    return fUpper;
}

Point Segment::lower() const {
    return fLower;
}

std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [left, right] = std::minmax(fUpper.x, fLower.x);
    return std::make_tuple(left, fUpper.y, right, fLower.y);
}

bool Segment::isHorizontal() const {
    return fUpper.y == fLower.y;
}

bool Segment::isVertical() const {
    return fUpper.x == fLower.x;
}

// Return:
//    | d0x   d1x |
//    | d0y   d1y |
int64_t cross(Point d0, Point d1) {
    const auto [d0x, d0y] = point_to_s64(d0);
    const auto [d1x, d1y] = point_to_s64(d1);
    return d0x * d1y - d1x * d0y;
}

// compare_slopes returns a comparison of the slope of s0 to the slope of s1 where
//    slope(s) = dx / dy
// instead of the regular dy / dx, and neither s0 nor s1 are horizontal.
//
// The slope for non-horizontal segments monotonically increases from the smallest along the
// negative x-axis increasing counterclockwise to the largest along the positive x-axis.
int64_t compare_slopes(const Segment& s0, const Segment& s1) {
    // Handle cases involving horizontal segments.
    if (s0.isHorizontal() || s1.isHorizontal()) {
        if (s0.isHorizontal() && s1.isHorizontal()) {
            // slope(s0) == slope(s1)
            return 0;
        }
        if (s0.isHorizontal()) {
            // slope(s0) > slope(s1)
            return 1;
        } else {
            // slope(s0) < slope(s1)
            return -1;
        }
    }

    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const Point d0 = l0 - u0;
    const Point d1 = l1 - u1;

    // Since horizontal lines are handled separately and because of the ordering of points for
    // a segment, then d0y and d1y should always be positive.
    SkASSERT(d0.y > 0 && d1.y > 0);

    //     * slope(s0) = d0.x / d0.y
    //     * slope(s1) = d1.x / d1.y
    // If we want to find d0.x / d0.y < d1.x / d1.y, then
    //    d0x * d1y < d1x * d0y
    //    d0x * d1y - d1x * d0y < 0.
    //
    // We know that d0.y and d1.y are both positive, therefore, we can do a cross multiply without
    // worrying about changing the relation.
    // We can define ==, and > in a similar way:
    //    * <  - cross_of_slopes(s0, s1) < 0
    //    * == - cross_of_slopes(s0, s1) == 0
    //    * >  - cross_of_slopes(s0, s1) > 0
    return cross(d0, d1);
}

// Returns true of slope(s0) < slope(s1). See compare_slopes above for more information.
bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1) {
    return compare_slopes(s0, s1) < 0;
}

// compare_point_to_segment the relation between a point p and a segment s in the following way:
//     * p < s if the cross product is negative.
//     * p == s if the cross product is zero.
//     * p > s if the cross product is positive.
int64_t compare_point_to_segment(Point p, const Segment& s) {
    const auto [u, l] = s;

    // The segment must span p vertically.
    SkASSERT(u.y <= p.y && p.y <= l.y);

    // Check horizontal extents.
    {
        const auto [left, right] = std::minmax(u.x, l.x);
        if (p.x < left) {
            return -1;
        }

        if (right < p.x) {
            return 1;
        }
    }

    // If s is horizontal, then p is on the interval [u.x, l.x].
    if (s.isHorizontal()) {
        return 0;
    }

    // The point p < s when:
    //     p.x < u.x + (l.x - u.x)(p.y - u.y) / (l.y - u.y),
    //     p.x - u.x < (l.x - u.x)(p.y - u.y) / (l.y - u.y),
    //     (p.x - u.x)(l.y - u.y) < (l.x - u.x)(p.y - u.y),
    //     (p.x - u.x)(l.y - u.y) - (l.x - u.x)(p.y - u.y) < 0,
    //     (p - u) x (l - u) < 0,
    //     dUtoP x dS < 0.
    // The other relations can be implemented in a similar way.
    const Point dUToP = p - u;
    const Point dS = l - u;

    SkASSERT(dS.y > 0);
    return cross(dUToP, dS);
}

// segment_less_than_upper_to_insert is used with std::lower_bound to find the place to insert the
// segment to_insert in a vector. The signature of this function is crafted to work with
// lower_bound.
bool segment_less_than_upper_to_insert(const Segment& segment, const Segment& to_insert) {
    const int64_t compare = compare_point_to_segment(to_insert.upper(), segment);

    // compare > 0 when segment < to_insert.upper().
    return (compare > 0) || ((compare == 0) && slope_s0_less_than_slope_s1(segment, to_insert));
}
}  // namespace myers
