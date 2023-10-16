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
// This method of calculating the intersection only uses 10 multiplies, and 1 division. It also
// determines if the two segments cross with no round-off error and is always correct using 8
// multiplies. However, the actual crossing point is rounded to fit back into the int32_t.
std::optional<Point> intersect(const Segment& s0, const Segment& s1) {

    // Check if the bounds intersect.
    if (no_intersection_by_bounding_box(s0, s1)) {
        return std::nullopt;
    }

    // Create the Qa, Ra, and Sa vectors rooted at s0.p0, Qb, Rb, Sb rooted at s1.p0.
    Point Oa = s0.p0,
          Qa = s0.p1 - Oa,
          Ra = s1.p0 - Oa,
          Sa = s1.p1 - Oa,
          Ob = s1.p0,
          Qb = s1.p1 - Ob,
          Rb = s0.p0 - Ob,
          Sb = s0.p1 - Ob;

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        int64_t x0 = SkToS64(v0.x),
                y0 = SkToS64(v0.y),
                x1 = SkToS64(v1.x),
                y1 = SkToS64(v1.y);
        return x0 * y1 - y0 * x1;
    };

    // Calculate the four cross products. Calculate the cross product of two sets of two
    // triangles -- the 'a' set and the b set. The two for the 'a' set are described by the vectors
    // Qa x Ra and Qa x Sa, and like wise for the 'b' set. If either set of cross products have the
    // same signs, then there is no crossing.
    int64_t QaxRa = cross(Qa, Ra),
            QaxSa = cross(Qa, Sa),
            QbxRb = cross(Qb, Rb),
            QbxSb = cross(Qb, Sb);

    // If the endpoint is on Q, then there is no crossing. Only true intersections are returned.
    // For the intersection calculation, line segments do not include their end-points.
    if (QaxRa == 0 || QaxSa == 0 || QbxRb == 0 || QbxSb == 0) {
        return std::nullopt;
    }

    // The cross products have the same sign, so no intersection. There is no round-off error in
    // QaxRa, QaxSa, QbxRb or QbxSb. This ensures that there is really an intersection.
    if ((QaxRa ^ QaxSa) >= 0 || (QbxRb ^ QbxSb) >= 0) {
        return std::nullopt;
    }

    // TODO: this calculation probably needs to use 32-bit x 64-bit -> 96-bit multiply and
    // 96-bit / 64-bit -> 32-bit quotient and a 64-bit remainder. Fake it with doubles below.
    // N / D constitute a value on [0, 1], where the intersection I is
    //     I = s0.p0 + (s0.p1 - s0.p0) * N/D.
    double N = QaxRa,
           D = QaxRa - QaxSa,
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

int compareSlopes(const Segment& s0, const Segment& s1) {
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
    int64_t lhs = (int64_t)s0Delta.x * (int64_t)s1Delta.y,
            rhs = (int64_t)s1Delta.x * (int64_t)s0Delta.y;

    if (lhs < rhs) {
        return -1;
    } else if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}
}  // namespace bentleyottmann
