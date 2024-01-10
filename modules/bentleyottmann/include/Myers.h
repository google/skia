// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Myers_DEFINED
#define Myers_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

namespace myers {

// -- Point ----------------------------------------------------------------------------------------
struct Point {
    int32_t x = 0;
    int32_t y = 0;
};

// Return p0 and p1 in the correct order for a Segment.
constexpr bool operator<(const Point& p0, const Point& p1) {
    return std::tie(p0.y, p0.x) < std::tie(p1.y, p1.x);
}

constexpr bool operator==(const Point& p0, const Point& p1) {
    return std::tie(p0.x, p0.y) == std::tie(p1.x, p1.y);
}

constexpr bool operator!=(const Point& p0, const Point& p1) {
    return std::tie(p0.x, p0.y) != std::tie(p1.x, p1.y);
}

// -- Segment --------------------------------------------------------------------------------------
// A Segment is an undirected edge where the points have an order u.y < l.y else
// if (u.y == l.y) u.x < x.y.
class Segment {
public:
    constexpr Segment(Point p0, Point p1)
        : Segment{std::minmax(p0, p1)} {}

    const Point& upper() const;
    const Point& lower() const;
    std::tuple<int32_t, int32_t, int32_t, int32_t> bounds() const;
    bool isHorizontal() const;
    bool isVertical() const;
    friend constexpr bool operator<(const Segment& s0, const Segment& s1);
    friend constexpr bool operator==(const Segment& s0, const Segment& s1);
    friend constexpr bool operator!=(const Segment& s0, const Segment& s1);

private:
    constexpr Segment(const std::tuple<Point, Point>& ps)
            : fUpper{std::get<0>(ps)}
            , fLower{std::get<1>(ps)} {
        SkASSERT(fUpper != fLower);
        SkASSERT(fUpper < fLower);
    }

    Point fUpper;
    Point fLower;
};

constexpr bool operator<(const Segment& s0, const Segment& s1) {
    return std::tie(s0.fUpper, s0.fLower) < std::tie(s1.fUpper, s1.fLower);
}

constexpr bool operator==(const Segment& s0, const Segment& s1) {
    return std::tie(s0.fUpper, s0.fLower) == std::tie(s1.fUpper, s1.fLower);
}

constexpr bool operator!=(const Segment& s0, const Segment& s1) {
    return std::tie(s0.fUpper, s0.fLower) != std::tie(s1.fUpper, s1.fLower);
}

// Support for Segment as a tuple.
template<size_t> const myers::Point& get(const myers::Segment&);
template<> inline const myers::Point& get<0>(const myers::Segment& s) { return s.upper(); }
template<> inline const myers::Point& get<1>(const myers::Segment& s) { return s.lower(); }

// -- Crossing -------------------------------------------------------------------------------------
class Crossing {
public:
    Crossing(const Segment& s0, const Segment& s1) : Crossing{std::minmax(s0, s1)} {}
    friend bool operator<(const Crossing& c0, const Crossing& c1);
    friend bool operator==(const Crossing& c0, const Crossing& c1);

private:
    Crossing(std::tuple<Segment, Segment> highLow)
            : fHigher{std::get<0>(highLow)}
            , fLower{std::get<1>(highLow)} {}

    Segment fHigher;
    Segment fLower;
};

inline bool operator<(const Crossing& c0, const Crossing& c1) {
    return std::tie(c0.fHigher, c0.fLower) < std::tie(c1.fHigher, c1.fLower);
}

inline bool operator==(const Crossing& c0, const Crossing& c1) {
    return std::tie(c0.fHigher, c0.fLower) == std::tie(c1.fHigher, c1.fLower);
}

std::vector<Crossing> myers_find_crossings(const SkSpan<const Segment> segments);
std::vector<Crossing> brute_force_crossings(SkSpan<Segment>);
}  // namespace myers

// Support for Segment as a tuple. Must be in top-level namespace.
template<> struct std::tuple_size<myers::Segment> {
    static constexpr int value = 2;
};

template<size_t Index> struct std::tuple_element<Index, myers::Segment> {
    using type = const myers::Point&;
};
#endif  // Myers_DEFINED
