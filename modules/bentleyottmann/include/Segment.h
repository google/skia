// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Segment_DEFINED
#define Segment_DEFINED

#include "modules/bentleyottmann/include/Point.h"

#include <cstdint>
#include <optional>
#include <tuple>

namespace bentleyottmann {

struct Segment {
    Point p0;
    Point p1;

    // Y is larger going down the y-axis.
    // Get the higher point. It will be left most for horizontal segment.
    Point upper() const;

    // Get the lower point. It will be the right most for horizontal segment.
    Point lower() const;

    std::tuple<int32_t, int32_t, int32_t, int32_t> bounds() const;
};

bool operator==(const Segment& s0, const Segment& s1);
bool operator<(const Segment& s0, const Segment& s1);

struct Crossing {
    const Segment s0;
    const Segment s1;
    const Point crossing;
};

bool no_intersection_by_bounding_box(const Segment& s0, const Segment& s1);

// Finds the intersection of s0 and s1. Returns nullopt if there is no intersection.
// Note this intersection assumes that line segments do not include their end points.
std::optional<Point> intersect(const Segment& s0, const Segment& s1);

// Compare two segments at the sweep line given by y.
// It is an error to pass segments that don't intersect the horizontal line at y.
bool less_than_at(const Segment& s0, const Segment& s1, int32_t y);

// Given a horizontal line defined by p.y, is p.x < the x value where the horizontal line passes
// segment.
bool point_less_than_segment_in_x(Point p, const Segment& segment);

// The following two routines are used to find the rounded comparison between a Point p and a
// segment s. s(y) is the x value of the segment at y. The rounding definition is:
//    x < ⌊s(y) + ½⌋
// Expanding the floor operation results in
//    (x - ½) ≤ s(y) < (x + ½)
// The two functions are the two halves of the above inequality.
// The rounding lower bound is: (x - ½) ≤ s(y). The ordering of the parameters facilitates using
// std::lower_bound.
bool rounded_point_less_than_segment_in_x_lower(const Segment& s, Point p);
// The rounding upper bound is: s(y) < (x + ½)
bool rounded_point_less_than_segment_in_x_upper(const Segment& s, Point p);

// Compare the slopes of two segments. If a slope is horizontal, then its slope is greater than
// all other slopes or equal of the other segment is also horizontal. The slope for
// non-horizontal segments monotonically increases from the smallest along the negative x-axis
// increasing counterclockwise to the largest along the positive x-axis.
// Returns:
// * -1 - slope(s0) < slope(s1)
// *  0 - slope(s0) == slope(s1)
// *  1 - slope(s0) > slope(s1)
int compare_slopes(const Segment& s0, const Segment& s1);

}  // namespace bentleyottmann
#endif  // Segment_DEFINED
