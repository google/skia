/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/geometry/GrPathUtils.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <algorithm>

using namespace skia_private;

static const SkScalar kMinCurveTol = 0.0001f;

static float tolerance_to_wangs_precision(float srcTol) {
    // You should have called scaleToleranceToSrc, which guarantees this
    SkASSERT(srcTol >= kMinCurveTol);

    // The GrPathUtil API defines tolerance as the max distance the linear segment can be from
    // the real curve. Wang's formula guarantees the linear segments will be within 1/precision
    // of the true curve, so precision = 1/srcTol
    return 1.f / srcTol;
}

uint32_t max_bezier_vertices(uint32_t chopCount) {
    static constexpr uint32_t kMaxChopsPerCurve = 10;
    static_assert((1 << kMaxChopsPerCurve) == GrPathUtils::kMaxPointsPerCurve);
    return 1 << std::min(chopCount, kMaxChopsPerCurve);
}

SkScalar GrPathUtils::scaleToleranceToSrc(SkScalar devTol,
                                          const SkMatrix& viewM,
                                          const SkRect& pathBounds) {
    // In order to tesselate the path we get a bound on how much the matrix can
    // scale when mapping to screen coordinates.
    SkScalar stretch = viewM.getMaxScale();

    if (stretch < 0) {
        // take worst case mapRadius amoung four corners.
        // (less than perfect)
        for (int i = 0; i < 4; ++i) {
            SkMatrix mat;
            mat.setTranslate((i % 2) ? pathBounds.fLeft : pathBounds.fRight,
                             (i < 2) ? pathBounds.fTop : pathBounds.fBottom);
            mat.postConcat(viewM);
            stretch = std::max(stretch, mat.mapRadius(SK_Scalar1));
        }
    }
    SkScalar srcTol = 0;
    if (stretch <= 0) {
        // We have degenerate bounds or some degenerate matrix. Thus we set the tolerance to be the
        // max of the path pathBounds width and height.
        srcTol = std::max(pathBounds.width(), pathBounds.height());
    } else {
        srcTol = devTol / stretch;
    }
    if (srcTol < kMinCurveTol) {
        srcTol = kMinCurveTol;
    }
    return srcTol;
}

uint32_t GrPathUtils::quadraticPointCount(const SkPoint points[], SkScalar tol) {
    return max_bezier_vertices(skgpu::wangs_formula::quadratic_log2(
            tolerance_to_wangs_precision(tol), points));
}

uint32_t GrPathUtils::generateQuadraticPoints(const SkPoint& p0,
                                              const SkPoint& p1,
                                              const SkPoint& p2,
                                              SkScalar tolSqd,
                                              SkPoint** points,
                                              uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (SkPointPriv::DistanceToLineSegmentBetweenSqd(p1, p0, p2)) < tolSqd) {
        (*points)[0] = p2;
        *points += 1;
        return 1;
    }

    SkPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
    };
    SkPoint r = { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) };

    pointsLeft >>= 1;
    uint32_t a = generateQuadraticPoints(p0, q[0], r, tolSqd, points, pointsLeft);
    uint32_t b = generateQuadraticPoints(r, q[1], p2, tolSqd, points, pointsLeft);
    return a + b;
}

uint32_t GrPathUtils::cubicPointCount(const SkPoint points[], SkScalar tol) {
    return max_bezier_vertices(skgpu::wangs_formula::cubic_log2(
            tolerance_to_wangs_precision(tol), points));
}

uint32_t GrPathUtils::generateCubicPoints(const SkPoint& p0,
                                          const SkPoint& p1,
                                          const SkPoint& p2,
                                          const SkPoint& p3,
                                          SkScalar tolSqd,
                                          SkPoint** points,
                                          uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (SkPointPriv::DistanceToLineSegmentBetweenSqd(p1, p0, p3) < tolSqd &&
         SkPointPriv::DistanceToLineSegmentBetweenSqd(p2, p0, p3) < tolSqd)) {
        (*points)[0] = p3;
        *points += 1;
        return 1;
    }
    SkPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
        { SkScalarAve(p2.fX, p3.fX), SkScalarAve(p2.fY, p3.fY) }
    };
    SkPoint r[] = {
        { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) },
        { SkScalarAve(q[1].fX, q[2].fX), SkScalarAve(q[1].fY, q[2].fY) }
    };
    SkPoint s = { SkScalarAve(r[0].fX, r[1].fX), SkScalarAve(r[0].fY, r[1].fY) };
    pointsLeft >>= 1;
    uint32_t a = generateCubicPoints(p0, q[0], r[0], s, tolSqd, points, pointsLeft);
    uint32_t b = generateCubicPoints(s, r[1], q[2], p3, tolSqd, points, pointsLeft);
    return a + b;
}

void GrPathUtils::QuadUVMatrix::set(const SkPoint qPts[3]) {
    // We want M such that M * xy_pt = uv_pt
    // We know M * control_pts = [0  1/2 1]
    //                           [0  0   1]
    //                           [1  1   1]
    // And control_pts = [x0 x1 x2]
    //                   [y0 y1 y2]
    //                   [1  1  1 ]
    // We invert the control pt matrix and post concat to both sides to get M.
    // Using the known form of the control point matrix and the result, we can
    // optimize and improve precision.

    double x0 = qPts[0].fX;
    double y0 = qPts[0].fY;
    double x1 = qPts[1].fX;
    double y1 = qPts[1].fY;
    double x2 = qPts[2].fX;
    double y2 = qPts[2].fY;

    // pre-calculate some adjugate matrix factors for determinant
    double a2 = x1*y2-x2*y1;
    double a5 = x2*y0-x0*y2;
    double a8 = x0*y1-x1*y0;
    double det = a2 + a5 + a8;

    if (!SkIsFinite(det)
        || SkScalarNearlyZero((float)det, SK_ScalarNearlyZero * SK_ScalarNearlyZero)) {
        // The quad is degenerate. Hopefully this is rare. Find the pts that are
        // farthest apart to compute a line (unless it is really a pt).
        SkScalar maxD = SkPointPriv::DistanceToSqd(qPts[0], qPts[1]);
        int maxEdge = 0;
        SkScalar d = SkPointPriv::DistanceToSqd(qPts[1], qPts[2]);
        if (d > maxD) {
            maxD = d;
            maxEdge = 1;
        }
        d = SkPointPriv::DistanceToSqd(qPts[2], qPts[0]);
        if (d > maxD) {
            maxD = d;
            maxEdge = 2;
        }
        // We could have a tolerance here, not sure if it would improve anything
        if (maxD > 0) {
            // Set the matrix to give (u = 0, v = distance_to_line)
            SkVector lineVec = qPts[(maxEdge + 1)%3] - qPts[maxEdge];
            // when looking from the point 0 down the line we want positive
            // distances to be to the left. This matches the non-degenerate
            // case.
            lineVec = SkPointPriv::MakeOrthog(lineVec, SkPointPriv::kLeft_Side);
            // first row
            fM[0] = 0;
            fM[1] = 0;
            fM[2] = 0;
            // second row
            fM[3] = lineVec.fX;
            fM[4] = lineVec.fY;
            fM[5] = -lineVec.dot(qPts[maxEdge]);
        } else {
            // It's a point. It should cover zero area. Just set the matrix such
            // that (u, v) will always be far away from the quad.
            fM[0] = 0; fM[1] = 0; fM[2] = 100.f;
            fM[3] = 0; fM[4] = 0; fM[5] = 100.f;
        }
    } else {
        double scale = 1.0/det;

        // compute adjugate matrix
        double a3, a4, a6, a7;
        a3 = y2-y0;
        a4 = x0-x2;

        a6 = y0-y1;
        a7 = x1-x0;

        // this performs the uv_pts*adjugate(control_pts) multiply,
        // then does the scale by 1/det afterwards to improve precision
        fM[0] = (float)((0.5*a3 + a6)*scale);
        fM[1] = (float)((0.5*a4 + a7)*scale);
        fM[2] = (float)((0.5*a5 + a8)*scale);
        fM[3] = (float)(a6*scale);
        fM[4] = (float)(a7*scale);
        fM[5] = (float)(a8*scale);
    }
}

////////////////////////////////////////////////////////////////////////////////

// k = (y2 - y0, x0 - x2, x2*y0 - x0*y2)
// l = (y1 - y0, x0 - x1, x1*y0 - x0*y1) * 2*w
// m = (y2 - y1, x1 - x2, x2*y1 - x1*y2) * 2*w
void GrPathUtils::getConicKLM(const SkPoint p[3], const SkScalar weight, SkMatrix* out) {
    SkMatrix& klm = *out;
    const SkScalar w2 = 2.f * weight;
    klm[0] = p[2].fY - p[0].fY;
    klm[1] = p[0].fX - p[2].fX;
    klm[2] = p[2].fX * p[0].fY - p[0].fX * p[2].fY;

    klm[3] = w2 * (p[1].fY - p[0].fY);
    klm[4] = w2 * (p[0].fX - p[1].fX);
    klm[5] = w2 * (p[1].fX * p[0].fY - p[0].fX * p[1].fY);

    klm[6] = w2 * (p[2].fY - p[1].fY);
    klm[7] = w2 * (p[1].fX - p[2].fX);
    klm[8] = w2 * (p[2].fX * p[1].fY - p[1].fX * p[2].fY);

    // scale the max absolute value of coeffs to 10
    SkScalar scale = 0.f;
    for (int i = 0; i < 9; ++i) {
       scale = std::max(scale, SkScalarAbs(klm[i]));
    }
    SkASSERT(scale > 0.f);
    scale = 10.f / scale;
    for (int i = 0; i < 9; ++i) {
        klm[i] *= scale;
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace {

// a is the first control point of the cubic.
// ab is the vector from a to the second control point.
// dc is the vector from the fourth to the third control point.
// d is the fourth control point.
// p is the candidate quadratic control point.
// this assumes that the cubic doesn't inflect and is simple
bool is_point_within_cubic_tangents(const SkPoint& a,
                                    const SkVector& ab,
                                    const SkVector& dc,
                                    const SkPoint& d,
                                    SkPathFirstDirection dir,
                                    const SkPoint p) {
    SkVector ap = p - a;
    SkScalar apXab = ap.cross(ab);
    if (SkPathFirstDirection::kCW == dir) {
        if (apXab > 0) {
            return false;
        }
    } else {
        SkASSERT(SkPathFirstDirection::kCCW == dir);
        if (apXab < 0) {
            return false;
        }
    }

    SkVector dp = p - d;
    SkScalar dpXdc = dp.cross(dc);
    if (SkPathFirstDirection::kCW == dir) {
        if (dpXdc < 0) {
            return false;
        }
    } else {
        SkASSERT(SkPathFirstDirection::kCCW == dir);
        if (dpXdc > 0) {
            return false;
        }
    }
    return true;
}

void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       SkScalar toleranceSqd,
                                       TArray<SkPoint, true>* quads,
                                       int sublevel = 0,
                                       bool preserveFirstTangent = true,
                                       bool preserveLastTangent = true) {
    // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it is
    // p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is p[1].
    SkVector ab = p[1] - p[0];
    SkVector dc = p[2] - p[3];

    if (SkPointPriv::LengthSqd(ab) < SK_ScalarNearlyZero) {
        if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
            SkPoint* degQuad = quads->push_back_n(3);
            degQuad[0] = p[0];
            degQuad[1] = p[0];
            degQuad[2] = p[3];
            return;
        }
        ab = p[2] - p[0];
    }
    if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
        dc = p[1] - p[3];
    }

    static const SkScalar kLengthScale = 3 * SK_Scalar1 / 2;
    static const int kMaxSubdivs = 10;

    ab.scale(kLengthScale);
    dc.scale(kLengthScale);

    // c0 and c1 are extrapolations along vectors ab and dc.
    SkPoint c0 = p[0] + ab;
    SkPoint c1 = p[3] + dc;

    SkScalar dSqd = sublevel > kMaxSubdivs ? 0 : SkPointPriv::DistanceToSqd(c0, c1);
    if (dSqd < toleranceSqd) {
        SkPoint newC;
        if (preserveFirstTangent == preserveLastTangent) {
            // We used to force a split when both tangents need to be preserved and c0 != c1.
            // This introduced a large performance regression for tiny paths for no noticeable
            // quality improvement. However, we aren't quite fulfilling our contract of guaranteeing
            // the two tangent vectors and this could introduce a missed pixel in
            // AAHairlinePathRenderer.
            newC = (c0 + c1) * 0.5f;
        } else if (preserveFirstTangent) {
            newC = c0;
        } else {
            newC = c1;
        }

        SkPoint* pts = quads->push_back_n(3);
        pts[0] = p[0];
        pts[1] = newC;
        pts[2] = p[3];
        return;
    }
    SkPoint choppedPts[7];
    SkChopCubicAtHalf(p, choppedPts);
    convert_noninflect_cubic_to_quads(
            choppedPts + 0, toleranceSqd, quads, sublevel + 1, preserveFirstTangent, false);
    convert_noninflect_cubic_to_quads(
            choppedPts + 3, toleranceSqd, quads, sublevel + 1, false, preserveLastTangent);
}

void convert_noninflect_cubic_to_quads_with_constraint(const SkPoint p[4],
                                                       SkScalar toleranceSqd,
                                                       SkPathFirstDirection dir,
                                                       TArray<SkPoint, true>* quads,
                                                       int sublevel = 0) {
    // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it is
    // p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is p[1].

    SkVector ab = p[1] - p[0];
    SkVector dc = p[2] - p[3];

    if (SkPointPriv::LengthSqd(ab) < SK_ScalarNearlyZero) {
        if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
            SkPoint* degQuad = quads->push_back_n(3);
            degQuad[0] = p[0];
            degQuad[1] = p[0];
            degQuad[2] = p[3];
            return;
        }
        ab = p[2] - p[0];
    }
    if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
        dc = p[1] - p[3];
    }

    // When the ab and cd tangents are degenerate or nearly parallel with vector from d to a the
    // constraint that the quad point falls between the tangents becomes hard to enforce and we are
    // likely to hit the max subdivision count. However, in this case the cubic is approaching a
    // line and the accuracy of the quad point isn't so important. We check if the two middle cubic
    // control points are very close to the baseline vector. If so then we just pick quadratic
    // points on the control polygon.

    SkVector da = p[0] - p[3];
    bool doQuads = SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero ||
                   SkPointPriv::LengthSqd(ab) < SK_ScalarNearlyZero;
    if (!doQuads) {
        SkScalar invDALengthSqd = SkPointPriv::LengthSqd(da);
        if (invDALengthSqd > SK_ScalarNearlyZero) {
            invDALengthSqd = SkScalarInvert(invDALengthSqd);
            // cross(ab, da)^2/length(da)^2 == sqd distance from b to line from d to a.
            // same goes for point c using vector cd.
            SkScalar detABSqd = ab.cross(da);
            detABSqd = SkScalarSquare(detABSqd);
            SkScalar detDCSqd = dc.cross(da);
            detDCSqd = SkScalarSquare(detDCSqd);
            if (detABSqd * invDALengthSqd < toleranceSqd &&
                detDCSqd * invDALengthSqd < toleranceSqd) {
                doQuads = true;
            }
        }
    }
    if (doQuads) {
        SkPoint b = p[0] + ab;
        SkPoint c = p[3] + dc;
        SkPoint mid = b + c;
        mid.scale(SK_ScalarHalf);
        // Insert two quadratics to cover the case when ab points away from d and/or dc
        // points away from a.
        if (SkVector::DotProduct(da, dc) < 0 || SkVector::DotProduct(ab, da) > 0) {
            SkPoint* qpts = quads->push_back_n(6);
            qpts[0] = p[0];
            qpts[1] = b;
            qpts[2] = mid;
            qpts[3] = mid;
            qpts[4] = c;
            qpts[5] = p[3];
        } else {
            SkPoint* qpts = quads->push_back_n(3);
            qpts[0] = p[0];
            qpts[1] = mid;
            qpts[2] = p[3];
        }
        return;
    }

    static const SkScalar kLengthScale = 3 * SK_Scalar1 / 2;
    static const int kMaxSubdivs = 10;

    ab.scale(kLengthScale);
    dc.scale(kLengthScale);

    // c0 and c1 are extrapolations along vectors ab and dc.
    SkVector c0 = p[0] + ab;
    SkVector c1 = p[3] + dc;

    SkScalar dSqd = sublevel > kMaxSubdivs ? 0 : SkPointPriv::DistanceToSqd(c0, c1);
    if (dSqd < toleranceSqd) {
        SkPoint cAvg = (c0 + c1) * 0.5f;
        bool subdivide = false;

        if (!is_point_within_cubic_tangents(p[0], ab, dc, p[3], dir, cAvg)) {
            // choose a new cAvg that is the intersection of the two tangent lines.
            ab = SkPointPriv::MakeOrthog(ab);
            SkScalar z0 = -ab.dot(p[0]);
            dc = SkPointPriv::MakeOrthog(dc);
            SkScalar z1 = -dc.dot(p[3]);
            cAvg.fX = ab.fY * z1 - z0 * dc.fY;
            cAvg.fY = z0 * dc.fX - ab.fX * z1;
            SkScalar z = ab.fX * dc.fY - ab.fY * dc.fX;
            z = sk_ieee_float_divide(1.0f, z);
            cAvg.fX *= z;
            cAvg.fY *= z;
            if (sublevel <= kMaxSubdivs) {
                SkScalar d0Sqd = SkPointPriv::DistanceToSqd(c0, cAvg);
                SkScalar d1Sqd = SkPointPriv::DistanceToSqd(c1, cAvg);
                // We need to subdivide if d0 + d1 > tolerance but we have the sqd values. We know
                // the distances and tolerance can't be negative.
                // (d0 + d1)^2 > toleranceSqd
                // d0Sqd + 2*d0*d1 + d1Sqd > toleranceSqd
                SkScalar d0d1 = SkScalarSqrt(d0Sqd * d1Sqd);
                subdivide = 2 * d0d1 + d0Sqd + d1Sqd > toleranceSqd;
            }
        }
        if (!subdivide) {
            SkPoint* pts = quads->push_back_n(3);
            pts[0] = p[0];
            pts[1] = cAvg;
            pts[2] = p[3];
            return;
        }
    }
    SkPoint choppedPts[7];
    SkChopCubicAtHalf(p, choppedPts);
    convert_noninflect_cubic_to_quads_with_constraint(
            choppedPts + 0, toleranceSqd, dir, quads, sublevel + 1);
    convert_noninflect_cubic_to_quads_with_constraint(
            choppedPts + 3, toleranceSqd, dir, quads, sublevel + 1);
}
}  // namespace

void GrPathUtils::convertCubicToQuads(const SkPoint p[4],
                                      SkScalar tolScale,
                                      TArray<SkPoint, true>* quads) {
    if (!p[0].isFinite() || !p[1].isFinite() || !p[2].isFinite() || !p[3].isFinite()) {
        return;
    }
    if (!SkIsFinite(tolScale)) {
        return;
    }
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    const SkScalar tolSqd = SkScalarSquare(tolScale);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, tolSqd, quads);
    }
}

void GrPathUtils::convertCubicToQuadsConstrainToTangents(const SkPoint p[4],
                                                         SkScalar tolScale,
                                                         SkPathFirstDirection dir,
                                                         TArray<SkPoint, true>* quads) {
    if (!p[0].isFinite() || !p[1].isFinite() || !p[2].isFinite() || !p[3].isFinite()) {
        return;
    }
    if (!SkIsFinite(tolScale)) {
        return;
    }
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    const SkScalar tolSqd = SkScalarSquare(tolScale);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads_with_constraint(cubic, tolSqd, dir, quads);
    }
}
