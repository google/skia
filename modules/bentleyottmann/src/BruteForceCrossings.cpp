// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BruteForceCrossings.h"

#include "modules/bentleyottmann/include/Segment.h"

#include <optional>
#include <vector>

namespace bentleyottmann {
std::optional<std::vector<Crossing>> brute_force_crossings(SkSpan<const Segment> segments) {
    std::vector<Crossing> answer;
    if (segments.size() >= 2) {
        for (auto i0 = segments.begin(); i0 != segments.end() - 1; ++i0) {
            for (auto i1 = i0 + 1; i1 != segments.end(); ++i1) {
                if (auto possiblePoint = intersect(*i0, *i1)) {
                    answer.push_back({*i0, *i1, possiblePoint.value()});
                }
            }
        }
    }
    return answer;
}
}  // namespace bentleyottmann
