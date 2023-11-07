// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Segment.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "modules/bentleyottmann/include/Int96.h"

#include <algorithm>
#include <cmath>

namespace bentleyottmann {

// -- Segment --------------------------------------------------------------------------------------
Point Segment::upper() const {
    return std::min(p0, p1);
}

Point Segment::lower() const {
    return std::max(p0, p1);
}

// Use auto [l, t, r, b] = s.bounds();
std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [l, r] = std::minmax(p0.x, p1.x);
    auto [t, b] = std::minmax(p0.y, p1.y);
    return std::make_tuple(l, t, r, b);
}

bool operator==(const Segment& s0, const Segment& s1) {
    return s0.upper() == s1.upper() && s0.lower() == s1.lower();
}

bool operator<(const Segment& s0, const Segment& s1) {
    return std::make_tuple(s0.upper(), s0.lower()) < std::make_tuple(s1.upper(), s1.lower());
}

bool no_intersection_by_bounding_box(const Segment& s0, const Segment& s1) {
    auto [left0, top0, right0, bottom0] = s0.bounds();
    auto [left1, top1, right1, bottom1] = s1.bounds();
    // If the sides of the box touch, then there is no new intersection.
    return right0 <= left1 || right1 <= left0 || bottom0 <= top1 || bottom1 <= top0;
}

// Derivation of Intersection
// The intersection point I = (X, Y) of the two segments (x0, y0) -> (x1, y1)
// and (x2, y2) -> (x3, y3).
//    X = x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    Y = y0 + s(y1 - y0) = y2 + t(y3 - y2)
//
// Solve for s in terms of x.
//    x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    s(x1 - x0) = x2 - x0 + t(x3 - x2)
//    s = (x2 - x0 + t(x3 - x2)) / (x1 - x0)
//
// Back substitute s into the equation for Y.
//    y0 + ((x2 - x0 + t(x3 - x2)) / (x1 - x0))(y1 - y0) = y2 + t(y3 - y2)
//    (x2 - x0 + t(x3 - x2)) / (x1 - x0) = (y2 - y0 + t(y3 - y2)) / (y1 - y0)
//    (y1 - y0)(x2 - x0 + t(x3 - x2)) = (x1 - x0)(y2 - y0 + t(y3 - y2))
//    (y1 - y0)(x2 - x0) + t(y1 - y0)(x3 - x2) = (x1 - x0)(y2 - y0) + t(x1 - x0)(y3 - y2)
// Collecting t's on one side, and constants on the other.
//    t((y1 - y0)(x3 - x2) - (x1 - x0)(y3 - y2)) = (x1 - x0)(y2 - y0) - (y1 - y0)(x2 - x0)
//
// Solve for t in terms of x.
//    x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    x0 - x2 + s(x1 - x0) = t(x3 - x2)
//    (x0 - x2 + s(x1 - x0)) / (x3 - x2) = t
// Back substitute t into the equation for Y.
//    y0 + s(y1 - y0) = y2 + ((x0 - x2 + s(x1 - x0)) / (x3 - x2))(y3 - y2)
//    (y0 - y2 + s(y1 - y0)) / (y3 - y2) = (x0 - x2 + s(x1 - x0)) / (x3 - x2)
//    (x3 - x2)(y0 - y2 + s(y1 - y0)) = (y3 - y2)(x0 - x2 + s(x1 - x0))
//    (x3 - x2)(y0 - y2) + s(x3 - x2)(y1 - y0) = (y3 - y2)(x0 - x2) + s(y3 - y2)(x1 - x0)
// Collecting s's on on side and constants on the other.
//    s((x3 - x2)(y1 - y0) - (y3 - y2)(x1 - x0)) = (y3 - y2)(x0 - x2) - (x3 - x2)(y0 - y2)

// Assign names and vectors to extract the cross products. The vector (x0, y0) -> (x1, y1) is
// P0 -> P1, and is named Q = (x1 - x0, y1 - y0) = P1 - P0. The following vectors are defined in
// a similar way.
//  * Q: P1 - P0
//  * R: P2 - P0
//  * T: P3 - P2
// Extracting cross products from above for t.
//    t((P3 - P2) x (P1 - P0)) = (P1 - P0) x (P2 - P0)
//    t(T x Q) = Q x R
//    t = (Q x R) / (T x Q)
// Extracting cross products from above for t.
//    s((P3 - P2) x (P1 - P0)) = (P0 - P2) x (P3 - P2)
//    s(T x Q) = -R x T
//    s = (T x R) / (T x Q)
//
// There is an intersection only if t and s are on [0, 1].
//
// This method of calculating the intersection only uses 8 multiplies, and 1 division. It also
// determines if the two segments cross with no round-off error and is always correct using 6
// multiplies. However, the actual crossing point is rounded to fit back into the int32_t.
std::optional<Point> intersect(const Segment& s0, const Segment& s1) {

    // Check if the bounds intersect.
    if (no_intersection_by_bounding_box(s0, s1)) {
        return std::nullopt;
    }

    // Create the end Points for s0 and s1
    const Point P0 = s0.upper(),
                P1 = s0.lower(),
                P2 = s1.upper(),
                P3 = s1.lower();

    if (P0 == P2 || P1 == P3 || P1 == P2 || P3 == P0) {
        // Lines don't intersect if they share an end point.
        return std::nullopt;
    }

    // Create the Q, R, and T.
    const Point Q = P1 - P0,
                R = P2 - P0,
                T = P3 - P2;

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        int64_t x0 = SkToS64(v0.x),
                y0 = SkToS64(v0.y),
                x1 = SkToS64(v1.x),
                y1 = SkToS64(v1.y);
        return x0 * y1 - y0 * x1;
    };

    // Calculate the cross products needed for calculating s and t.
    const int64_t QxR = cross(Q, R),
                  TxR = cross(T, R),
                  TxQ = cross(T, Q);

    if (TxQ == 0) {
        // Both t and s are either < 0 or > 1 because the denominator is 0.
        return std::nullopt;
    }

    // t = (Q x R) / (T x Q). s = (T x R) / (T x Q). Check that t & s are on [0, 1]
    if ((QxR ^ TxQ) < 0 || (TxR ^ TxQ) < 0) {
        // The division is negative and t or s < 0.
        return std::nullopt;
    }

    if (TxQ > 0) {
        if (QxR > TxQ || TxR > TxQ) {
            // t or s is greater than 1.
            return std::nullopt;
        }
    } else {
        if (QxR < TxQ || TxR < TxQ) {
            // t or s is greater than 1.
            return std::nullopt;
        }
    }

    // Calculate the intersection using doubles.
    // TODO: This is just a placeholder approximation for calculating x and y should use big math
    // above.
    const double t = static_cast<double>(QxR) / static_cast<double>(TxQ);
    SkASSERT(0 <= t && t <= 1);
    const int32_t x = std::round(t * (P3.x - P2.x) + P2.x),
                  y = std::round(t * (P3.y - P2.y) + P2.y);

    return Point{x, y};
}

// The comparison is:
//     x0 + (y - y0)(x1 - x0) / (y1 - y0) <? x2 + (y - y2)(x3 - x2) / (y3 - y2)
// Factor out numerators:
//    [x0(y1 - y0) + (y - y0)(x1 - x0)] / (y1 - y0) <? [x2(y3 - y2) + (y - y2)(x3 -x 2)] / (y3 - y2)
// Removing the divides by cross multiplying.
//   [x0(y1 - y0) + (y - y0)(x1 - x0)] (y3 - y2) <? [x2(y3 - y2) + (y - y2)(x3 - x2)] (y1 - y0)
// This is a 64-bit int x0 + (y - y0) (x1 - x0) times a 32-int (y3 - y2) resulting in a 96-bit int,
// and the same applies to the other side of the <?. Because y0 <= y1 and y2 <= y3, then the
// differences of (y1 - y0) and (y3 - y2) are positive allowing us to multiply through without
// worrying about sign changes.
bool less_than_at(const Segment& s0, const Segment& s1, int32_t y) {
    auto [l0, t0, r0, b0] = s0.bounds();
    auto [l1, t1, r1, b1] = s1.bounds();
    SkASSERT(t0 <= y && y <= b0);
    SkASSERT(t1 <= y && y <= b1);

    // Return true if the bounding box of s0 is fully to the left of s1.
    if (r0 < l1) {
        return true;
    }

    // Return false if the bounding box of s0 is fully to the right of s1.
    if (r1 < l0) {
        return false;
    }

    // Check the x intercepts along the horizontal line at y.
    // Make s0 be (x0, y0) -> (x1, y1) and s1 be (x2, y2) -> (x3, y3).
    auto [x0, y0] = s0.upper();
    auto [x1, y1] = s0.lower();
    auto [x2, y2] = s1.upper();
    auto [x3, y3] = s1.lower();

    int64_t s0YDiff = y - y0,
            s1YDiff = y - y2,
            s0YDelta = y1 - y0,
            s1YDelta = y3 - y2,
            x0Offset = x0 * s0YDelta + s0YDiff * (x1 - x0),
            x2Offset = x2 * s1YDelta + s1YDiff * (x3 - x2);

    Int96 s0Factor = multiply(x0Offset, y3 - y2),
          s1Factor = multiply(x2Offset, y1 - y0);

    return s0Factor < s1Factor;
}

bool point_less_than_segment_in_x(Point p, const Segment& segment) {
    auto [l, t, r, b] = segment.bounds();

    // Ensure that the segment intersects the horizontal sweep line
    SkASSERT(t <= p.y && p.y <= b);

    // Fast answers using bounding boxes.
    if (p.x < l) {
        return true;
    } else if (p.x >= r) {
        return false;
    }

    auto [x0, y0] = segment.upper();
    auto [x1, y1] = segment.lower();
    auto [x2, y2] = p;

    // For a point and a segment the comparison is:
    //    x2 < x0 + (y2 - y0)(x1 - x0) / (y1 - y0)
    // becomes
    //    (x2 - x0)(y1 - y0) < (x1 - x0)(y2 - y0)
    // We don't need to worry about the signs changing in the cross multiply because (y1 - y0) is
    // always positive. Manipulating a little further derives predicate 2 from "Robust Plane
    // Sweep for Intersecting Segments" page 9.
    //    0 < (x1 - x0)(y2 - y0) - (x2 - x0)(y1 - y0)
    // becomes
    //        | x1-x0   x2-x0 |
    //   0 <  | y1-y0   y2-y0 |
    return SkToS64(x2 - x0) * SkToS64(y1 - y0) < SkToS64(y2 - y0) * SkToS64(x1 - x0);
}

// The design of this function allows its use with std::lower_bound. lower_bound returns the
// iterator to the first segment where rounded_point_less_than_segment_in_x_lower returns false.
// Therefore, we want s(y) < (x - ½) to return true, then start returning false when s(y) ≥ (x - ½).
bool rounded_point_less_than_segment_in_x_lower(const Segment& s, Point p) {
    const auto [l, t, r, b] = s.bounds();
    const auto [x, y] = p;

    // Ensure that the segment intersects the horizontal sweep line
    SkASSERT(t <= y && y <= b);

    // In the comparisons below, x is really x - ½
    if (r < x) {
        // s is entirely < p.
        return true;
    } else if (x <= l) {
        // s is entirely > p. This also handles vertical lines, so we don't have to handle them
        // below.
        return false;
    }

    const auto [x0, y0] = s.upper();
    const auto [x1, y1] = s.lower();

    // Horizontal - from the guards above we know that p is on s.
    if (y0 == y1) {
        return false;
    }

    // s is not horizontal or vertical.
    SkASSERT(x0 != x1 && y0 != y1);

    // Given the segment upper = (x0, y0) and lower = (x1, y1)
    // x0 + (x1 - x0)(y - y0) / (y1 - y0) < x - ½
    // (x1 - x0)(y - y0) / (y1 - y0) < x - x0 - ½
    // Because (y1 - y0) is always positive we can multiply through the inequality without
    // worrying about sign changes.
    // (x1 - x0)(y - y0) < (x - x0 - ½)(y1 - y0)
    // (x1 - x0)(y - y0) < ½(2x - 2x0 - 1)(y1 - y0)
    // 2(x1 - x0)(y - y0) < (2(x - x0) - 1)(y1 - y0)
    return 2 * SkToS64(x1 - x0) * SkToS64(y - y0) < (2 * SkToS64(x - x0) - 1) * SkToS64(y1 - y0);
}

// The design of this function allows use with std::lower_bound. lower_bound returns the iterator
// to the first segment where rounded_point_less_than_segment_in_x_upper is false. This function
// implements s(y) < (x + ½).
bool rounded_point_less_than_segment_in_x_upper(const Segment& s, Point p) {
    const auto [l, t, r, b] = s.bounds();
    const auto [x, y] = p;

    // Ensure that the segment intersects the horizontal sweep line
    SkASSERT(t <= y && y <= b);

    // In the comparisons below, x is really x + ½
    if (r <= x) {
        // s is entirely < p.
        return true;
    } else if (x < l) {
        // s is entirely > p. This also handles vertical lines, so we don't have to handle them
        // below.
        return false;
    }

    const auto [x0, y0] = s.upper();
    const auto [x1, y1] = s.lower();

    // Horizontal - from the guards above we know that p is on s.
    if (y0 == y1) {
        return false;
    }

    // s is not horizontal or vertical.
    SkASSERT(x0 != x1 && y0 != y1);

    // Given the segment upper = (x0, y0) and lower = (x1, y1)
    // x0 + (x1 - x0)(y - y0) / (y1 - y0) < x + ½
    // (x1 - x0)(y - y0) / (y1 - y0) < x - x0 + ½
    // Because (y1 - y0) is always positive we can multiply through the inequality without
    // worrying about sign changes.
    // (x1 - x0)(y - y0) < (x - x0 + ½)(y1 - y0)
    // (x1 - x0)(y - y0) < ½(2x - 2x0 + 1)(y1 - y0)
    // 2(x1 - x0)(y - y0) < (2(x - x0) + 1)(y1 - y0)
    return 2 * SkToS64(x1 - x0) * SkToS64(y - y0) < (2 * SkToS64(x - x0) + 1) * SkToS64(y1 - y0);
}

int compare_slopes(const Segment& s0, const Segment& s1) {
    Point s0Delta = s0.lower() - s0.upper(),
          s1Delta = s1.lower() - s1.upper();

    // Handle the horizontal cases to avoid dealing with infinities.
    if (s0Delta.y == 0 || s1Delta.y == 0) {
        if (s0Delta.y != 0) {
            return -1;
        } else if (s1Delta.y != 0) {
            return 1;
        } else {
            return 0;
        }
    }

    // Compare s0Delta.x / s0Delta.y ? s1Delta.x / s1Delta.y. I used the alternate slope form for
    // two reasons.
    // * no change of sign - since the delta ys are always positive, then I don't need to worry
    //                       about the change in sign with the cross-multiply.
    // * proper slope ordering - the slope monotonically increases from the smallest along the
    //                           negative x-axis increasing counterclockwise to the largest along
    //                           the positive x-axis.
    int64_t lhs = SkToS64(s0Delta.x) * SkToS64(s1Delta.y),
            rhs = SkToS64(s1Delta.x) * SkToS64(s0Delta.y);

    if (lhs < rhs) {
        return -1;
    } else if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}
}  // namespace bentleyottmann
