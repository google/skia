/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCGeometry.h"

#include "GrTypes.h"
#include "GrPathUtils.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// We convert between SkPoint and Sk2f freely throughout this file.
GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
GR_STATIC_ASSERT(2 * sizeof(float) == sizeof(SkPoint));
GR_STATIC_ASSERT(0 == offsetof(SkPoint, fX));

void GrCCGeometry::beginPath() {
    SkASSERT(!fBuildingContour);
    fVerbs.push_back(Verb::kBeginPath);
}

void GrCCGeometry::beginContour(const SkPoint& devPt) {
    SkASSERT(!fBuildingContour);

    fCurrFanPoint = fCurrAnchorPoint = devPt;

    // Store the current verb count in the fTriangles field for now. When we close the contour we
    // will use this value to calculate the actual number of triangles in its fan.
    fCurrContourTallies = {fVerbs.count(), 0, 0, 0};

    fPoints.push_back(devPt);
    fVerbs.push_back(Verb::kBeginContour);

    SkDEBUGCODE(fBuildingContour = true);
}

void GrCCGeometry::lineTo(const SkPoint& devPt) {
    SkASSERT(fBuildingContour);
    SkASSERT(fCurrFanPoint == fPoints.back());
    fCurrFanPoint = devPt;
    fPoints.push_back(devPt);
    fVerbs.push_back(Verb::kLineTo);
}

static inline Sk2f normalize(const Sk2f& n) {
    Sk2f nn = n*n;
    return n * (nn + SkNx_shuffle<1,0>(nn)).rsqrt();
}

static inline float dot(const Sk2f& a, const Sk2f& b) {
    float product[2];
    (a * b).store(product);
    return product[0] + product[1];
}

static inline bool are_collinear(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2) {
    static constexpr float kFlatnessTolerance = 4; // 1/4 of a pixel.

    // Area (times 2) of the triangle.
    Sk2f a = (p0 - p1) * SkNx_shuffle<1,0>(p1 - p2);
    a = (a - SkNx_shuffle<1,0>(a)).abs();

    // Bounding box of the triangle.
    Sk2f bbox0 = Sk2f::Min(Sk2f::Min(p0, p1), p2);
    Sk2f bbox1 = Sk2f::Max(Sk2f::Max(p0, p1), p2);

    // The triangle is linear if its area is within a fraction of the largest bounding box
    // dimension, or else if its area is within a fraction of a pixel.
    return (a * (kFlatnessTolerance/2) < Sk2f::Max(bbox1 - bbox0, 1)).anyTrue();
}

// Returns whether the (convex) curve segment is monotonic with respect to [endPt - startPt].
static inline bool is_convex_curve_monotonic(const Sk2f& startPt, const Sk2f& startTan,
                                             const Sk2f& endPt, const Sk2f& endTan) {
    Sk2f v = endPt - startPt;
    float dot0 = dot(startTan, v);
    float dot1 = dot(endTan, v);

    // A small, negative tolerance handles floating-point error in the case when one tangent
    // approaches 0 length, meaning the (convex) curve segment is effectively a flat line.
    float tolerance = -std::max(std::abs(dot0), std::abs(dot1)) * SK_ScalarNearlyZero;
    return dot0 >= tolerance && dot1 >= tolerance;
}

static inline Sk2f lerp(const Sk2f& a, const Sk2f& b, const Sk2f& t) {
    return SkNx_fma(t, b - a, a);
}

void GrCCGeometry::quadraticTo(const SkPoint& devP0, const SkPoint& devP1) {
    SkASSERT(fBuildingContour);
    SkASSERT(fCurrFanPoint == fPoints.back());

    Sk2f p0 = Sk2f::Load(&fCurrFanPoint);
    Sk2f p1 = Sk2f::Load(&devP0);
    Sk2f p2 = Sk2f::Load(&devP1);
    fCurrFanPoint = devP1;

    this->appendMonotonicQuadratics(p0, p1, p2);
}

inline void GrCCGeometry::appendMonotonicQuadratics(const Sk2f& p0, const Sk2f& p1,
                                                    const Sk2f& p2) {
    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    // This should almost always be this case for well-behaved curves in the real world.
    if (is_convex_curve_monotonic(p0, tan0, p2, tan1)) {
        this->appendSingleMonotonicQuadratic(p0, p1, p2);
        return;
    }

    // Chop the curve into two segments with equal curvature. To do this we find the T value whose
    // tangent is perpendicular to the vector that bisects tan0 and -tan1.
    Sk2f n = normalize(tan0) - normalize(tan1);

    // This tangent can be found where (dQ(t) dot n) = 0:
    //
    //   0 = (dQ(t) dot n) = | 2*t  1 | * | p0 - 2*p1 + p2 | * | n |
    //                                    | -2*p0 + 2*p1   |   | . |
    //
    //                     = | 2*t  1 | * | tan1 - tan0 | * | n |
    //                                    | 2*tan0      |   | . |
    //
    //                     = 2*t * ((tan1 - tan0) dot n) + (2*tan0 dot n)
    //
    //   t = (tan0 dot n) / ((tan0 - tan1) dot n)
    Sk2f dQ1n = (tan0 - tan1) * n;
    Sk2f dQ0n = tan0 * n;
    Sk2f t = (dQ0n + SkNx_shuffle<1,0>(dQ0n)) / (dQ1n + SkNx_shuffle<1,0>(dQ1n));
    t = Sk2f::Min(Sk2f::Max(t, 0), 1); // Clamp for FP error.

    Sk2f p01 = SkNx_fma(t, tan0, p0);
    Sk2f p12 = SkNx_fma(t, tan1, p1);
    Sk2f p012 = lerp(p01, p12, t);

    this->appendSingleMonotonicQuadratic(p0, p01, p012);
    this->appendSingleMonotonicQuadratic(p012, p12, p2);
}

inline void GrCCGeometry::appendSingleMonotonicQuadratic(const Sk2f& p0, const Sk2f& p1,
                                                         const Sk2f& p2) {
    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));

    // Don't send curves to the GPU if we know they are nearly flat (or just very small).
    if (are_collinear(p0, p1, p2)) {
        p2.store(&fPoints.push_back());
        fVerbs.push_back(Verb::kLineTo);
        return;
    }

    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    fVerbs.push_back(Verb::kMonotonicQuadraticTo);
    ++fCurrContourTallies.fQuadratics;
}

using ExcludedTerm = GrPathUtils::ExcludedTerm;

// Calculates the padding to apply around inflection points, in homogeneous parametric coordinates.
//
// More specifically, if the inflection point lies at C(t/s), then C((t +/- returnValue) / s) will
// be the two points on the curve at which a square box with radius "padRadius" will have a corner
// that touches the inflection point's tangent line.
//
// A serpentine cubic has two inflection points, so this method takes Sk2f and computes the padding
// for both in SIMD.
static inline Sk2f calc_inflect_homogeneous_padding(float padRadius, const Sk2f& t, const Sk2f& s,
                                                    const SkMatrix& CIT, ExcludedTerm skipTerm) {
    SkASSERT(padRadius >= 0);

    Sk2f Clx = s*s*s;
    Sk2f Cly = (ExcludedTerm::kLinearTerm == skipTerm) ? s*s*t*-3 : s*t*t*3;

    Sk2f Lx = CIT[0] * Clx + CIT[3] * Cly;
    Sk2f Ly = CIT[1] * Clx + CIT[4] * Cly;

    float ret[2];
    Sk2f bloat = padRadius * (Lx.abs() + Ly.abs());
    (bloat * s >= 0).thenElse(bloat, -bloat).store(ret);

    ret[0] = cbrtf(ret[0]);
    ret[1] = cbrtf(ret[1]);
    return Sk2f::Load(ret);
}

static inline void swap_if_greater(float& a, float& b) {
    if (a > b) {
        std::swap(a, b);
    }
}

// Calculates all parameter values for a loop at which points a square box with radius "padRadius"
// will have a corner that touches a tangent line from the intersection.
//
// T2 must contain the lesser parameter value of the loop intersection in its first component, and
// the greater in its second.
//
// roots[0] will be filled with 1 or 3 sorted parameter values, representing the padding points
// around the first tangent. roots[1] will be filled with the padding points for the second tangent.
static inline void calc_loop_intersect_padding_pts(float padRadius, const Sk2f& T2,
                                                  const SkMatrix& CIT, ExcludedTerm skipTerm,
                                                  SkSTArray<3, float, true> roots[2]) {
    SkASSERT(padRadius >= 0);
    SkASSERT(T2[0] <= T2[1]);
    SkASSERT(roots[0].empty());
    SkASSERT(roots[1].empty());

    Sk2f T1 = SkNx_shuffle<1,0>(T2);
    Sk2f Cl = (ExcludedTerm::kLinearTerm == skipTerm) ? T2*-2 - T1 : T2*T2 + T2*T1*2;
    Sk2f Lx = Cl * CIT[3] + CIT[0];
    Sk2f Ly = Cl * CIT[4] + CIT[1];

    Sk2f bloat = Sk2f(+.5f * padRadius, -.5f * padRadius) * (Lx.abs() + Ly.abs());
    Sk2f q = (1.f/3) * (T2 - T1);

    Sk2f qqq = q*q*q;
    Sk2f discr = qqq*bloat*2 + bloat*bloat;

    float numRoots[2], D[2];
    (discr < 0).thenElse(3, 1).store(numRoots);
    (T2 - q).store(D);

    // Values for calculating one root.
    float R[2], QQ[2];
    if ((discr >= 0).anyTrue()) {
        Sk2f r = qqq + bloat;
        Sk2f s = r.abs() + discr.sqrt();
        (r > 0).thenElse(-s, s).store(R);
        (q*q).store(QQ);
    }

    // Values for calculating three roots.
    float P[2], cosTheta3[2];
    if ((discr < 0).anyTrue()) {
        (q.abs() * -2).store(P);
        ((q >= 0).thenElse(1, -1) + bloat / qqq.abs()).store(cosTheta3);
    }

    for (int i = 0; i < 2; ++i) {
        if (1 == numRoots[i]) {
            float A = cbrtf(R[i]);
            float B = A != 0 ? QQ[i]/A : 0;
            roots[i].push_back(A + B + D[i]);
            continue;
        }

        static constexpr float k2PiOver3 = 2 * SK_ScalarPI / 3;
        float theta = std::acos(cosTheta3[i]) * (1.f/3);
        roots[i].push_back(P[i] * std::cos(theta) + D[i]);
        roots[i].push_back(P[i] * std::cos(theta + k2PiOver3) + D[i]);
        roots[i].push_back(P[i] * std::cos(theta - k2PiOver3) + D[i]);

        // Sort the three roots.
        swap_if_greater(roots[i][0], roots[i][1]);
        swap_if_greater(roots[i][1], roots[i][2]);
        swap_if_greater(roots[i][0], roots[i][1]);
    }
}

static inline Sk2f first_unless_nearly_zero(const Sk2f& a, const Sk2f& b) {
    Sk2f aa = a*a;
    aa += SkNx_shuffle<1,0>(aa);
    SkASSERT(aa[0] == aa[1]);

    Sk2f bb = b*b;
    bb += SkNx_shuffle<1,0>(bb);
    SkASSERT(bb[0] == bb[1]);

    return (aa > bb * SK_ScalarNearlyZero).thenElse(a, b);
}

static inline bool is_cubic_nearly_quadratic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                             const Sk2f& p3, Sk2f& tan0, Sk2f& tan3, Sk2f& c) {
    tan0 = first_unless_nearly_zero(p1 - p0, p2 - p0);
    tan3 = first_unless_nearly_zero(p3 - p2, p3 - p1);

    Sk2f c1 = SkNx_fma(Sk2f(1.5f), tan0, p0);
    Sk2f c2 = SkNx_fma(Sk2f(-1.5f), tan3, p3);
    c = (c1 + c2) * .5f; // Hopefully optimized out if not used?

    return ((c1 - c2).abs() <= 1).allTrue();
}

void GrCCGeometry::cubicTo(const SkPoint& devP1, const SkPoint& devP2, const SkPoint& devP3,
                           float inflectPad, float loopIntersectPad) {
    SkASSERT(fBuildingContour);
    SkASSERT(fCurrFanPoint == fPoints.back());

    SkPoint devPts[4] = {fCurrFanPoint, devP1, devP2, devP3};
    Sk2f p0 = Sk2f::Load(&fCurrFanPoint);
    Sk2f p1 = Sk2f::Load(&devP1);
    Sk2f p2 = Sk2f::Load(&devP2);
    Sk2f p3 = Sk2f::Load(&devP3);
    fCurrFanPoint = devP3;

    // Don't crunch on the curve and inflate geometry if it is nearly flat (or just very small).
    if (are_collinear(p0, p1, p2) &&
        are_collinear(p1, p2, p3) &&
        are_collinear(p0, (p1 + p2) * .5f, p3)) {
        p3.store(&fPoints.push_back());
        fVerbs.push_back(Verb::kLineTo);
        return;
    }

    // Also detect near-quadratics ahead of time.
    Sk2f tan0, tan3, c;
    if (is_cubic_nearly_quadratic(p0, p1, p2, p3, tan0, tan3, c)) {
        this->appendMonotonicQuadratics(p0, c, p3);
        return;
    }

    double tt[2], ss[2];
    fCurrCubicType = SkClassifyCubic(devPts, tt, ss);
    SkASSERT(!SkCubicIsDegenerate(fCurrCubicType)); // Should have been caught above.

    SkMatrix CIT;
    ExcludedTerm skipTerm = GrPathUtils::calcCubicInverseTransposePowerBasisMatrix(devPts, &CIT);
    SkASSERT(ExcludedTerm::kNonInvertible != skipTerm); // Should have been caught above.
    SkASSERT(0 == CIT[6]);
    SkASSERT(0 == CIT[7]);
    SkASSERT(1 == CIT[8]);

    // Each cubic has five different sections (not always inside t=[0..1]):
    //
    //   1. The section before the first inflection or loop intersection point, with padding.
    //   2. The section that passes through the first inflection/intersection (aka the K,L
    //      intersection point or T=tt[0]/ss[0]).
    //   3. The section between the two inflections/intersections, with padding.
    //   4. The section that passes through the second inflection/intersection (aka the K,M
    //      intersection point or T=tt[1]/ss[1]).
    //   5. The section after the second inflection/intersection, with padding.
    //
    // Sections 1,3,5 can be rendered directly using the CCPR cubic shader.
    //
    // Sections 2 & 4 must be approximated. For loop intersections we render them with
    // quadratic(s), and when passing through an inflection point we use a plain old flat line.
    //
    // We find T0..T3 below to be the dividing points between these five sections.
    float T0, T1, T2, T3;
    if (SkCubicType::kLoop != fCurrCubicType) {
        Sk2f t = Sk2f(static_cast<float>(tt[0]), static_cast<float>(tt[1]));
        Sk2f s = Sk2f(static_cast<float>(ss[0]), static_cast<float>(ss[1]));
        Sk2f pad = calc_inflect_homogeneous_padding(inflectPad, t, s, CIT, skipTerm);

        float T[2];
        ((t - pad) / s).store(T);
        T0 = T[0];
        T2 = T[1];

        ((t + pad) / s).store(T);
        T1 = T[0];
        T3 = T[1];
    } else {
        const float T[2] = {static_cast<float>(tt[0]/ss[0]), static_cast<float>(tt[1]/ss[1])};
        SkSTArray<3, float, true> roots[2];
        calc_loop_intersect_padding_pts(loopIntersectPad, Sk2f::Load(T), CIT, skipTerm, roots);
        T0 = roots[0].front();
        if (1 == roots[0].count() || 1 == roots[1].count()) {
            // The loop is tighter than our desired padding. Collapse the middle section to a point
            // somewhere in the middle-ish of the loop and Sections 2 & 4 will approximate the the
            // whole thing with quadratics.
            T1 = T2 = (T[0] + T[1]) * .5f;
        } else {
            T1 = roots[0][1];
            T2 = roots[1][1];
        }
        T3 = roots[1].back();
    }

    // Guarantee that T0..T3 are monotonic.
    if (T0 > T3) {
        // This is not a mathematically valid scenario. The only reason it would happen is if
        // padding is very small and we have encountered FP rounding error.
        T0 = T1 = T2 = T3 = (T0 + T3) / 2;
    } else if (T1 > T2) {
        // This just means padding before the middle section overlaps the padding after it. We
        // collapse the middle section to a single point that splits the difference between the
        // overlap in padding.
        T1 = T2 = (T1 + T2) / 2;
    }
    // Clamp T1 & T2 inside T0..T3. The only reason this would be necessary is if we have
    // encountered FP rounding error.
    T1 = std::max(T0, std::min(T1, T3));
    T2 = std::max(T0, std::min(T2, T3));

    // Next we chop the cubic up at all T0..T3 inside 0..1 and store the resulting segments.
    if (T1 >= 1) {
        // Only sections 1 & 2 can be in 0..1.
        this->chopCubic<&GrCCGeometry::appendMonotonicCubics,
                        &GrCCGeometry::appendCubicApproximation>(p0, p1, p2, p3, T0);
        return;
    }

    if (T2 <= 0) {
        // Only sections 4 & 5 can be in 0..1.
        this->chopCubic<&GrCCGeometry::appendCubicApproximation,
                        &GrCCGeometry::appendMonotonicCubics>(p0, p1, p2, p3, T3);
        return;
    }

    Sk2f midp0, midp1; // These hold the first two bezier points of the middle section, if needed.

    if (T1 > 0) {
        Sk2f T1T1 = Sk2f(T1);
        Sk2f ab1 = lerp(p0, p1, T1T1);
        Sk2f bc1 = lerp(p1, p2, T1T1);
        Sk2f cd1 = lerp(p2, p3, T1T1);
        Sk2f abc1 = lerp(ab1, bc1, T1T1);
        Sk2f bcd1 = lerp(bc1, cd1, T1T1);
        Sk2f abcd1 = lerp(abc1, bcd1, T1T1);

        // Sections 1 & 2.
        this->chopCubic<&GrCCGeometry::appendMonotonicCubics,
                        &GrCCGeometry::appendCubicApproximation>(p0, ab1, abc1, abcd1, T0/T1);

        if (T2 >= 1) {
            // The rest of the curve is Section 3 (middle section).
            this->appendMonotonicCubics(abcd1, bcd1, cd1, p3);
            return;
        }

        // Now calculate the first two bezier points of the middle section. The final two will come
        // from when we chop the other side, as that is numerically more stable.
        midp0 = abcd1;
        midp1 = lerp(abcd1, bcd1, Sk2f((T2 - T1) / (1 - T1)));
    } else if (T2 >= 1) {
        // The entire cubic is Section 3 (middle section).
        this->appendMonotonicCubics(p0, p1, p2, p3);
        return;
    }

    SkASSERT(T2 > 0 && T2 < 1);

    Sk2f T2T2 = Sk2f(T2);
    Sk2f ab2 = lerp(p0, p1, T2T2);
    Sk2f bc2 = lerp(p1, p2, T2T2);
    Sk2f cd2 = lerp(p2, p3, T2T2);
    Sk2f abc2 = lerp(ab2, bc2, T2T2);
    Sk2f bcd2 = lerp(bc2, cd2, T2T2);
    Sk2f abcd2 = lerp(abc2, bcd2, T2T2);

    if (T1 <= 0) {
        // The curve begins at Section 3 (middle section).
        this->appendMonotonicCubics(p0, ab2, abc2, abcd2);
    } else if (T2 > T1) {
        // Section 3 (middle section).
        Sk2f midp2 = lerp(abc2, abcd2, T1/T2);
        this->appendMonotonicCubics(midp0, midp1, midp2, abcd2);
    }

    // Sections 4 & 5.
    this->chopCubic<&GrCCGeometry::appendCubicApproximation,
                    &GrCCGeometry::appendMonotonicCubics>(abcd2, bcd2, cd2, p3, (T3-T2) / (1-T2));
}

template<GrCCGeometry::AppendCubicFn AppendLeftRight>
inline void GrCCGeometry::chopCubicAtMidTangent(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                                const Sk2f& p3, const Sk2f& tan0,
                                                const Sk2f& tan3, int maxFutureSubdivisions) {
    // Find the T value whose tangent is perpendicular to the vector that bisects tan0 and -tan3.
    Sk2f n = normalize(tan0) - normalize(tan3);

    float a = 3 * dot(p3 + (p1 - p2)*3 - p0, n);
    float b = 6 * dot(p0 - p1*2 + p2, n);
    float c = 3 * dot(p1 - p0, n);

    float discr = b*b - 4*a*c;
    if (discr < 0) {
        // If this is the case then the cubic must be nearly flat.
        (this->*AppendLeftRight)(p0, p1, p2, p3, maxFutureSubdivisions);
        return;
    }

    float q = -.5f * (b + copysignf(std::sqrt(discr), b));
    float m = .5f*q*a;
    float T = std::abs(q*q - m) < std::abs(a*c - m) ? q/a : c/q;

    this->chopCubic<AppendLeftRight, AppendLeftRight>(p0, p1, p2, p3, T, maxFutureSubdivisions);
}

template<GrCCGeometry::AppendCubicFn AppendLeft, GrCCGeometry::AppendCubicFn AppendRight>
inline void GrCCGeometry::chopCubic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                    const Sk2f& p3, float T, int maxFutureSubdivisions) {
    if (T >= 1) {
        (this->*AppendLeft)(p0, p1, p2, p3, maxFutureSubdivisions);
        return;
    }

    if (T <= 0) {
        (this->*AppendRight)(p0, p1, p2, p3, maxFutureSubdivisions);
        return;
    }

    Sk2f TT = T;
    Sk2f ab = lerp(p0, p1, TT);
    Sk2f bc = lerp(p1, p2, TT);
    Sk2f cd = lerp(p2, p3, TT);
    Sk2f abc = lerp(ab, bc, TT);
    Sk2f bcd = lerp(bc, cd, TT);
    Sk2f abcd = lerp(abc, bcd, TT);
    (this->*AppendLeft)(p0, ab, abc, abcd, maxFutureSubdivisions);
    (this->*AppendRight)(abcd, bcd, cd, p3, maxFutureSubdivisions);
}

void GrCCGeometry::appendMonotonicCubics(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                         const Sk2f& p3, int maxSubdivisions) {
    SkASSERT(maxSubdivisions >= 0);
    if ((p0 == p3).allTrue()) {
        return;
    }

    if (maxSubdivisions) {
        Sk2f tan0 = first_unless_nearly_zero(p1 - p0, p2 - p0);
        Sk2f tan3 = first_unless_nearly_zero(p3 - p2, p3 - p1);

        if (!is_convex_curve_monotonic(p0, tan0, p3, tan3)) {
            this->chopCubicAtMidTangent<&GrCCGeometry::appendMonotonicCubics>(p0, p1, p2, p3,
                                                                              tan0, tan3,
                                                                              maxSubdivisions - 1);
            return;
        }
    }

    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));

    // Don't send curves to the GPU if we know they are nearly flat (or just very small).
    // Since the cubic segment is known to be convex at this point, our flatness check is simple.
    if (are_collinear(p0, (p1 + p2) * .5f, p3)) {
        p3.store(&fPoints.push_back());
        fVerbs.push_back(Verb::kLineTo);
        return;
    }

    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    p3.store(&fPoints.push_back());
    fVerbs.push_back(Verb::kMonotonicCubicTo);
    ++fCurrContourTallies.fCubics;
}

void GrCCGeometry::appendCubicApproximation(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                            const Sk2f& p3, int maxSubdivisions) {
    SkASSERT(maxSubdivisions >= 0);
    if ((p0 == p3).allTrue()) {
        return;
    }

    if (SkCubicType::kLoop != fCurrCubicType && SkCubicType::kQuadratic != fCurrCubicType) {
        // This section passes through an inflection point, so we can get away with a flat line.
        // This can cause some curves to feel slightly more flat when inspected rigorously back and
        // forth against another renderer, but for now this seems acceptable given the simplicity.
        SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));
        p3.store(&fPoints.push_back());
        fVerbs.push_back(Verb::kLineTo);
        return;
    }

    Sk2f tan0, tan3, c;
    if (!is_cubic_nearly_quadratic(p0, p1, p2, p3, tan0, tan3, c) && maxSubdivisions) {
        this->chopCubicAtMidTangent<&GrCCGeometry::appendCubicApproximation>(p0, p1, p2, p3,
                                                                             tan0, tan3,
                                                                             maxSubdivisions - 1);
        return;
    }

    if (maxSubdivisions) {
        this->appendMonotonicQuadratics(p0, c, p3);
    } else {
        this->appendSingleMonotonicQuadratic(p0, c, p3);
    }
}

GrCCGeometry::PrimitiveTallies GrCCGeometry::endContour() {
    SkASSERT(fBuildingContour);
    SkASSERT(fVerbs.count() >= fCurrContourTallies.fTriangles);

    // The fTriangles field currently contains this contour's starting verb index. We can now
    // use it to calculate the size of the contour's fan.
    int fanSize = fVerbs.count() - fCurrContourTallies.fTriangles;
    if (fCurrFanPoint == fCurrAnchorPoint) {
        --fanSize;
        fVerbs.push_back(Verb::kEndClosedContour);
    } else {
        fVerbs.push_back(Verb::kEndOpenContour);
    }

    fCurrContourTallies.fTriangles = SkTMax(fanSize - 2, 0);

    SkDEBUGCODE(fBuildingContour = false);
    return fCurrContourTallies;
}
