/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathUtils.h"

#include "GrTypes.h"
#include "SkGeometry.h"
#include "SkMathPriv.h"

SkScalar GrPathUtils::scaleToleranceToSrc(SkScalar devTol,
                                          const SkMatrix& viewM,
                                          const SkRect& pathBounds) {
    // In order to tesselate the path we get a bound on how much the matrix can
    // scale when mapping to screen coordinates.
    SkScalar stretch = viewM.getMaxScale();
    SkScalar srcTol = devTol;

    if (stretch < 0) {
        // take worst case mapRadius amoung four corners.
        // (less than perfect)
        for (int i = 0; i < 4; ++i) {
            SkMatrix mat;
            mat.setTranslate((i % 2) ? pathBounds.fLeft : pathBounds.fRight,
                             (i < 2) ? pathBounds.fTop : pathBounds.fBottom);
            mat.postConcat(viewM);
            stretch = SkMaxScalar(stretch, mat.mapRadius(SK_Scalar1));
        }
    }
    return srcTol / stretch;
}

static const int MAX_POINTS_PER_CURVE = 1 << 10;
static const SkScalar gMinCurveTol = 0.0001f;

uint32_t GrPathUtils::quadraticPointCount(const SkPoint points[], SkScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    SkASSERT(tol > 0);

    SkScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
    if (!SkScalarIsFinite(d)) {
        return MAX_POINTS_PER_CURVE;
    } else if (d <= tol) {
        return 1;
    } else {
        // Each time we subdivide, d should be cut in 4. So we need to
        // subdivide x = log4(d/tol) times. x subdivisions creates 2^(x)
        // points.
        // 2^(log4(x)) = sqrt(x);
        SkScalar divSqrt = SkScalarSqrt(d / tol);
        if (((SkScalar)SK_MaxS32) <= divSqrt) {
            return MAX_POINTS_PER_CURVE;
        } else {
            int temp = SkScalarCeilToInt(divSqrt);
            int pow2 = GrNextPow2(temp);
            // Because of NaNs & INFs we can wind up with a degenerate temp
            // such that pow2 comes out negative. Also, our point generator
            // will always output at least one pt.
            if (pow2 < 1) {
                pow2 = 1;
            }
            return SkTMin(pow2, MAX_POINTS_PER_CURVE);
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
        (p1.distanceToLineSegmentBetweenSqd(p0, p2)) < tolSqd) {
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
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    SkASSERT(tol > 0);

    SkScalar d = SkTMax(
        points[1].distanceToLineSegmentBetweenSqd(points[0], points[3]),
        points[2].distanceToLineSegmentBetweenSqd(points[0], points[3]));
    d = SkScalarSqrt(d);
    if (!SkScalarIsFinite(d)) {
        return MAX_POINTS_PER_CURVE;
    } else if (d <= tol) {
        return 1;
    } else {
        SkScalar divSqrt = SkScalarSqrt(d / tol);
        if (((SkScalar)SK_MaxS32) <= divSqrt) {
            return MAX_POINTS_PER_CURVE;
        } else {
            int temp = SkScalarCeilToInt(SkScalarSqrt(d / tol));
            int pow2 = GrNextPow2(temp);
            // Because of NaNs & INFs we can wind up with a degenerate temp
            // such that pow2 comes out negative. Also, our point generator
            // will always output at least one pt.
            if (pow2 < 1) {
                pow2 = 1;
            }
            return SkTMin(pow2, MAX_POINTS_PER_CURVE);
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
        (p1.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd &&
         p2.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd)) {
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

int GrPathUtils::worstCasePointCount(const SkPath& path, int* subpaths, SkScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    SkASSERT(tol > 0);

    int pointCount = 0;
    *subpaths = 1;

    bool first = true;

    SkPath::Iter iter(path, false);
    SkPath::Verb verb;

    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {

        switch (verb) {
            case SkPath::kLine_Verb:
                pointCount += 1;
                break;
            case SkPath::kConic_Verb: {
                SkScalar weight = iter.conicWeight();
                SkAutoConicToQuads converter;
                const SkPoint* quadPts = converter.computeQuads(pts, weight, tol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    pointCount += quadraticPointCount(quadPts + 2*i, tol);
                }
            }
            case SkPath::kQuad_Verb:
                pointCount += quadraticPointCount(pts, tol);
                break;
            case SkPath::kCubic_Verb:
                pointCount += cubicPointCount(pts, tol);
                break;
            case SkPath::kMove_Verb:
                pointCount += 1;
                if (!first) {
                    ++(*subpaths);
                }
                break;
            default:
                break;
        }
        first = false;
    }
    return pointCount;
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
        SkScalar maxD = qPts[0].distanceToSqd(qPts[1]);
        int maxEdge = 0;
        SkScalar d = qPts[1].distanceToSqd(qPts[2]);
        if (d > maxD) {
            maxD = d;
            maxEdge = 1;
        }
        d = qPts[2].distanceToSqd(qPts[0]);
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
            lineVec.setOrthog(lineVec, SkPoint::kLeft_Side);
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
       scale = SkMaxScalar(scale, SkScalarAbs(klm[i]));
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
                                    SkPathPriv::FirstDirection dir,
                                    const SkPoint p) {
    SkVector ap = p - a;
    SkScalar apXab = ap.cross(ab);
    if (SkPathPriv::kCW_FirstDirection == dir) {
        if (apXab > 0) {
            return false;
        }
    } else {
        SkASSERT(SkPathPriv::kCCW_FirstDirection == dir);
        if (apXab < 0) {
            return false;
        }
    }

    SkVector dp = p - d;
    SkScalar dpXdc = dp.cross(dc);
    if (SkPathPriv::kCW_FirstDirection == dir) {
        if (dpXdc < 0) {
            return false;
        }
    } else {
        SkASSERT(SkPathPriv::kCCW_FirstDirection == dir);
        if (dpXdc > 0) {
            return false;
        }
    }
    return true;
}

void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       SkScalar toleranceSqd,
                                       bool constrainWithinTangents,
                                       SkPathPriv::FirstDirection dir,
                                       SkTArray<SkPoint, true>* quads,
                                       int sublevel = 0) {

    // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it is
    // p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is p[1].

    SkVector ab = p[1] - p[0];
    SkVector dc = p[2] - p[3];

    if (ab.lengthSqd() < SK_ScalarNearlyZero) {
        if (dc.lengthSqd() < SK_ScalarNearlyZero) {
            SkPoint* degQuad = quads->push_back_n(3);
            degQuad[0] = p[0];
            degQuad[1] = p[0];
            degQuad[2] = p[3];
            return;
        }
        ab = p[2] - p[0];
    }
    if (dc.lengthSqd() < SK_ScalarNearlyZero) {
        dc = p[1] - p[3];
    }

    // When the ab and cd tangents are degenerate or nearly parallel with vector from d to a the
    // constraint that the quad point falls between the tangents becomes hard to enforce and we are
    // likely to hit the max subdivision count. However, in this case the cubic is approaching a
    // line and the accuracy of the quad point isn't so important. We check if the two middle cubic
    // control points are very close to the baseline vector. If so then we just pick quadratic
    // points on the control polygon.

    if (constrainWithinTangents) {
        SkVector da = p[0] - p[3];
        bool doQuads = dc.lengthSqd() < SK_ScalarNearlyZero ||
                       ab.lengthSqd() < SK_ScalarNearlyZero;
        if (!doQuads) {
            SkScalar invDALengthSqd = da.lengthSqd();
            if (invDALengthSqd > SK_ScalarNearlyZero) {
                invDALengthSqd = SkScalarInvert(invDALengthSqd);
                // cross(ab, da)^2/length(da)^2 == sqd distance from b to line from d to a.
                // same goes for point c using vector cd.
                SkScalar detABSqd = ab.cross(da);
                detABSqd = SkScalarSquare(detABSqd);
                SkScalar detDCSqd = dc.cross(da);
                detDCSqd = SkScalarSquare(detDCSqd);
                if (detABSqd * invDALengthSqd < toleranceSqd &&
                    detDCSqd * invDALengthSqd < toleranceSqd)
                {
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
            if (SkVector::DotProduct(da, dc) < 0 || SkVector::DotProduct(ab,da) > 0) {
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
    }

    static const SkScalar kLengthScale = 3 * SK_Scalar1 / 2;
    static const int kMaxSubdivs = 10;

    ab.scale(kLengthScale);
    dc.scale(kLengthScale);

    // e0 and e1 are extrapolations along vectors ab and dc.
    SkVector c0 = p[0];
    c0 += ab;
    SkVector c1 = p[3];
    c1 += dc;

    SkScalar dSqd = sublevel > kMaxSubdivs ? 0 : c0.distanceToSqd(c1);
    if (dSqd < toleranceSqd) {
        SkPoint cAvg = c0;
        cAvg += c1;
        cAvg.scale(SK_ScalarHalf);

        bool subdivide = false;

        if (constrainWithinTangents &&
            !is_point_within_cubic_tangents(p[0], ab, dc, p[3], dir, cAvg)) {
            // choose a new cAvg that is the intersection of the two tangent lines.
            ab.setOrthog(ab);
            SkScalar z0 = -ab.dot(p[0]);
            dc.setOrthog(dc);
            SkScalar z1 = -dc.dot(p[3]);
            cAvg.fX = ab.fY * z1 - z0 * dc.fY;
            cAvg.fY = z0 * dc.fX - ab.fX * z1;
            SkScalar z = ab.fX * dc.fY - ab.fY * dc.fX;
            z = SkScalarInvert(z);
            cAvg.fX *= z;
            cAvg.fY *= z;
            if (sublevel <= kMaxSubdivs) {
                SkScalar d0Sqd = c0.distanceToSqd(cAvg);
                SkScalar d1Sqd = c1.distanceToSqd(cAvg);
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
    convert_noninflect_cubic_to_quads(choppedPts + 0,
                                      toleranceSqd,
                                      constrainWithinTangents,
                                      dir,
                                      quads,
                                      sublevel + 1);
    convert_noninflect_cubic_to_quads(choppedPts + 3,
                                      toleranceSqd,
                                      constrainWithinTangents,
                                      dir,
                                      quads,
                                      sublevel + 1);
}
}

void GrPathUtils::convertCubicToQuads(const SkPoint p[4],
                                      SkScalar tolScale,
                                      SkTArray<SkPoint, true>* quads) {
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    const SkScalar tolSqd = SkScalarSquare(tolScale);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        // The direction param is ignored if the third param is false.
        convert_noninflect_cubic_to_quads(cubic, tolSqd, false,
                                          SkPathPriv::kCCW_FirstDirection, quads);
    }
}

void GrPathUtils::convertCubicToQuadsConstrainToTangents(const SkPoint p[4],
                                                         SkScalar tolScale,
                                                         SkPathPriv::FirstDirection dir,
                                                         SkTArray<SkPoint, true>* quads) {
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    const SkScalar tolSqd = SkScalarSquare(tolScale);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, tolSqd, true, dir, quads);
    }
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Computes an SkMatrix that can find the cubic KLM functionals as follows:
 *
 *     | ..K.. |   | ..kcoeffs.. |
 *     | ..L.. | = | ..lcoeffs.. | * inverse_transpose_power_basis_matrix
 *     | ..M.. |   | ..mcoeffs.. |
 *
 * 'kcoeffs' are the power basis coefficients to a scalar valued cubic function that returns the
 * signed distance to line K from a given point on the curve:
 *
 *     k(t,s) = C(t,s) * K   [C(t,s) is defined in the following comment]
 *
 * The same applies for lcoeffs and mcoeffs. These are found separately, depending on the type of
 * curve. There are 4 coefficients but 3 rows in the matrix, so in order to do this calculation the
 * caller must first remove a specific column of coefficients.
 *
 * @return which column of klm coefficients to exclude from the calculation.
 */
static int calc_inverse_transpose_power_basis_matrix(const SkPoint pts[4], SkMatrix* out) {
    using SkScalar4 = SkNx<4, SkScalar>;

    // First we convert the bezier coordinates 'pts' to power basis coefficients X,Y,W=[0 0 0 1].
    // M3 is the matrix that does this conversion. The homogeneous equation for the cubic becomes:
    //
    //                                     | X   Y   0 |
    // C(t,s) = [t^3  t^2*s  t*s^2  s^3] * | .   .   0 |
    //                                     | .   .   0 |
    //                                     | .   .   1 |
    //
    const SkScalar4 M3[3] = {SkScalar4(-1, 3, -3, 1),
                             SkScalar4(3, -6, 3, 0),
                             SkScalar4(-3, 3, 0, 0)};
    // 4th column of M3   =  SkScalar4(1, 0, 0, 0)};
    SkScalar4 X(pts[3].x(), 0, 0, 0);
    SkScalar4 Y(pts[3].y(), 0, 0, 0);
    for (int i = 2; i >= 0; --i) {
        X += M3[i] * pts[i].x();
        Y += M3[i] * pts[i].y();
    }

    // The matrix is 3x4. In order to invert it, we first need to make it square by throwing out one
    // of the top three rows. We toss the row that leaves us with the largest absolute determinant.
    // Since the right column will be [0 0 1], the determinant reduces to x0*y1 - y0*x1.
    SkScalar absDet[4];
    const SkScalar4 DETX1 = SkNx_shuffle<1,0,0,3>(X), DETY1 = SkNx_shuffle<1,0,0,3>(Y);
    const SkScalar4 DETX2 = SkNx_shuffle<2,2,1,3>(X), DETY2 = SkNx_shuffle<2,2,1,3>(Y);
    const SkScalar4 DET = DETX1 * DETY2 - DETY1 * DETX2;
    DET.abs().store(absDet);
    const int skipRow = absDet[0] > absDet[2] ? (absDet[0] > absDet[1] ? 0 : 1)
                                              : (absDet[1] > absDet[2] ? 1 : 2);
    const SkScalar rdet = 1 / DET[skipRow];
    const int row0 = (0 != skipRow) ? 0 : 1;
    const int row1 = (2 == skipRow) ? 1 : 2;

    // Compute the inverse-transpose of the power basis matrix with the 'skipRow'th row removed.
    // Since W=[0 0 0 1], it follows that our corresponding solution will be equal to:
    //
    //             |  y1  -x1   x1*y2 - y1*x2 |
    //     1/det * | -y0   x0  -x0*y2 + y0*x2 |
    //             |   0    0             det |
    //
    const SkScalar4 R(rdet, rdet, rdet, 1);
    X *= R;
    Y *= R;

    SkScalar x[4], y[4], z[4];
    X.store(x);
    Y.store(y);
    (X * SkNx_shuffle<3,3,3,3>(Y) - Y * SkNx_shuffle<3,3,3,3>(X)).store(z);

    out->setAll( y[row1], -x[row1],  z[row1],
                -y[row0],  x[row0], -z[row0],
                       0,        0,        1);

    return skipRow;
}

static void negate_kl(SkMatrix* klm) {
    // We could use klm->postScale(-1, -1), but it ends up doing a full matrix multiply.
    for (int i = 0; i < 6; ++i) {
        (*klm)[i] = -(*klm)[i];
    }
}

static void calc_serp_klm(const SkPoint pts[4], const SkScalar d[3], SkMatrix* klm) {
    SkMatrix CIT;
    int skipCol = calc_inverse_transpose_power_basis_matrix(pts, &CIT);

    const SkScalar root = SkScalarSqrt(9 * d[1] * d[1] - 12 * d[0] * d[2]);

    const SkScalar tl = 3 * d[1] + root;
    const SkScalar sl = 6 * d[0];
    const SkScalar tm = 3 * d[1] - root;
    const SkScalar sm = 6 * d[0];

    SkMatrix klmCoeffs;
    int col = 0;
    if (0 != skipCol) {
        klmCoeffs[0] = 0;
        klmCoeffs[3] = -sl * sl * sl;
        klmCoeffs[6] = -sm * sm * sm;
        ++col;
    }
    if (1 != skipCol) {
        klmCoeffs[col + 0] = sl * sm;
        klmCoeffs[col + 3] = 3 * sl * sl * tl;
        klmCoeffs[col + 6] = 3 * sm * sm * tm;
        ++col;
    }
    if (2 != skipCol) {
        klmCoeffs[col + 0] = -tl * sm - tm * sl;
        klmCoeffs[col + 3] = -3 * sl * tl * tl;
        klmCoeffs[col + 6] = -3 * sm * tm * tm;
        ++col;
    }

    SkASSERT(2 == col);
    klmCoeffs[2] = tl * tm;
    klmCoeffs[5] = tl * tl * tl;
    klmCoeffs[8] = tm * tm * tm;

    klm->setConcat(klmCoeffs, CIT);

    // If d0 > 0 we need to flip the orientation of our curve
    // This is done by negating the k and l values
    // We want negative distance values to be on the inside
    if (d[0] > 0) {
        negate_kl(klm);
    }
}

static void calc_loop_klm(const SkPoint pts[4], SkScalar d1, SkScalar td, SkScalar sd,
                          SkScalar te, SkScalar se, SkMatrix* klm) {
    SkMatrix CIT;
    int skipCol = calc_inverse_transpose_power_basis_matrix(pts, &CIT);

    const SkScalar tesd = te * sd;
    const SkScalar tdse = td * se;

    SkMatrix klmCoeffs;
    int col = 0;
    if (0 != skipCol) {
        klmCoeffs[0] = 0;
        klmCoeffs[3] = -sd * sd * se;
        klmCoeffs[6] = -se * se * sd;
        ++col;
    }
    if (1 != skipCol) {
        klmCoeffs[col + 0] = sd * se;
        klmCoeffs[col + 3] = sd * (2 * tdse + tesd);
        klmCoeffs[col + 6] = se * (2 * tesd + tdse);
        ++col;
    }
    if (2 != skipCol) {
        klmCoeffs[col + 0] = -tdse - tesd;
        klmCoeffs[col + 3] = -td * (tdse + 2 * tesd);
        klmCoeffs[col + 6] = -te * (tesd + 2 * tdse);
        ++col;
    }

    SkASSERT(2 == col);
    klmCoeffs[2] = td * te;
    klmCoeffs[5] = td * td * te;
    klmCoeffs[8] = te * te * td;

    klm->setConcat(klmCoeffs, CIT);

    // For the general loop curve, we flip the orientation in the same pattern as the serp case
    // above. Thus we only check d1. Technically we should check the value of the hessian as well
    // cause we care about the sign of d1*Hessian. However, the Hessian is always negative outside
    // the loop section and positive inside. We take care of the flipping for the loop sections
    // later on.
    if (d1 > 0) {
        negate_kl(klm);
    }
}

// For the case when we have a cusp at a parameter value of infinity (discr == 0, d1 == 0).
static void calc_inf_cusp_klm(const SkPoint pts[4], SkScalar d2, SkScalar d3, SkMatrix* klm) {
    SkMatrix CIT;
    int skipCol = calc_inverse_transpose_power_basis_matrix(pts, &CIT);

    const SkScalar tn = d3;
    const SkScalar sn = 3 * d2;

    SkMatrix klmCoeffs;
    int col = 0;
    if (0 != skipCol) {
        klmCoeffs[0] = 0;
        klmCoeffs[3] = -sn * sn * sn;
        ++col;
    }
    if (1 != skipCol) {
        klmCoeffs[col + 0] = 0;
        klmCoeffs[col + 3] = 3 * sn * sn * tn;
        ++col;
    }
    if (2 != skipCol) {
        klmCoeffs[col + 0] = -sn;
        klmCoeffs[col + 3] = -3 * sn * tn * tn;
        ++col;
    }

    SkASSERT(2 == col);
    klmCoeffs[2] = tn;
    klmCoeffs[5] = tn * tn * tn;

    klmCoeffs[6] = 0;
    klmCoeffs[7] = 0;
    klmCoeffs[8] = 1;

    klm->setConcat(klmCoeffs, CIT);
}

// For the case when a cubic bezier is actually a quadratic. We duplicate k in l so that the
// implicit becomes:
//
//     k^3 - l*m == k^3 - l*k == k * (k^2 - l)
//
// In the quadratic case we can simply assign fixed values at each control point:
//
//     | ..K.. |     | pts[0]  pts[1]  pts[2]  pts[3] |      | 0   1/3  2/3  1 |
//     | ..L.. |  *  |   .       .       .       .    |  ==  | 0     0  1/3  1 |
//     | ..K.. |     |   1       1       1       1    |      | 0   1/3  2/3  1 |
//
static void calc_quadratic_klm(const SkPoint pts[4], SkScalar d3, SkMatrix* klm) {
    SkMatrix klmAtPts;
    klmAtPts.setAll(0,  1.f/3,  1,
                    0,      0,  1,
                    0,  1.f/3,  1);

    SkMatrix inversePts;
    inversePts.setAll(pts[0].x(),  pts[1].x(),  pts[3].x(),
                      pts[0].y(),  pts[1].y(),  pts[3].y(),
                               1,           1,           1);
    SkAssertResult(inversePts.invert(&inversePts));

    klm->setConcat(klmAtPts, inversePts);

    // If d3 > 0 we need to flip the orientation of our curve
    // This is done by negating the k and l values
    if (d3 > 0) {
        negate_kl(klm);
    }
}

// For the case when a cubic bezier is actually a line. We set K=0, L=1, M=-line, which results in
// the following implicit:
//
//     k^3 - l*m == 0^3 - 1*(-line) == -(-line) == line
//
static void calc_line_klm(const SkPoint pts[4], SkMatrix* klm) {
    SkScalar ny = pts[0].x() - pts[3].x();
    SkScalar nx = pts[3].y() - pts[0].y();
    SkScalar k = nx * pts[0].x() + ny * pts[0].y();
    klm->setAll(  0,   0, 0,
                  0,   0, 1,
                -nx, -ny, k);
}

int GrPathUtils::chopCubicAtLoopIntersection(const SkPoint src[4], SkPoint dst[10], SkMatrix* klm,
                                             int* loopIndex) {
    // Variables to store the two parametric values at the loop double point.
    SkScalar t1 = 0, t2 = 0;

    // Homogeneous parametric values at the loop double point.
    SkScalar td, sd, te, se;

    SkScalar d[3];
    SkCubicType cType = SkClassifyCubic(src, d);

    int chop_count = 0;
    if (kLoop_SkCubicType == cType) {
        SkScalar tempSqrt = SkScalarSqrt(4.f * d[0] * d[2] - 3.f * d[1] * d[1]);
        td = d[1] + tempSqrt;
        sd = 2.f * d[0];
        te = d[1] - tempSqrt;
        se = 2.f * d[0];

        t1 = td / sd;
        t2 = te / se;
        // need to have t values sorted since this is what is expected by SkChopCubicAt
        if (t1 > t2) {
            SkTSwap(t1, t2);
        }

        SkScalar chop_ts[2];
        if (t1 > 0.f && t1 < 1.f) {
            chop_ts[chop_count++] = t1;
        }
        if (t2 > 0.f && t2 < 1.f) {
            chop_ts[chop_count++] = t2;
        }
        if(dst) {
            SkChopCubicAt(src, dst, chop_ts, chop_count);
        }
    } else {
        if (dst) {
            memcpy(dst, src, sizeof(SkPoint) * 4);
        }
    }

    if (loopIndex) {
        if (2 == chop_count) {
            *loopIndex = 1;
        } else if (1 == chop_count) {
            if (t1 < 0.f) {
                *loopIndex = 0;
            } else {
                *loopIndex = 1;
            }
        } else {
            if (t1 < 0.f && t2 > 1.f) {
                *loopIndex = 0;
            } else {
                *loopIndex = -1;
            }
        }
    }

    if (klm) {
        switch (cType) {
            case kSerpentine_SkCubicType:
                calc_serp_klm(src, d, klm);
                break;
            case kLoop_SkCubicType:
                calc_loop_klm(src, d[0], td, sd, te, se, klm);
                break;
            case kCusp_SkCubicType:
                if (0 != d[0]) {
                    // FIXME: SkClassifyCubic has a tolerance, but we need an exact classification
                    // here to be sure we won't get a negative in the square root.
                    calc_serp_klm(src, d, klm);
                } else {
                    calc_inf_cusp_klm(src, d[1], d[2], klm);
                }
                break;
            case kQuadratic_SkCubicType:
                calc_quadratic_klm(src, d[2], klm);
                break;
            case kLine_SkCubicType:
            case kPoint_SkCubicType:
                calc_line_klm(src, klm);
                break;
        };
    }
    return chop_count + 1;
}
