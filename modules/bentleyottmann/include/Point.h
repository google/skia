// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Point_DEFINED
#define Point_DEFINED

#include <cstdint>

namespace bentleyottmann {
struct Point {
    int32_t x;
    int32_t y;

    // Relation for ordering events.
    friend bool operator<(const Point& p0, const Point& p1);
    friend bool operator>(const Point& p0, const Point& p1);
    friend bool operator>=(const Point& p0, const Point& p1);
    friend bool operator<=(const Point& p0, const Point& p1);

    // Equality
    friend bool operator==(const Point& p0, const Point& p1);
    friend bool operator!=(const Point& p0, const Point& p1);

    // Extremes
    static Point Smallest();
    static Point Largest();
    static bool DifferenceTooBig(Point p0, Point p1);

    // Terms
    friend Point operator+(const Point& p0, const Point& p1) {
        return {p0.x + p1.x, p0.y + p1.y};
    }
    friend Point operator-(const Point& p0, const Point& p1) {
        return {p0.x - p1.x, p0.y - p1.y};
    }
};
}  // namespace bentleyottmann
#endif  // Point_DEFINED
