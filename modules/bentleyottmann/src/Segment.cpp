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
//    (x2 - x0 + t(x3 - x2)) / (x1 - x0) = (y2 - y1 + t(y3 - y2)) / (y1 - y0)
//    (y1 - y0)(x2 - x0 + t(x3 - x2)) = (x1 - x0)(y2 - y1 + t(y3 - y2))
//    (y1 - y0)(x2 - x0) + t(y1 - y0)(x3 - x2) = (x1 - x0)(y2 - y1) + t(x1 - x0)(y3 - y2)
// Collecting t's on one side, and constants on the other.
//    t((y1 - y0)(x3 - x2) - (x1 - x0)(y3 - y2)) = (x1 - x0)(y2 - y1) - (y1 - y0)(x2 - x0)
// Assign names and vectors to extract the cross products. The vector (x0, y0) -> (x1, y1) is
// P0 -> P1, and is named Q = (x1 - x0, y1 - y0) = P1 - P0. The following vectors are defined in
// a similar way.
//  * Q: P1 - P0
//  * R: P2 - P0
//  * S: P3 - P0
//  * T: P3 - P2
// Extracting cross products from above.
//    t((P3 - P2) x (P1 - P0)) = (P1 - P0) x (P2 - P0)
//    t(T x Q) = Q x R
//    t = (Q x R) / (T x Q)
// But, T x Q is not something we want to calculate. What must be calculated is Q x R and Q x S.
// If these two cross products have different signs, then there is an intersection, otherwise
// there is not. We can calculate T x Q in terms of Q x R and Q x S in the following way.
//    T x Q = Q x R - Q x S
//          = (P1 - P0) x (P2 - P0) - (P1 - P0) x (P3 - P0)
//          = (P1 - P0) x ((P2 - P0) - (P3 - P0))
//          = (P1 - P0) x (P2 - P3)
//          = Q x -T
//          = -(Q x T)
//          = T x Q.
// So, t is equal to
//    t = (Q x R) / ((Q x R) - (Q x S)).
// This is then substituted into I = (x2 + t(x3 - x2), y2 + t(y3 - y2)).
//
// This method of calculating the intersection only uses 6 multiplies, and 1 division. It also
// determines if the two segments cross with no round-off error and is always correct using 4
// multiplies. However, the actual crossing point is rounded to fit back into the int32_t.
std::optional<Point> intersect(const Segment& s0, const Segment& s1) {

    // Check if the bounds intersect.
    if (no_intersection_by_bounding_box(s0, s1)) {
        return std::nullopt;
    }

    // Create the Q, R, and S vectors rooted at s0.p0.
    Point O = s0.p0,
          Q = s0.p1 - O,
          R = s1.p0 - O,
          S = s1.p1 - O;

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        int64_t x0 = SkToS64(v0.x),
                y0 = SkToS64(v0.y),
                x1 = SkToS64(v1.x),
                y1 = SkToS64(v1.y);
        return x0 * y1 - y0 * x1;
    };

    // Calculate the two cross products.
    int64_t QxR = cross(Q, R),
            QxS = cross(Q, S);

    // If the endpoint is on Q, then there is no crossing. Only true intersections are returned.
    // For the intersection calculation, line segments do not include their end-points.
    if (QxR == 0 || QxS == 0) {
        return std::nullopt;
    }

    // The cross products have the same sign, so no intersection. There is no round-off error in
    // QXR or QXS. This ensures that there is really an intersection.
    if ((QxR ^ QxS) >= 0) {
        return std::nullopt;
    }

    // TODO: this calculation probably needs to use 32-bit x 64-bit -> 96-bit multiply and
    // 96-bit / 64-bit -> 32-bit quotient and a 64-bit remainder. Fake it with doubles below.
    // N / D constitute a value on [0, 1], where the intersection I is
    //     I = s0.p0 + (s0.p1 - s0.p0) * N/D.
    double N = QxR,
           D = QxR - QxS,
           t = N / D;

    SkASSERT(0 <= t && t <= 1);

    // Calculate the intersection using doubles.
    // TODO: This is just a placeholder approximation for calculating x and y should use big math
    // above.
    int32_t x = std::round(t * (s1.p1.x - s1.p0.x) + s1.p0.x),
            y = std::round(t * (s1.p1.y - s1.p0.y) + s1.p0.y);

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
bool lessThanAt(const Segment& s0, const Segment& s1, int32_t y) {
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
}  // namespace bentleyottmann
