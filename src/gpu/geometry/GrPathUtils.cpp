/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrPathUtils.h"

#include "include/gpu/GrTypes.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkUtils.h"

static const SkScalar gMinCurveTol = 0.0001f;

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
    if (srcTol < gMinCurveTol) {
        srcTol = gMinCurveTol;
    }
    return srcTol;
}

uint32_t GrPathUtils::quadraticPointCount(const SkPoint points[], SkScalar tol) {
    // You should have called scaleToleranceToSrc, which guarantees this
    SkASSERT(tol >= gMinCurveTol);

    SkScalar d = SkPointPriv::DistanceToLineSegmentBetween(points[1], points[0], points[2]);
    if (!SkScalarIsFinite(d)) {
        return kMaxPointsPerCurve;
    } else if (d <= tol) {
        return 1;
    } else {
        // Each time we subdivide, d should be cut in 4. So we need to
        // subdivide x = log4(d/tol) times. x subdivisions creates 2^(x)
        // points.
        // 2^(log4(x)) = sqrt(x);
        SkScalar divSqrt = SkScalarSqrt(d / tol);
        if (((SkScalar)SK_MaxS32) <= divSqrt) {
            return kMaxPointsPerCurve;
        } else {
            int temp = SkScalarCeilToInt(divSqrt);
            int pow2 = GrNextPow2(temp);
            // Because of NaNs & INFs we can wind up with a degenerate temp
            // such that pow2 comes out negative. Also, our point generator
            // will always output at least one pt.
            if (pow2 < 1) {
                pow2 = 1;
            }
            return std::min(pow2, kMaxPointsPerCurve);
        }
    }
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

uint32_t GrPathUtils::cubicPointCount(const SkPoint points[],
                                           SkScalar tol) {
    // You should have called scaleToleranceToSrc, which guarantees this
    SkASSERT(tol >= gMinCurveTol);

    SkScalar d = std::max(
        SkPointPriv::DistanceToLineSegmentBetweenSqd(points[1], points[0], points[3]),
        SkPointPriv::DistanceToLineSegmentBetweenSqd(points[2], points[0], points[3]));
    d = SkScalarSqrt(d);
    if (!SkScalarIsFinite(d)) {
        return kMaxPointsPerCurve;
    } else if (d <= tol) {
        return 1;
    } else {
        SkScalar divSqrt = SkScalarSqrt(d / tol);
        if (((SkScalar)SK_MaxS32) <= divSqrt) {
            return kMaxPointsPerCurve;
        } else {
            int temp = SkScalarCeilToInt(SkScalarSqrt(d / tol));
            int pow2 = GrNextPow2(temp);
            // Because of NaNs & INFs we can wind up with a degenerate temp
            // such that pow2 comes out negative. Also, our point generator
            // will always output at least one pt.
            if (pow2 < 1) {
                pow2 = 1;
            }
            return std::min(pow2, kMaxPointsPerCurve);
        }
    }
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
    SkMatrix m;
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
    double det = x0*y1 - y0*x1 + x2*y0 - y2*x0 + x1*y2 - y1*x2;

    if (!sk_float_isfinite(det)
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
        double a2, a3, a4, a5, a6, a7, a8;
        a2 = x1*y2-x2*y1;

        a3 = y2-y0;
        a4 = x0-x2;
        a5 = x2*y0-x0*y2;

        a6 = y0-y1;
        a7 = x1-x0;
        a8 = x0*y1-x1*y0;

        // this performs the uv_pts*adjugate(control_pts) multiply,
        // then does the scale by 1/det afterwards to improve precision
        m[SkMatrix::kMScaleX] = (float)((0.5*a3 + a6)*scale);
        m[SkMatrix::kMSkewX]  = (float)((0.5*a4 + a7)*scale);
        m[SkMatrix::kMTransX] = (float)((0.5*a5 + a8)*scale);

        m[SkMatrix::kMSkewY]  = (float)(a6*scale);
        m[SkMatrix::kMScaleY] = (float)(a7*scale);
        m[SkMatrix::kMTransY] = (float)(a8*scale);

        // kMPersp0 & kMPersp1 should algebraically be zero
        m[SkMatrix::kMPersp0] = 0.0f;
        m[SkMatrix::kMPersp1] = 0.0f;
        m[SkMatrix::kMPersp2] = (float)((a2 + a5 + a8)*scale);

        // It may not be normalized to have 1.0 in the bottom right
        float m33 = m.get(SkMatrix::kMPersp2);
        if (1.f != m33) {
            m33 = 1.f / m33;
            fM[0] = m33 * m.get(SkMatrix::kMScaleX);
            fM[1] = m33 * m.get(SkMatrix::kMSkewX);
            fM[2] = m33 * m.get(SkMatrix::kMTransX);
            fM[3] = m33 * m.get(SkMatrix::kMSkewY);
            fM[4] = m33 * m.get(SkMatrix::kMScaleY);
            fM[5] = m33 * m.get(SkMatrix::kMTransY);
        } else {
            fM[0] = m.get(SkMatrix::kMScaleX);
            fM[1] = m.get(SkMatrix::kMSkewX);
            fM[2] = m.get(SkMatrix::kMTransX);
            fM[3] = m.get(SkMatrix::kMSkewY);
            fM[4] = m.get(SkMatrix::kMScaleY);
            fM[5] = m.get(SkMatrix::kMTransY);
        }
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
                                       SkTArray<SkPoint, true>* quads,
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
            // GrAAHairlinePathRenderer.
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
                                                       SkTArray<SkPoint, true>* quads,
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
            z = SkScalarInvert(z);
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
                                      SkTArray<SkPoint, true>* quads) {
    if (!p[0].isFinite() || !p[1].isFinite() || !p[2].isFinite() || !p[3].isFinite()) {
        return;
    }
    if (!SkScalarIsFinite(tolScale)) {
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
                                                         SkTArray<SkPoint, true>* quads) {
    if (!p[0].isFinite() || !p[1].isFinite() || !p[2].isFinite() || !p[3].isFinite()) {
        return;
    }
    if (!SkScalarIsFinite(tolScale)) {
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

int GrPathUtils::findCubicConvex180Chops(const SkPoint pts[], float T[2], bool* areCusps) {
    using grvx::float2;
    SkASSERT(pts);
    SkASSERT(T);
    SkASSERT(areCusps);

    // If a chop falls within a distance of "kEpsilon" from 0 or 1, throw it out. Tangents become
    // unstable when we chop too close to the boundary. This works out because the tessellation
    // shaders don't allow more than 2^10 parametric segments, and they snap the beginning and
    // ending edges at 0 and 1. So if we overstep an inflection or point of 180-degree rotation by a
    // fraction of a tessellation segment, it just gets snapped.
    constexpr static float kEpsilon = 1.f / (1 << 11);
    // Floating-point representation of "1 - 2*kEpsilon".
    constexpr static uint32_t kIEEE_one_minus_2_epsilon = (127 << 23) - 2 * (1 << (24 - 11));
    // Unfortunately we don't have a way to static_assert this, but we can runtime assert that the
    // kIEEE_one_minus_2_epsilon bits are correct.
    SkASSERT(sk_bit_cast<float>(kIEEE_one_minus_2_epsilon) == 1 - 2*kEpsilon);

    float2 p0 = skvx::bit_pun<float2>(pts[0]);
    float2 p1 = skvx::bit_pun<float2>(pts[1]);
    float2 p2 = skvx::bit_pun<float2>(pts[2]);
    float2 p3 = skvx::bit_pun<float2>(pts[3]);

    // Find the cubic's power basis coefficients. These define the bezier curve as:
    //
    //                                    |T^3|
    //     Cubic(T) = x,y = |A  3B  3C| * |T^2| + P0
    //                      |.   .   .|   |T  |
    //
    // And the tangent direction (scaled by a uniform 1/3) will be:
    //
    //                                                 |T^2|
    //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
    //                                    |.   .  .|   |1  |
    //
    float2 C = p1 - p0;
    float2 D = p2 - p1;
    float2 E = p3 - p0;
    float2 B = D - C;
    float2 A = grvx::fast_madd<2>(-3,D,E);

    // Now find the cubic's inflection function. There are inflections where F' x F'' == 0.
    // We formulate this as a quadratic equation:  F' x F'' == aT^2 + bT + c == 0.
    // See: https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
    // NOTE: We only need the roots, so a uniform scale factor does not affect the solution.
    float a = grvx::cross(A,B);
    float b = grvx::cross(A,C);
    float c = grvx::cross(B,C);
    float b_over_minus_2 = -.5f * b;
    float discr_over_4 = b_over_minus_2*b_over_minus_2 - a*c;

    // If -cuspThreshold <= discr_over_4 <= cuspThreshold, it means the two roots are within
    // kEpsilon of one another (in parametric space). This is close enough for our purposes to
    // consider them a single cusp.
    float cuspThreshold = a * (kEpsilon/2);
    cuspThreshold *= cuspThreshold;

    if (discr_over_4 < -cuspThreshold) {
        // The curve does not inflect or cusp. This means it might rotate more than 180 degrees
        // instead. Chop were rotation == 180 deg. (This is the 2nd root where the tangent is
        // parallel to tan0.)
        //
        //      Tangent_Direction(T) x tan0 == 0
        //      (AT^2 x tan0) + (2BT x tan0) + (C x tan0) == 0
        //      (A x C)T^2 + (2B x C)T + (C x C) == 0  [[because tan0 == P1 - P0 == C]]
        //      bT^2 + 2cT + 0 == 0  [[because A x C == b, B x C == c]]
        //      T = [0, -2c/b]
        //
        // NOTE: if C == 0, then C != tan0. But this is fine because the curve is definitely
        // convex-180 if any points are colocated, and T[0] will equal NaN which returns 0 chops.
        *areCusps = false;
        float root = sk_ieee_float_divide(c, b_over_minus_2);
        // Is "root" inside the range [kEpsilon, 1 - kEpsilon)?
        if (sk_bit_cast<uint32_t>(root - kEpsilon) < kIEEE_one_minus_2_epsilon) {
            T[0] = root;
            return 1;
        }
        return 0;
    }

    *areCusps = (discr_over_4 <= cuspThreshold);
    if (*areCusps) {
        // The two roots are close enough that we can consider them a single cusp.
        if (a != 0 || b_over_minus_2 != 0 || c != 0) {
            // Pick the average of both roots.
            float root = sk_ieee_float_divide(b_over_minus_2, a);
            // Is "root" inside the range [kEpsilon, 1 - kEpsilon)?
            if (sk_bit_cast<uint32_t>(root - kEpsilon) < kIEEE_one_minus_2_epsilon) {
                T[0] = root;
                return 1;
            }
            return 0;
        }

        // The curve is a flat line. The standard inflection function doesn't detect cusps from flat
        // lines. Find cusps by searching instead for points where the tangent is perpendicular to
        // tan0. This will find any cusp point.
        //
        //     dot(tan0, Tangent_Direction(T)) == 0
        //
        //                         |T^2|
        //     tan0 * |A  2B  C| * |T  | == 0
        //            |.   .  .|   |1  |
        //
        float2 tan0 = skvx::if_then_else(C != 0, C, p2 - p0);
        a = grvx::dot(tan0, A);
        b_over_minus_2 = -grvx::dot(tan0, B);
        c = grvx::dot(tan0, C);
        discr_over_4 = std::max(b_over_minus_2*b_over_minus_2 - a*c, 0.f);
    }

    // Solve our quadratic equation to find where to chop. See the quadratic formula from
    // Numerical Recipes in C.
    float q = sqrtf(discr_over_4);
    q = copysignf(q, b_over_minus_2);
    q = q + b_over_minus_2;
    float2 roots = float2{q,c} / float2{a,q};

    auto inside = (roots > kEpsilon) & (roots < (1 - kEpsilon));
    if (inside[0]) {
        if (inside[1] && roots[0] != roots[1]) {
            if (roots[0] > roots[1]) {
                roots = skvx::shuffle<1,0>(roots);  // Sort.
            }
            roots.store(T);
            return 2;
        }
        T[0] = roots[0];
        return 1;
    }
    if (inside[1]) {
        T[0] = roots[1];
        return 1;
    }
    return 0;
}
