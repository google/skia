// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef QuadraticCrossings_DEFINED
#define QuadraticCrossings_DEFINED

#include "include/core/SkSpan.h"
#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <vector>

namespace bentleyottmann {
// Takes in a list of segments, and returns intersection points found in the list of segments.
// An empty vector means there are no self intersections.
//
std::optional<std::vector<Point>> brute_force_crossings(SkSpan<const Segment> segments);
}  // namespace bentleyottmann

#endif  // QuadraticCrossings_DEFINED
