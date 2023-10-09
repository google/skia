// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Segment_DEFINED
#define Segment_DEFINED

#include "modules/bentleyottmann/include/Point.h"

#include <optional>
#include <tuple>

namespace bentleyottmann {

struct Segment {
    const Point p0;
    const Point p1;

    // Y is larger going down the y-axis.
    // Get the higher point. It will be left most for horizontal segment.
    Point upper() const;

    // Get the lower point. It will be the right most for horizontal segment.
    Point lower() const;

    std::tuple<int32_t, int32_t, int32_t, int32_t> bounds() const;
};

bool no_intersection_by_bounding_box(const Segment& s0, const Segment& s1);

// Finds the intersection of s0 and s1. Returns nullopt if there is no intersection.
// Note this intersection assumes that line segments do not include their end points.
std::optional<Point> intersect(const Segment& s0, const Segment& s1);

// Compare two segments at the sweep line given by y.
// It is an error to pass segments that don't intersect the horizontal line at y.
bool lessThanAt(const Segment& s0, const Segment& s1, int32_t y);

// Compare the slopes of two segments. If a slope is horizontal, then its slope is greater than
// all other slopes or equal of the other segment is also horizontal. The slope for
// non-horizontal segments monotonically increases from the smallest along the negative x-axis
// increasing counterclockwise to the largest along the positive x-axis.
// Returns:
// * -1 - slope(s0) < slope(s1)
// *  0 - slope(s0) == slope(s1)
// *  1 - slope(s0) > slope(s1)
int compareSlopes(const Segment& s0, const Segment& s1);

}  // namespace bentleyottmann
#endif  // Segment_DEFINED
