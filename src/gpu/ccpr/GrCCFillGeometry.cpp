/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCFillGeometry.h"

#include "include/gpu/GrTypes.h"
#include "src/core/SkGeometry.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static constexpr float kFlatnessThreshold = 1/16.f; // 1/16 of a pixel.

void GrCCFillGeometry::beginPath() {
    SkASSERT(!fBuildingContour);
    fVerbs.push_back(Verb::kBeginPath);
}

void GrCCFillGeometry::beginContour(const SkPoint& pt) {
    SkASSERT(!fBuildingContour);
    // Store the current verb count in the fTriangles field for now. When we close the contour we
    // will use this value to calculate the actual number of triangles in its fan.
    fCurrContourTallies = {fVerbs.count(), 0, 0, 0, 0};

    fPoints.push_back(pt);
    fVerbs.push_back(Verb::kBeginContour);
    fCurrAnchorPoint = pt;

    SkDEBUGCODE(fBuildingContour = true);
}

void GrCCFillGeometry::lineTo(const SkPoint P[2]) {
    SkASSERT(fBuildingContour);
    SkASSERT(P[0] == fPoints.back());
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    this->appendLine(p0, p1);
}

inline void GrCCFillGeometry::appendLine(const Sk2f& p0, const Sk2f& p1) {
    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));
    if ((p0 == p1).allTrue()) {
        return;
    }
    p1.store(&fPoints.push_back());
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

static inline bool are_collinear(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                 float tolerance = kFlatnessThreshold) {
    Sk2f l = p2 - p0; // Line from p0 -> p2.

    // lwidth = Manhattan width of l.
    Sk2f labs = l.abs();
    float lwidth = labs[0] + labs[1];

    // d = |p1 - p0| dot | l.y|
    //                   |-l.x| = distance from p1 to l.
    Sk2f dd = (p1 - p0) * SkNx_shuffle<1,0>(l);
    float d = dd[0] - dd[1];

    // We are collinear if a box with radius "tolerance", centered on p1, touches the line l.
    // To decide this, we check if the distance from p1 to the line is less than the distance from
    // p1 to the far corner of this imaginary box, along that same normal vector.
    // The far corner of the box can be found at "p1 + sign(n) * tolerance", where n is normal to l:
    //
    //   abs(dot(p1 - p0, n)) <= dot(sign(n) * tolerance, n)
    //
    // Which reduces to:
    //
    //   abs(d) <= (n.x * sign(n.x) + n.y * sign(n.y)) * tolerance
    //   abs(d) <= (abs(n.x) + abs(n.y)) * tolerance
    //
    // Use "<=" in case l == 0.
    return std::abs(d) <= lwidth * tolerance;
}

static inline bool are_collinear(const SkPoint P[4], float tolerance = kFlatnessThreshold) {
    Sk4f Px, Py;               // |Px  Py|   |p0 - p3|
    Sk4f::Load2(P, &Px, &Py);  // |.   . | = |p1 - p3|
    Px -= Px[3];               // |.   . |   |p2 - p3|
    Py -= Py[3];               // |.   . |   |   0   |

    // Find [lx, ly] = the line from p3 to the furthest-away point from p3.
    Sk4f Pwidth = Px.abs() + Py.abs(); // Pwidth = Manhattan width of each point.
    int lidx = Pwidth[0] > Pwidth[1] ? 0 : 1;
    lidx = Pwidth[lidx] > Pwidth[2] ? lidx : 2;
    float lx = Px[lidx], ly = Py[lidx];
    float lwidth = Pwidth[lidx]; // lwidth = Manhattan width of [lx, ly].

    //     |Px  Py|
    // d = |.   . | * | ly| = distances from each point to l (two of the distances will be zero).
    //     |.   . |   |-lx|
    //     |.   . |
    Sk4f d = Px*ly - Py*lx;

    // We are collinear if boxes with radius "tolerance", centered on all 4 points all touch line l.
    // (See the rationale for this formula in the above, 3-point version of this function.)
    // Use "<=" in case l == 0.
    return (d.abs() <= lwidth * tolerance).allTrue();
}

// Returns whether the (convex) curve segment is monotonic with respect to [endPt - startPt].
static inline bool is_convex_curve_monotonic(const Sk2f& startPt, const Sk2f& tan0,
                                             const Sk2f& endPt, const Sk2f& tan1) {
    Sk2f v = endPt - startPt;
    float dot0 = dot(tan0, v);
    float dot1 = dot(tan1, v);

    // A small, negative tolerance handles floating-point error in the case when one tangent
    // approaches 0 length, meaning the (convex) curve segment is effectively a flat line.
    float tolerance = -std::max(std::abs(dot0), std::abs(dot1)) * SK_ScalarNearlyZero;
    return dot0 >= tolerance && dot1 >= tolerance;
}

template<int N> static inline SkNx<N,float> lerp(const SkNx<N,float>& a, const SkNx<N,float>& b,
                                                 const SkNx<N,float>& t) {
    return SkNx_fma(t, b - a, a);
}

void GrCCFillGeometry::quadraticTo(const SkPoint P[3]) {
    SkASSERT(fBuildingContour);
    SkASSERT(P[0] == fPoints.back());
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);

    // Don't crunch on the curve if it is nearly flat (or just very small). Flat curves can break
    // The monotonic chopping math.
    if (are_collinear(p0, p1, p2)) {
        this->appendLine(p0, p2);
        return;
    }

    this->appendQuadratics(p0, p1, p2);
}

inline void GrCCFillGeometry::appendQuadratics(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2) {
    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    // This should almost always be this case for well-behaved curves in the real world.
    if (is_convex_curve_monotonic(p0, tan0, p2, tan1)) {
        this->appendMonotonicQuadratic(p0, p1, p2);
        return;
    }

    // Chop the curve into two segments with equal curvature. To do this we find the T value whose
    // tangent angle is halfway between tan0 and tan1.
    Sk2f n = normalize(tan0) - normalize(tan1);

    // The midtangent can be found where (dQ(t) dot n) = 0:
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

    this->appendMonotonicQuadratic(p0, p01, p012);
    this->appendMonotonicQuadratic(p012, p12, p2);
}

inline void GrCCFillGeometry::appendMonotonicQuadratic(const Sk2f& p0, const Sk2f& p1,
                                                       const Sk2f& p2) {
    // Don't send curves to the GPU if we know they are nearly flat (or just very small).
    if (are_collinear(p0, p1, p2)) {
        this->appendLine(p0, p2);
        return;
    }

    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));
    SkASSERT((p0 != p2).anyTrue());
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    fVerbs.push_back(Verb::kMonotonicQuadraticTo);
    ++fCurrContourTallies.fQuadratics;
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

static inline void get_cubic_tangents(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                      const Sk2f& p3, Sk2f* tan0, Sk2f* tan1) {
    *tan0 = first_unless_nearly_zero(p1 - p0, p2 - p0);
    *tan1 = first_unless_nearly_zero(p3 - p2, p3 - p1);
}

static inline bool is_cubic_nearly_quadratic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                             const Sk2f& p3, const Sk2f& tan0, const Sk2f& tan1,
                                             Sk2f* c) {
    Sk2f c1 = SkNx_fma(Sk2f(1.5f), tan0, p0);
    Sk2f c2 = SkNx_fma(Sk2f(-1.5f), tan1, p3);
    *c = (c1 + c2) * .5f; // Hopefully optimized out if not used?
    return ((c1 - c2).abs() <= 1).allTrue();
}

enum class ExcludedTerm : bool {
    kQuadraticTerm,
    kLinearTerm
};

// Finds where to chop a non-loop around its inflection points. The resulting cubic segments will be
// chopped such that a box of radius 'padRadius', centered at any point along the curve segment, is
// guaranteed to not cross the tangent lines at the inflection points (a.k.a lines L & M).
//
// 'chops' will be filled with 0, 2, or 4 T values. The segments between T0..T1 and T2..T3 must be
// drawn with flat lines instead of cubics.
//
// A serpentine cubic has two inflection points, so this method takes Sk2f and computes the padding
// for both in SIMD.
static inline void find_chops_around_inflection_points(float padRadius, Sk2f tl, Sk2f sl,
                                                       const Sk2f& C0, const Sk2f& C1,
                                                       ExcludedTerm skipTerm, float Cdet,
                                                       SkSTArray<4, float>* chops) {
    SkASSERT(chops->empty());
    SkASSERT(padRadius >= 0);

    padRadius /= std::abs(Cdet); // Scale this single value rather than all of C^-1 later on.

    // The homogeneous parametric functions for distance from lines L & M are:
    //
    //     l(t,s) = (t*sl - s*tl)^3
    //     m(t,s) = (t*sm - s*tm)^3
    //
    // See "Resolution Independent Curve Rendering using Programmable Graphics Hardware",
    // 4.3 Finding klmn:
    //
    // https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
    //
    // From here on we use Sk2f with "L" names, but the second lane will be for line M.
    tl = (sl > 0).thenElse(tl, -tl); // Tl=tl/sl is the triple root of l(t,s). Normalize so s >= 0.
    sl = sl.abs();

    // Convert l(t,s), m(t,s) to power-basis form:
    //
    //                                                  | l3  m3 |
    //    |l(t,s)  m(t,s)| = |t^3  t^2*s  t*s^2  s^3| * | l2  m2 |
    //                                                  | l1  m1 |
    //                                                  | l0  m0 |
    //
    Sk2f l3 = sl*sl*sl;
    Sk2f l2or1 = (ExcludedTerm::kLinearTerm == skipTerm) ? sl*sl*tl*-3 : sl*tl*tl*3;

    // The equation for line L can be found as follows:
    //
    //     L = C^-1 * (l excluding skipTerm)
    //
    // (See comments for GrPathUtils::calcCubicInverseTransposePowerBasisMatrix.)
    // We are only interested in the normal to L, so only need the upper 2x2 of C^-1. And rather
    // than divide by determinant(C) here, we have already performed this divide on padRadius.
    Sk2f Lx =  C1[1]*l3 - C0[1]*l2or1;
    Sk2f Ly = -C1[0]*l3 + C0[0]*l2or1;

    // A box of radius "padRadius" is touching line L if "center dot L" is less than the Manhattan
    // with of L. (See rationale in are_collinear.)
    Sk2f Lwidth = Lx.abs() + Ly.abs();
    Sk2f pad = Lwidth * padRadius;

    // Will T=(t + cbrt(pad))/s be greater than 0? No need to solve roots outside T=0..1.
    Sk2f insideLeftPad = pad + tl*tl*tl;

    // Will T=(t - cbrt(pad))/s be less than 1? No need to solve roots outside T=0..1.
    Sk2f tms = tl - sl;
    Sk2f insideRightPad = pad - tms*tms*tms;

    // Solve for the T values where abs(l(T)) = pad.
    if (insideLeftPad[0] > 0 && insideRightPad[0] > 0) {
        float padT = cbrtf(pad[0]);
        Sk2f pts = (tl[0] + Sk2f(-padT, +padT)) / sl[0];
        pts.store(chops->push_back_n(2));
    }

    // Solve for the T values where abs(m(T)) = pad.
    if (insideLeftPad[1] > 0 && insideRightPad[1] > 0) {
        float padT = cbrtf(pad[1]);
        Sk2f pts = (tl[1] + Sk2f(-padT, +padT)) / sl[1];
        pts.store(chops->push_back_n(2));
    }
}

static inline void swap_if_greater(float& a, float& b) {
    if (a > b) {
        std::swap(a, b);
    }
}

// Finds where to chop a non-loop around its intersection point. The resulting cubic segments will
// be chopped such that a box of radius 'padRadius', centered at any point along the curve segment,
// is guaranteed to not cross the tangent lines at the intersection point (a.k.a lines L & M).
//
// 'chops' will be filled with 0, 2, or 4 T values. The segments between T0..T1 and T2..T3 must be
// drawn with quadratic splines instead of cubics.
//
// A loop intersection falls at two different T values, so this method takes Sk2f and computes the
// padding for both in SIMD.
static inline void find_chops_around_loop_intersection(float padRadius, Sk2f t2, Sk2f s2,
                                                       const Sk2f& C0, const Sk2f& C1,
                                                       ExcludedTerm skipTerm, float Cdet,
                                                       SkSTArray<4, float>* chops) {
    SkASSERT(chops->empty());
    SkASSERT(padRadius >= 0);

    padRadius /= std::abs(Cdet); // Scale this single value rather than all of C^-1 later on.

    // The parametric functions for distance from lines L & M are:
    //
    //     l(T) = (T - Td)^2 * (T - Te)
    //     m(T) = (T - Td) * (T - Te)^2
    //
    // See "Resolution Independent Curve Rendering using Programmable Graphics Hardware",
    // 4.3 Finding klmn:
    //
    // https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
    Sk2f T2 = t2/s2; // T2 is the double root of l(T).
    Sk2f T1 = SkNx_shuffle<1,0>(T2); // T1 is the other root of l(T).

    // Convert l(T), m(T) to power-basis form:
    //
    //                                      |  1   1 |
    //    |l(T)  m(T)| = |T^3  T^2  T  1| * | l2  m2 |
    //                                      | l1  m1 |
    //                                      | l0  m0 |
    //
    // From here on we use Sk2f with "L" names, but the second lane will be for line M.
    Sk2f l2 = SkNx_fma(Sk2f(-2), T2, -T1);
    Sk2f l1 = T2 * SkNx_fma(Sk2f(2), T1, T2);
    Sk2f l0 = -T2*T2*T1;

    // The equation for line L can be found as follows:
    //
    //     L = C^-1 * (l excluding skipTerm)
    //
    // (See comments for GrPathUtils::calcCubicInverseTransposePowerBasisMatrix.)
    // We are only interested in the normal to L, so only need the upper 2x2 of C^-1. And rather
    // than divide by determinant(C) here, we have already performed this divide on padRadius.
    Sk2f l2or1 = (ExcludedTerm::kLinearTerm == skipTerm) ? l2 : l1;
    Sk2f Lx = -C0[1]*l2or1 + C1[1]; // l3 is always 1.
    Sk2f Ly =  C0[0]*l2or1 - C1[0];

    // A box of radius "padRadius" is touching line L if "center dot L" is less than the Manhattan
    // with of L. (See rationale in are_collinear.)
    Sk2f Lwidth = Lx.abs() + Ly.abs();
    Sk2f pad = Lwidth * padRadius;

    // Is l(T=0) outside the padding around line L?
    Sk2f lT0 = l0; // l(T=0) = |0  0  0  1| dot |1  l2  l1  l0| = l0
    Sk2f outsideT0 = lT0.abs() - pad;

    // Is l(T=1) outside the padding around line L?
    Sk2f lT1 = (Sk2f(1) + l2 + l1 + l0).abs(); // l(T=1) = |1  1  1  1| dot |1  l2  l1  l0|
    Sk2f outsideT1 = lT1.abs() - pad;

    // Values for solving the cubic.
    Sk2f p, q, qqq, discr, numRoots, D;
    bool hasDiscr = false;

    // Values for calculating one root (rarely needed).
    Sk2f R, QQ;
    bool hasOneRootVals = false;

    // Values for calculating three roots.
    Sk2f P, cosTheta3;
    bool hasThreeRootVals = false;

    // Solve for the T values where l(T) = +pad and m(T) = -pad.
    for (int i = 0; i < 2; ++i) {
        float T = T2[i]; // T is the point we are chopping around.
        if ((T < 0 && outsideT0[i] >= 0) || (T > 1 && outsideT1[i] >= 0)) {
            // The padding around T is completely out of range. No point solving for it.
            continue;
        }

        if (!hasDiscr) {
            p = Sk2f(+.5f, -.5f) * pad;
            q = (1.f/3) * (T2 - T1);
            qqq = q*q*q;
            discr = qqq*p*2 + p*p;
            numRoots = (discr < 0).thenElse(3, 1);
            D = T2 - q;
            hasDiscr = true;
        }

        if (1 == numRoots[i]) {
            if (!hasOneRootVals) {
                Sk2f r = qqq + p;
                Sk2f s = r.abs() + discr.sqrt();
                R = (r > 0).thenElse(-s, s);
                QQ = q*q;
                hasOneRootVals = true;
            }

            float A = cbrtf(R[i]);
            float B = A != 0 ? QQ[i]/A : 0;
            // When there is only one root, ine L chops from root..1, line M chops from 0..root.
            if (1 == i) {
                chops->push_back(0);
            }
            chops->push_back(A + B + D[i]);
            if (0 == i) {
                chops->push_back(1);
            }
            continue;
        }

        if (!hasThreeRootVals) {
            P = q.abs() * -2;
            cosTheta3 = (q >= 0).thenElse(1, -1) + p / qqq.abs();
            hasThreeRootVals = true;
        }

        static constexpr float k2PiOver3 = 2 * SK_ScalarPI / 3;
        float theta = std::acos(cosTheta3[i]) * (1.f/3);
        float roots[3] = {P[i] * std::cos(theta) + D[i],
                          P[i] * std::cos(theta + k2PiOver3) + D[i],
                          P[i] * std::cos(theta - k2PiOver3) + D[i]};

        // Sort the three roots.
        swap_if_greater(roots[0], roots[1]);
        swap_if_greater(roots[1], roots[2]);
        swap_if_greater(roots[0], roots[1]);

        // Line L chops around the first 2 roots, line M chops around the second 2.
        chops->push_back_n(2, &roots[i]);
    }
}

void GrCCFillGeometry::cubicTo(const SkPoint P[4], float inflectPad, float loopIntersectPad) {
    SkASSERT(fBuildingContour);
    SkASSERT(P[0] == fPoints.back());

    // Don't crunch on the curve or inflate geometry if it is nearly flat (or just very small).
    // Flat curves can break the math below.
    if (are_collinear(P)) {
        Sk2f p0 = Sk2f::Load(P);
        Sk2f p3 = Sk2f::Load(P+3);
        this->appendLine(p0, p3);
        return;
    }

    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);
    Sk2f p3 = Sk2f::Load(P+3);

    // Also detect near-quadratics ahead of time.
    Sk2f tan0, tan1, c;
    get_cubic_tangents(p0, p1, p2, p3, &tan0, &tan1);
    if (is_cubic_nearly_quadratic(p0, p1, p2, p3, tan0, tan1, &c)) {
        this->appendQuadratics(p0, c, p3);
        return;
    }

    double tt[2], ss[2], D[4];
    fCurrCubicType = SkClassifyCubic(P, tt, ss, D);
    SkASSERT(!SkCubicIsDegenerate(fCurrCubicType));
    Sk2f t = Sk2f(static_cast<float>(tt[0]), static_cast<float>(tt[1]));
    Sk2f s = Sk2f(static_cast<float>(ss[0]), static_cast<float>(ss[1]));

    ExcludedTerm skipTerm = (std::abs(D[2]) > std::abs(D[1]))
                                    ? ExcludedTerm::kQuadraticTerm
                                    : ExcludedTerm::kLinearTerm;
    Sk2f C0 = SkNx_fma(Sk2f(3), p1 - p2, p3 - p0);
    Sk2f C1 = (ExcludedTerm::kLinearTerm == skipTerm
                       ? SkNx_fma(Sk2f(-2), p1, p0 + p2)
                       : p1 - p0) * 3;
    Sk2f C0x1 = C0 * SkNx_shuffle<1,0>(C1);
    float Cdet = C0x1[0] - C0x1[1];

    SkSTArray<4, float> chops;
    if (SkCubicType::kLoop != fCurrCubicType) {
        find_chops_around_inflection_points(inflectPad, t, s, C0, C1, skipTerm, Cdet, &chops);
    } else {
        find_chops_around_loop_intersection(loopIntersectPad, t, s, C0, C1, skipTerm, Cdet, &chops);
    }
    if (4 == chops.count() && chops[1] >= chops[2]) {
        // This just the means the KLM roots are so close that their paddings overlap. We will
        // approximate the entire middle section, but still have it chopped midway. For loops this
        // chop guarantees the append code only sees convex segments. Otherwise, it means we are (at
        // least almost) a cusp and the chop makes sure we get a sharp point.
        Sk2f ts = t * SkNx_shuffle<1,0>(s);
        chops[1] = chops[2] = (ts[0] + ts[1]) / (2*s[0]*s[1]);
    }

#ifdef SK_DEBUG
    for (int i = 1; i < chops.count(); ++i) {
        SkASSERT(chops[i] >= chops[i - 1]);
    }
#endif
    this->appendCubics(AppendCubicMode::kLiteral, p0, p1, p2, p3, chops.begin(), chops.count());
}

static inline void chop_cubic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2, const Sk2f& p3,
                              float T, Sk2f* ab, Sk2f* abc, Sk2f* abcd, Sk2f* bcd, Sk2f* cd) {
    Sk2f TT = T;
    *ab = lerp(p0, p1, TT);
    Sk2f bc = lerp(p1, p2, TT);
    *cd = lerp(p2, p3, TT);
    *abc = lerp(*ab, bc, TT);
    *bcd = lerp(bc, *cd, TT);
    *abcd = lerp(*abc, *bcd, TT);
}

void GrCCFillGeometry::appendCubics(AppendCubicMode mode, const Sk2f& p0, const Sk2f& p1,
                                    const Sk2f& p2, const Sk2f& p3, const float chops[],
                                    int numChops, float localT0, float localT1) {
    if (numChops) {
        SkASSERT(numChops > 0);
        int midChopIdx = numChops/2;
        float T = chops[midChopIdx];
        // Chops alternate between literal and approximate mode.
        AppendCubicMode rightMode = (AppendCubicMode)((bool)mode ^ (midChopIdx & 1) ^ 1);

        if (T <= localT0) {
            // T is outside 0..1. Append the right side only.
            this->appendCubics(rightMode, p0, p1, p2, p3, &chops[midChopIdx + 1],
                               numChops - midChopIdx - 1, localT0, localT1);
            return;
        }

        if (T >= localT1) {
            // T is outside 0..1. Append the left side only.
            this->appendCubics(mode, p0, p1, p2, p3, chops, midChopIdx, localT0, localT1);
            return;
        }

        float localT = (T - localT0) / (localT1 - localT0);
        Sk2f p01, p02, pT, p11, p12;
        chop_cubic(p0, p1, p2, p3, localT, &p01, &p02, &pT, &p11, &p12);
        this->appendCubics(mode, p0, p01, p02, pT, chops, midChopIdx, localT0, T);
        this->appendCubics(rightMode, pT, p11, p12, p3, &chops[midChopIdx + 1],
                           numChops - midChopIdx - 1, T, localT1);
        return;
    }

    this->appendCubics(mode, p0, p1, p2, p3);
}

void GrCCFillGeometry::appendCubics(AppendCubicMode mode, const Sk2f& p0, const Sk2f& p1,
                                    const Sk2f& p2, const Sk2f& p3, int maxSubdivisions) {
    if (SkCubicType::kLoop != fCurrCubicType) {
        // Serpentines and cusps are always monotonic after chopping around inflection points.
        SkASSERT(!SkCubicIsDegenerate(fCurrCubicType));

        if (AppendCubicMode::kApproximate == mode) {
            // This section passes through an inflection point, so we can get away with a flat line.
            // This can cause some curves to feel slightly more flat when inspected rigorously back
            // and forth against another renderer, but for now this seems acceptable given the
            // simplicity.
            this->appendLine(p0, p3);
            return;
        }
    } else {
        Sk2f tan0, tan1;
        get_cubic_tangents(p0, p1, p2, p3, &tan0, &tan1);

        if (maxSubdivisions && !is_convex_curve_monotonic(p0, tan0, p3, tan1)) {
            this->chopAndAppendCubicAtMidTangent(mode, p0, p1, p2, p3, tan0, tan1,
                                                 maxSubdivisions - 1);
            return;
        }

        if (AppendCubicMode::kApproximate == mode) {
            Sk2f c;
            if (!is_cubic_nearly_quadratic(p0, p1, p2, p3, tan0, tan1, &c) && maxSubdivisions) {
                this->chopAndAppendCubicAtMidTangent(mode, p0, p1, p2, p3, tan0, tan1,
                                                     maxSubdivisions - 1);
                return;
            }

            this->appendMonotonicQuadratic(p0, c, p3);
            return;
        }
    }

    // Don't send curves to the GPU if we know they are nearly flat (or just very small).
    // Since the cubic segment is known to be convex at this point, our flatness check is simple.
    if (are_collinear(p0, (p1 + p2) * .5f, p3)) {
        this->appendLine(p0, p3);
        return;
    }

    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));
    SkASSERT((p0 != p3).anyTrue());
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    p3.store(&fPoints.push_back());
    fVerbs.push_back(Verb::kMonotonicCubicTo);
    ++fCurrContourTallies.fCubics;
}

// Given a convex curve segment with the following order-2 tangent function:
//
//                                                       |C2x  C2y|
//     tan = some_scale * |dx/dt  dy/dt| = |t^2  t  1| * |C1x  C1y|
//                                                       |C0x  C0y|
//
// This function finds the T value whose tangent angle is halfway between the tangents at T=0 and
// T=1 (tan0 and tan1).
static inline float find_midtangent(const Sk2f& tan0, const Sk2f& tan1,
                                    const Sk2f& C2, const Sk2f& C1, const Sk2f& C0) {
    // Tangents point in the direction of increasing T, so tan0 and -tan1 both point toward the
    // midtangent. 'n' will therefore bisect tan0 and -tan1, giving us the normal to the midtangent.
    //
    //     n dot midtangent = 0
    //
    Sk2f n = normalize(tan0) - normalize(tan1);

    // Find the T value at the midtangent. This is a simple quadratic equation:
    //
    //     midtangent dot n = 0
    //
    //     (|t^2  t  1| * C) dot n = 0
    //
    //     |t^2  t  1| dot C*n = 0
    //
    // First find coeffs = C*n.
    Sk4f C[2];
    Sk2f::Store4(C, C2, C1, C0, 0);
    Sk4f coeffs = C[0]*n[0] + C[1]*n[1];

    // Now solve the quadratic.
    float a = coeffs[0], b = coeffs[1], c = coeffs[2];
    float discr = b*b - 4*a*c;
    if (discr < 0) {
        return 0; // This will only happen if the curve is a line.
    }

    // The roots are q/a and c/q. Pick the one closer to T=.5.
    float q = -.5f * (b + copysignf(std::sqrt(discr), b));
    float r = .5f*q*a;
    return std::abs(q*q - r) < std::abs(a*c - r) ? q/a : c/q;
}

inline void GrCCFillGeometry::chopAndAppendCubicAtMidTangent(AppendCubicMode mode, const Sk2f& p0,
                                                             const Sk2f& p1, const Sk2f& p2,
                                                             const Sk2f& p3, const Sk2f& tan0,
                                                             const Sk2f& tan1,
                                                             int maxFutureSubdivisions) {
    float midT = find_midtangent(tan0, tan1, p3 + (p1 - p2)*3 - p0,
                                             (p0 - p1*2 + p2)*2,
                                             p1 - p0);
    // Use positive logic since NaN fails comparisons. (However midT should not be NaN since we cull
    // near-flat cubics in cubicTo().)
    if (!(midT > 0 && midT < 1)) {
        // The cubic is flat. Otherwise there would be a real midtangent inside T=0..1.
        this->appendLine(p0, p3);
        return;
    }

    Sk2f p01, p02, pT, p11, p12;
    chop_cubic(p0, p1, p2, p3, midT, &p01, &p02, &pT, &p11, &p12);
    this->appendCubics(mode, p0, p01, p02, pT, maxFutureSubdivisions);
    this->appendCubics(mode, pT, p11, p12, p3, maxFutureSubdivisions);
}

void GrCCFillGeometry::conicTo(const SkPoint P[3], float w) {
    SkASSERT(fBuildingContour);
    SkASSERT(P[0] == fPoints.back());
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    if (!is_convex_curve_monotonic(p0, tan0, p2, tan1)) {
        // The derivative of a conic has a cumbersome order-4 denominator. However, this isn't
        // necessary if we are only interested in a vector in the same *direction* as a given
        // tangent line. Since the denominator scales dx and dy uniformly, we can throw it out
        // completely after evaluating the derivative with the standard quotient rule. This leaves
        // us with a simpler quadratic function that we use to find the midtangent.
        float midT = find_midtangent(tan0, tan1, (w - 1) * (p2 - p0),
                                                 (p2 - p0) - 2*w*(p1 - p0),
                                                 w*(p1 - p0));
        // Use positive logic since NaN fails comparisons. (However midT should not be NaN since we
        // cull near-linear conics above. And while w=0 is flat, it's not a line and has valid
        // midtangents.)
        if (!(midT > 0 && midT < 1)) {
            // The conic is flat. Otherwise there would be a real midtangent inside T=0..1.
            this->appendLine(p0, p2);
            return;
        }

        // Chop the conic at midtangent to produce two monotonic segments.
        Sk4f p3d0 = Sk4f(p0[0], p0[1], 1, 0);
        Sk4f p3d1 = Sk4f(p1[0], p1[1], 1, 0) * w;
        Sk4f p3d2 = Sk4f(p2[0], p2[1], 1, 0);
        Sk4f midT4 = midT;

        Sk4f p3d01 = lerp(p3d0, p3d1, midT4);
        Sk4f p3d12 = lerp(p3d1, p3d2, midT4);
        Sk4f p3d012 = lerp(p3d01, p3d12, midT4);

        Sk2f midpoint = Sk2f(p3d012[0], p3d012[1]) / p3d012[2];
        Sk2f ww = Sk2f(p3d01[2], p3d12[2]) * Sk2f(p3d012[2]).rsqrt();

        this->appendMonotonicConic(p0, Sk2f(p3d01[0], p3d01[1]) / p3d01[2], midpoint, ww[0]);
        this->appendMonotonicConic(midpoint, Sk2f(p3d12[0], p3d12[1]) / p3d12[2], p2, ww[1]);
        return;
    }

    this->appendMonotonicConic(p0, p1, p2, w);
}

void GrCCFillGeometry::appendMonotonicConic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                            float w) {
    SkASSERT(w >= 0);

    Sk2f base = p2 - p0;
    Sk2f baseAbs = base.abs();
    float baseWidth = baseAbs[0] + baseAbs[1];

    // Find the height of the curve. Max height always occurs at T=.5 for conics.
    Sk2f d = (p1 - p0) * SkNx_shuffle<1,0>(base);
    float h1 = std::abs(d[1] - d[0]); // Height of p1 above the base.
    float ht = h1*w, hs = 1 + w; // Height of the conic = ht/hs.

    // i.e. (ht/hs <= baseWidth * kFlatnessThreshold). Use "<=" in case base == 0.
    if (ht <= (baseWidth*hs) * kFlatnessThreshold) {
        // We are flat. (See rationale in are_collinear.)
        this->appendLine(p0, p2);
        return;
    }

    // i.e. (w > 1 && h1 - ht/hs < baseWidth).
    if (w > 1 && h1*hs - ht < baseWidth*hs) {
        // If we get within 1px of p1 when w > 1, we will pick up artifacts from the implicit
        // function's reflection. Chop at max height (T=.5) and draw a triangle instead.
        Sk2f p1w = p1*w;
        Sk2f ab = p0 + p1w;
        Sk2f bc = p1w + p2;
        Sk2f highpoint = (ab + bc) / (2*(1 + w));
        this->appendLine(p0, highpoint);
        this->appendLine(highpoint, p2);
        return;
    }

    SkASSERT(fPoints.back() == SkPoint::Make(p0[0], p0[1]));
    SkASSERT((p0 != p2).anyTrue());
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    fConicWeights.push_back(w);
    fVerbs.push_back(Verb::kMonotonicConicTo);
    ++fCurrContourTallies.fConics;
}

GrCCFillGeometry::PrimitiveTallies GrCCFillGeometry::endContour() {
    SkASSERT(fBuildingContour);
    SkASSERT(fVerbs.count() >= fCurrContourTallies.fTriangles);

    // The fTriangles field currently contains this contour's starting verb index. We can now
    // use it to calculate the size of the contour's fan.
    int fanSize = fVerbs.count() - fCurrContourTallies.fTriangles;
    if (fPoints.back() == fCurrAnchorPoint) {
        --fanSize;
        fVerbs.push_back(Verb::kEndClosedContour);
    } else {
        fVerbs.push_back(Verb::kEndOpenContour);
    }

    fCurrContourTallies.fTriangles = std::max(fanSize - 2, 0);

    SkDEBUGCODE(fBuildingContour = false);
    return fCurrContourTallies;
}
