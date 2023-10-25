// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef BentleyOttman1_DEFINED
#define BentleyOttman1_DEFINED

#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

namespace bentleyottmann {
struct Crossing;
struct Segment;

// Takes in a list of segments, and returns intersection points found in the list of segments.
// A return value of nullopt means that the data are out of range. An empty vector means there
// are no self intersections.
//
// If nullopt is returned, you could divide all your points by 2, and try again.
std::optional<std::vector<Crossing>> bentley_ottmann_1(SkSpan<const Segment> segments);
}  // namespace bentleyottmann

#endif  // BentleyOttman1_DEFINED
