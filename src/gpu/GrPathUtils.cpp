
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathUtils.h"
#include "GrPoint.h"
#include "SkGeometry.h"

SkScalar GrPathUtils::scaleToleranceToSrc(SkScalar devTol,
                                          const SkMatrix& viewM,
                                          const GrRect& pathBounds) {
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    SkScalar stretch = viewM.getMaxStretch();
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
    srcTol = SkScalarDiv(srcTol, stretch);
    return srcTol;
}

static const int MAX_POINTS_PER_CURVE = 1 << 10;
static const SkScalar gMinCurveTol = SkFloatToScalar(0.0001f);

uint32_t GrPathUtils::quadraticPointCount(const GrPoint points[],
                                          SkScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    GrAssert(tol > 0);

    SkScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
    if (d <= tol) {
        return 1;
    } else {
        // Each time we subdivide, d should be cut in 4. So we need to
        // subdivide x = log4(d/tol) times. x subdivisions creates 2^(x)
        // points.
        // 2^(log4(x)) = sqrt(x);
        int temp = SkScalarCeil(SkScalarSqrt(SkScalarDiv(d, tol)));
        int pow2 = GrNextPow2(temp);
        // Because of NaNs & INFs we can wind up with a degenerate temp
        // such that pow2 comes out negative. Also, our point generator
        // will always output at least one pt.
        if (pow2 < 1) {
            pow2 = 1;
        }
        return GrMin(pow2, MAX_POINTS_PER_CURVE);
    }
}

uint32_t GrPathUtils::generateQuadraticPoints(const GrPoint& p0,
                                              const GrPoint& p1,
                                              const GrPoint& p2,
                                              SkScalar tolSqd,
                                              GrPoint** points,
                                              uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p2)) < tolSqd) {
        (*points)[0] = p2;
        *points += 1;
        return 1;
    }

    GrPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
    };
    GrPoint r = { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) };

    pointsLeft >>= 1;
    uint32_t a = generateQuadraticPoints(p0, q[0], r, tolSqd, points, pointsLeft);
    uint32_t b = generateQuadraticPoints(r, q[1], p2, tolSqd, points, pointsLeft);
    return a + b;
}

uint32_t GrPathUtils::cubicPointCount(const GrPoint points[],
                                           SkScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    GrAssert(tol > 0);

    SkScalar d = GrMax(
        points[1].distanceToLineSegmentBetweenSqd(points[0], points[3]),
        points[2].distanceToLineSegmentBetweenSqd(points[0], points[3]));
    d = SkScalarSqrt(d);
    if (d <= tol) {
        return 1;
    } else {
        int temp = SkScalarCeil(SkScalarSqrt(SkScalarDiv(d, tol)));
        int pow2 = GrNextPow2(temp);
        // Because of NaNs & INFs we can wind up with a degenerate temp
        // such that pow2 comes out negative. Also, our point generator
        // will always output at least one pt.
        if (pow2 < 1) {
            pow2 = 1;
        }
        return GrMin(pow2, MAX_POINTS_PER_CURVE);
    }
}

uint32_t GrPathUtils::generateCubicPoints(const GrPoint& p0,
                                          const GrPoint& p1,
                                          const GrPoint& p2,
                                          const GrPoint& p3,
                                          SkScalar tolSqd,
                                          GrPoint** points,
                                          uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd &&
         p2.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd)) {
            (*points)[0] = p3;
            *points += 1;
            return 1;
        }
    GrPoint q[] = {
        { SkScalarAve(p0.fX, p1.fX), SkScalarAve(p0.fY, p1.fY) },
        { SkScalarAve(p1.fX, p2.fX), SkScalarAve(p1.fY, p2.fY) },
        { SkScalarAve(p2.fX, p3.fX), SkScalarAve(p2.fY, p3.fY) }
    };
    GrPoint r[] = {
        { SkScalarAve(q[0].fX, q[1].fX), SkScalarAve(q[0].fY, q[1].fY) },
        { SkScalarAve(q[1].fX, q[2].fX), SkScalarAve(q[1].fY, q[2].fY) }
    };
    GrPoint s = { SkScalarAve(r[0].fX, r[1].fX), SkScalarAve(r[0].fY, r[1].fY) };
    pointsLeft >>= 1;
    uint32_t a = generateCubicPoints(p0, q[0], r[0], s, tolSqd, points, pointsLeft);
    uint32_t b = generateCubicPoints(s, r[1], q[2], p3, tolSqd, points, pointsLeft);
    return a + b;
}

int GrPathUtils::worstCasePointCount(const SkPath& path, int* subpaths,
                                     SkScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    GrAssert(tol > 0);

    int pointCount = 0;
    *subpaths = 1;

    bool first = true;

    SkPath::Iter iter(path, false);
    GrPathCmd cmd;

    GrPoint pts[4];
    while ((cmd = (GrPathCmd)iter.next(pts)) != kEnd_PathCmd) {

        switch (cmd) {
            case kLine_PathCmd:
                pointCount += 1;
                break;
            case kQuadratic_PathCmd:
                pointCount += quadraticPointCount(pts, tol);
                break;
            case kCubic_PathCmd:
                pointCount += cubicPointCount(pts, tol);
                break;
            case kMove_PathCmd:
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

void GrPathUtils::QuadUVMatrix::set(const GrPoint qPts[3]) {
    // can't make this static, no cons :(
    SkMatrix UVpts;
#ifndef SK_SCALAR_IS_FLOAT
    GrCrash("Expected scalar is float.");
#endif
    SkMatrix m;
    // We want M such that M * xy_pt = uv_pt
    // We know M * control_pts = [0  1/2 1]
    //                           [0  0   1]
    //                           [1  1   1]
    // We invert the control pt matrix and post concat to both sides to get M.
    UVpts.setAll(0,   SK_ScalarHalf,  SK_Scalar1,
                 0,               0,  SK_Scalar1,
                 SkScalarToPersp(SK_Scalar1),
                 SkScalarToPersp(SK_Scalar1),
                 SkScalarToPersp(SK_Scalar1));
    m.setAll(qPts[0].fX, qPts[1].fX, qPts[2].fX,
             qPts[0].fY, qPts[1].fY, qPts[2].fY,
             SkScalarToPersp(SK_Scalar1),
             SkScalarToPersp(SK_Scalar1),
             SkScalarToPersp(SK_Scalar1));
    if (!m.invert(&m)) {
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
            GrVec lineVec = qPts[(maxEdge + 1)%3] - qPts[maxEdge];
            // when looking from the point 0 down the line we want positive
            // distances to be to the left. This matches the non-degenerate
            // case.
            lineVec.setOrthog(lineVec, GrPoint::kLeft_Side);
            lineVec.dot(qPts[0]);
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
        m.postConcat(UVpts);

        // The matrix should not have perspective.
        SkDEBUGCODE(static const SkScalar gTOL = SkFloatToScalar(1.f / 100.f));
        GrAssert(SkScalarAbs(m.get(SkMatrix::kMPersp0)) < gTOL);
        GrAssert(SkScalarAbs(m.get(SkMatrix::kMPersp1)) < gTOL);

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
                                    SkPath::Direction dir,
                                    const SkPoint p) {
    SkVector ap = p - a;
    SkScalar apXab = ap.cross(ab);
    if (SkPath::kCW_Direction == dir) {
        if (apXab > 0) {
            return false;
        }
    } else {
        GrAssert(SkPath::kCCW_Direction == dir);
        if (apXab < 0) {
            return false;
        }
    }

    SkVector dp = p - d;
    SkScalar dpXdc = dp.cross(dc);
    if (SkPath::kCW_Direction == dir) {
        if (dpXdc < 0) {
            return false;
        }
    } else {
        GrAssert(SkPath::kCCW_Direction == dir);
        if (dpXdc > 0) {
            return false;
        }
    }
    return true;
}

void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       SkScalar toleranceSqd,
                                       bool constrainWithinTangents,
                                       SkPath::Direction dir,
                                       SkTArray<SkPoint, true>* quads,
                                       int sublevel = 0) {

    // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it is
    // p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is p[1].

    SkVector ab = p[1] - p[0];
    SkVector dc = p[2] - p[3];

    if (ab.isZero()) {
        if (dc.isZero()) {
            SkPoint* degQuad = quads->push_back_n(3);
            degQuad[0] = p[0];
            degQuad[1] = p[0];
            degQuad[2] = p[3];
            return;
        }
        ab = p[2] - p[0];
    }
    if (dc.isZero()) {
        dc = p[1] - p[3];
    }

    // When the ab and cd tangents are nearly parallel with vector from d to a the constraint that
    // the quad point falls between the tangents becomes hard to enforce and we are likely to hit
    // the max subdivision count. However, in this case the cubic is approaching a line and the
    // accuracy of the quad point isn't so important. We check if the two middle cubic control
    // points are very close to the baseline vector. If so then we just pick quadratic points on the
    // control polygon.

    if (constrainWithinTangents) {
        SkVector da = p[0] - p[3];
        SkScalar invDALengthSqd = da.lengthSqd();
        if (invDALengthSqd > SK_ScalarNearlyZero) {
            invDALengthSqd = SkScalarInvert(invDALengthSqd);
            // cross(ab, da)^2/length(da)^2 == sqd distance from b to line from d to a.
            // same goed for point c using vector cd.
            SkScalar detABSqd = ab.cross(da);
            detABSqd = SkScalarSquare(detABSqd);
            SkScalar detDCSqd = dc.cross(da);
            detDCSqd = SkScalarSquare(detDCSqd);
            if (SkScalarMul(detABSqd, invDALengthSqd) < toleranceSqd &&
                SkScalarMul(detDCSqd, invDALengthSqd) < toleranceSqd) {
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
            cAvg.fX = SkScalarMul(ab.fY, z1) - SkScalarMul(z0, dc.fY);
            cAvg.fY = SkScalarMul(z0, dc.fX) - SkScalarMul(ab.fX, z1);
            SkScalar z = SkScalarMul(ab.fX, dc.fY) - SkScalarMul(ab.fY, dc.fX);
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
                SkScalar d0d1 = SkScalarSqrt(SkScalarMul(d0Sqd, d1Sqd));
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

void GrPathUtils::convertCubicToQuads(const GrPoint p[4],
                                      SkScalar tolScale,
                                      bool constrainWithinTangents,
                                      SkPath::Direction dir,
                                      SkTArray<SkPoint, true>* quads) {
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    // base tolerance is 1 pixel.
    static const SkScalar kTolerance = SK_Scalar1;
    const SkScalar tolSqd = SkScalarSquare(SkScalarMul(tolScale, kTolerance));

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, tolSqd, constrainWithinTangents, dir, quads);
    }

}
