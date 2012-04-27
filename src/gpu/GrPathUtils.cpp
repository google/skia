
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathUtils.h"
#include "GrPoint.h"
#include "SkGeometry.h"

GrScalar GrPathUtils::scaleToleranceToSrc(GrScalar devTol,
                                          const GrMatrix& viewM,
                                          const GrRect& pathBounds) {
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    GrScalar srcTol = devTol;

    if (stretch < 0) {
        // take worst case mapRadius amoung four corners.
        // (less than perfect)
        for (int i = 0; i < 4; ++i) {
            GrMatrix mat;
            mat.setTranslate((i % 2) ? pathBounds.fLeft : pathBounds.fRight,
                             (i < 2) ? pathBounds.fTop : pathBounds.fBottom);
            mat.postConcat(viewM);
            stretch = SkMaxScalar(stretch, mat.mapRadius(SK_Scalar1));
        }
    }
    srcTol = GrScalarDiv(srcTol, stretch);
    return srcTol;
}

static const int MAX_POINTS_PER_CURVE = 1 << 10;
static const GrScalar gMinCurveTol = GrFloatToScalar(0.0001f);

uint32_t GrPathUtils::quadraticPointCount(const GrPoint points[],
                                          GrScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    GrAssert(tol > 0);

    GrScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
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
                                              GrScalar tolSqd,
                                              GrPoint** points,
                                              uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p2)) < tolSqd) {
        (*points)[0] = p2;
        *points += 1;
        return 1;
    }

    GrPoint q[] = {
        { GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY) },
        { GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY) },
    };
    GrPoint r = { GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY) };

    pointsLeft >>= 1;
    uint32_t a = generateQuadraticPoints(p0, q[0], r, tolSqd, points, pointsLeft);
    uint32_t b = generateQuadraticPoints(r, q[1], p2, tolSqd, points, pointsLeft);
    return a + b;
}

uint32_t GrPathUtils::cubicPointCount(const GrPoint points[],
                                           GrScalar tol) {
    if (tol < gMinCurveTol) {
        tol = gMinCurveTol;
    }
    GrAssert(tol > 0);

    GrScalar d = GrMax(
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
                                          GrScalar tolSqd,
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
        { GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY) },
        { GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY) },
        { GrScalarAve(p2.fX, p3.fX), GrScalarAve(p2.fY, p3.fY) }
    };
    GrPoint r[] = {
        { GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY) },
        { GrScalarAve(q[1].fX, q[2].fX), GrScalarAve(q[1].fY, q[2].fY) }
    };
    GrPoint s = { GrScalarAve(r[0].fX, r[1].fX), GrScalarAve(r[0].fY, r[1].fY) };
    pointsLeft >>= 1;
    uint32_t a = generateCubicPoints(p0, q[0], r[0], s, tolSqd, points, pointsLeft);
    uint32_t b = generateCubicPoints(s, r[1], q[2], p3, tolSqd, points, pointsLeft);
    return a + b;
}

int GrPathUtils::worstCasePointCount(const SkPath& path, int* subpaths,
                                     GrScalar tol) {
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
    UVpts.setAll(0,   0.5f,  1.f,
                 0,   0,     1.f,
                 1.f, 1.f,   1.f);
    m.setAll(qPts[0].fX, qPts[1].fX, qPts[2].fX,
             qPts[0].fY, qPts[1].fY, qPts[2].fY,
             1.f,        1.f,        1.f);
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
        static const GrScalar gTOL = 1.f / 100.f;
        GrAssert(GrScalarAbs(m.get(SkMatrix::kMPersp0)) < gTOL);
        GrAssert(GrScalarAbs(m.get(SkMatrix::kMPersp1)) < gTOL);

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
void convert_noninflect_cubic_to_quads(const SkPoint p[4],
                                       SkScalar tolScale,
                                       SkTArray<SkPoint, true>* quads,
                                       int sublevel = 0) {
    SkVector ab = p[1];
    ab -= p[0];
    SkVector dc = p[2];
    dc -= p[3];

    static const SkScalar gLengthScale = 3 * SK_Scalar1 / 2;
    // base tolerance is 2 pixels in dev coords.
    const SkScalar distanceSqdTol = SkScalarMul(tolScale, 1 * SK_Scalar1);
    static const int kMaxSubdivs = 10;

    ab.scale(gLengthScale);
    dc.scale(gLengthScale);

    SkVector c0 = p[0];
    c0 += ab;
    SkVector c1 = p[3];
    c1 += dc;

    SkScalar dSqd = c0.distanceToSqd(c1);
    if (sublevel > kMaxSubdivs || dSqd <= distanceSqdTol) {
        SkPoint cAvg = c0;
        cAvg += c1;
        cAvg.scale(SK_ScalarHalf);

        SkPoint* pts = quads->push_back_n(3);
        pts[0] = p[0];
        pts[1] = cAvg;
        pts[2] = p[3];

        return;
    } else {
        SkPoint choppedPts[7];
        SkChopCubicAtHalf(p, choppedPts);
        convert_noninflect_cubic_to_quads(choppedPts + 0, tolScale, 
                                          quads, sublevel + 1);
        convert_noninflect_cubic_to_quads(choppedPts + 3, tolScale,
                                          quads, sublevel + 1);
    }
}
}

void GrPathUtils::convertCubicToQuads(const GrPoint p[4],
                                      SkScalar tolScale,
                                      SkTArray<SkPoint, true>* quads) {
    SkPoint chopped[10];
    int count = SkChopCubicAtInflections(p, chopped);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3*i;
        convert_noninflect_cubic_to_quads(cubic, tolScale, quads);
    }

}
