// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Point.h"

#include <limits>
#include <tuple>

namespace bentleyottmann {

// -- Point ----------------------------------------------------------------------------------------
bool operator<(const Point& p0, const Point& p1) {
    return std::tie(p0.y, p0.x) < std::tie(p1.y, p1.x);
}

bool operator>(const Point& p0, const Point& p1) {
    return p1 < p0;
}

bool operator>=(const Point& p0, const Point& p1) {
    return !(p0 < p1);
}

bool operator<=(const Point& p0, const Point& p1) {
    return !(p0 > p1);
}

bool operator==(const Point& p0, const Point& p1) {
    return std::tie(p0.y, p0.x) == std::tie(p1.y, p1.x);
}

bool operator!=(const Point& p0, const Point& p1) {
    return !(p0 == p1);
}

Point Point::Smallest() {
    const int32_t kMinCoordinate = std::numeric_limits<int32_t>::min();
    return {kMinCoordinate, kMinCoordinate};
}

Point Point::Largest() {
    const int32_t kMaxCoordinate = std::numeric_limits<int32_t>::max();
    return {kMaxCoordinate, kMaxCoordinate};
}

bool Point::DifferenceTooBig(Point p0, Point p1) {
    auto tooBig = [](int32_t a, int32_t b) {
        return (b > 0 && a < std::numeric_limits<int32_t>::min() + b) ||
               (b < 0 && a > std::numeric_limits<int32_t>::max() + b);
    };

    return tooBig(p0.x, p1.x) || tooBig(p0.y, p1.y);
}
}  // namespace bentleyottmann
