/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCStrokeGeometry.h"

#include "SkGeometry.h"
#include "SkMathPriv.h"
#include "SkNx.h"
#include "SkStrokeRec.h"

// This is the maximum distance in pixels that we can stray from the edge of a stroke when
// converting it to flat line segments.
static constexpr float kMaxErrorFromLinearization = 1/8.f;

static inline float length(const Sk2f& n) {
    Sk2f nn = n*n;
    return SkScalarSqrt(nn[0] + nn[1]);
}

static inline Sk2f normalize(const Sk2f& v) {
    Sk2f vv = v*v;
    vv += SkNx_shuffle<1,0>(vv);
    return v * vv.rsqrt();
}

static inline void transpose(const Sk2f& a, const Sk2f& b, Sk2f* X, Sk2f* Y) {
    float transpose[4];
    a.store(transpose);
    b.store(transpose+2);
    Sk2f::Load2(transpose, X, Y);
}

static inline void normalize2(const Sk2f& v0, const Sk2f& v1, SkPoint out[2]) {
    Sk2f X, Y;
    transpose(v0, v1, &X, &Y);
    Sk2f invlength = (X*X + Y*Y).rsqrt();
    Sk2f::Store2(out, Y * invlength, -X * invlength);
}

static inline float calc_curvature_costheta(const Sk2f& leftTan, const Sk2f& rightTan) {
    Sk2f X, Y;
    transpose(leftTan, rightTan, &X, &Y);
    Sk2f invlength = (X*X + Y*Y).rsqrt();
    Sk2f dotprod = leftTan * rightTan;
    return (dotprod[0] + dotprod[1]) * invlength[0] * invlength[1];
}

void GrCCStrokeGeometry::beginPath(const SkStrokeRec& stroke, float strokeDevWidth,
                                   InstanceTallies* tallies) {
    SkASSERT(!fInsideContour);
    // Client should have already converted the stroke to device space (i.e. width=1 for hairline).
    SkASSERT(strokeDevWidth > 0);

    fCurrStrokeRadius = strokeDevWidth/2;
    fCurrStrokeJoinType = stroke.getJoin();
    fCurrStrokeCapType = stroke.getCap();
    fCurrStrokeTallies = tallies;

    if (SkPaint::kMiter_Join == fCurrStrokeJoinType) {
        // We implement miters by placing a triangle-shaped cap on top of a bevel join. Convert the
        // "miter limit" to how tall that triangle cap can be.
        float m = stroke.getMiter();
        fMiterMaxCapHeightOverWidth = .5f * SkScalarSqrt(m*m - 1);
    }

    // Find the angle of curvature where the arc height above a simple line from point A to point B
    // is equal to kMaxErrorFromLinearization.
    float r = SkTMax(1 - kMaxErrorFromLinearization / fCurrStrokeRadius, 0.f);
    fMaxCurvatureCosTheta = 2*r*r - 1;

    fCurrContourFirstPtIdx = -1;
    fCurrContourFirstNormalIdx = -1;

    fVerbs.push_back(Verb::kBeginPath);
}

void GrCCStrokeGeometry::moveTo(const SkPoint& pt) {
    SkASSERT(!fInsideContour);
    fCurrContourFirstPtIdx = fPoints.count();
    fCurrContourFirstNormalIdx = fNormals.count();
    fPoints.push_back(pt);
    SkDEBUGCODE(fInsideContour = true);
}

void GrCCStrokeGeometry::lineTo(const SkPoint& pt) {
    SkASSERT(fInsideContour);
    this->lineTo(fCurrStrokeJoinType, pt);
}

void GrCCStrokeGeometry::quadraticTo(const SkPoint P[3]) {
    SkASSERT(fInsideContour);
    this->quadraticTo(fCurrStrokeJoinType, P, SkFindQuadMaxCurvature(P));
}

void GrCCStrokeGeometry::cubicTo(const SkPoint P[4]) {
    SkASSERT(fInsideContour);
    float roots[3];
    int numRoots = SkFindCubicMaxCurvature(P, roots);
    this->cubicTo(fCurrStrokeJoinType, P,
                  numRoots > 0 ? roots[numRoots/2] : 0,
                  numRoots > 1 ? roots[0] : kLeftMaxCurvatureNone,
                  numRoots > 2 ? roots[2] : kRightMaxCurvatureNone);
}

void GrCCStrokeGeometry::rotateTo(SkPaint::Join leftJoinType, const SkVector& normal) {
    this->recordLeftJoinIfNotEmpty(leftJoinType, normal);
    fNormals.push_back(normal);
}

void GrCCStrokeGeometry::closeContour() {
    SkASSERT(fInsideContour);
    SkASSERT(fPoints.count() > fCurrContourFirstPtIdx);
    if (fPoints.back() != fPoints[fCurrContourFirstPtIdx]) {
        // Draw a line back to the beginning.
        SkPoint startPt = fPoints[fCurrContourFirstPtIdx];  // Copy in case the array grows.
        this->lineTo(fCurrStrokeJoinType, startPt);
    }
    if (fNormals.count() > fCurrContourFirstNormalIdx) {
        // Join the first and last lines.
        SkPoint firstNormal = fNormals[fCurrContourFirstNormalIdx];  // Copy in case array grows.
        this->rotateTo(fCurrStrokeJoinType, firstNormal);
    } else {
        // This contour is empty. Add a bogus normal since the iterator always expects one.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        fNormals.push_back({0, 0});
    }
    fVerbs.push_back(Verb::kEndContour);
    SkDEBUGCODE(fInsideContour = false);
}

void GrCCStrokeGeometry::exitContour() {
    SkASSERT(fInsideContour);
    if (fCurrContourFirstNormalIdx >= fNormals.count()) {
        // This contour is empty. Add a normal in the direction that caps orient on empty geometry.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        fNormals.push_back({1, 0});
    }

    if (SkPaint::kButt_Cap != fCurrStrokeCapType) {
        Verb capVerb;
        if (SkPaint::kSquare_Cap == fCurrStrokeCapType) {
            capVerb = Verb::kSquareCap;
            fCurrStrokeTallies->fStrokes[0] += 2;
        } else {
            SkASSERT(SkPaint::kRound_Cap == fCurrStrokeCapType);
            capVerb = Verb::kRoundCap;
            fCurrStrokeTallies->fTriangles += 2;
            fCurrStrokeTallies->fConics += 4;
        }

        fVerbs.push_back(capVerb);
        fVerbs.push_back(Verb::kEndContour);

        fVerbs.push_back(capVerb);
        fPoints.push_back(fPoints[fCurrContourFirstPtIdx]);
        fNormals.push_back(-fNormals[fCurrContourFirstNormalIdx]);
    }
    fVerbs.push_back(Verb::kEndContour);

    SkDEBUGCODE(fInsideContour = false);
}

void GrCCStrokeGeometry::lineTo(SkPaint::Join leftJoinType, const SkPoint& pt) {
    Sk2f tan = Sk2f::Load(&pt) - Sk2f::Load(&fPoints.back());
    if ((tan == 0).allTrue()) {
        return;
    }

    tan = normalize(tan);
    SkVector n = SkVector::Make(tan[1], -tan[0]);

    this->recordLeftJoinIfNotEmpty(leftJoinType, n);
    fNormals.push_back(n);

    this->recordStroke(Verb::kLinearStroke, 0);
    fPoints.push_back(pt);
}

// Wang's formula for quadratics (1985) gives us the number of evenly spaced (in the parametric
// sense) line segments that are guaranteed to be within a distance of "kMaxErrorFromLinearization"
// from the actual curve.
static inline float wangs_formula_quadratic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2) {
    static constexpr float k = 2 / (8 * kMaxErrorFromLinearization);
    float f = SkScalarSqrt(k * length(p2 - p1*2 + p0));
    return SkScalarCeilToInt(f);
}

void GrCCStrokeGeometry::quadraticTo(SkPaint::Join leftJoinType, const SkPoint P[3],
                                     float maxCurvatureT) {
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    // Snap to a "lineTo" if the control point is so close to an endpoint that FP error will become
    // an issue.
    if ((tan0.abs() < SK_ScalarNearlyZero).allTrue() ||  // p0 ~= p1
        (tan1.abs() < SK_ScalarNearlyZero).allTrue()) {  // p1 ~= p2
        this->lineTo(leftJoinType, P[2]);
        return;
    }

    SkPoint normals[2];
    normalize2(tan0, tan1, normals);

    // Decide how many flat line segments to chop the curve into.
    int numSegments = wangs_formula_quadratic(p0, p1, p2);
    if (numSegments <= 1) {
        this->rotateTo(leftJoinType, normals[0]);
        this->lineTo(SkPaint::kRound_Join, P[2]);
        this->rotateTo(SkPaint::kRound_Join, normals[1]);
        return;
    }

    // At + B gives a vector tangent to the quadratic.
    Sk2f A = p0 - p1*2 + p2;
    Sk2f B = p1 - p0;

    // Find a line segment that crosses max curvature.
    float segmentWidth = 1.f/numSegments;
    float leftT = maxCurvatureT - segmentWidth/2;
    float rightT = maxCurvatureT + segmentWidth/2;
    Sk2f leftTan, rightTan;
    if (leftT <= 0) {
        leftT = 0;
        leftTan = tan0;
        rightT = segmentWidth;
        rightTan = A*rightT + B;
    } else if (rightT >= 1) {
        leftT = 1 - segmentWidth;
        leftTan = A*leftT + B;
        rightT = 1;
        rightTan = tan1;
    } else {
        leftTan = A*leftT + B;
        rightTan = A*rightT + B;
    }

    // Check if curvature is too strong for a triangle strip on the line segment that crosses max
    // curvature. If it is, we will chop and convert the segment to a "lineTo" with round joins.
    //
    // FIXME: This is quite costly and the vast majority of curves only have moderate curvature. We
    // would benefit significantly from a quick reject that detects curves that don't need special
    // treatment for strong curvature.
    bool isCurvatureTooStrong = calc_curvature_costheta(leftTan, rightTan) < fMaxCurvatureCosTheta;
    if (isCurvatureTooStrong) {
        SkPoint ptsBuffer[5];
        const SkPoint* currQuadratic = P;

        if (leftT > 0) {
            SkChopQuadAt(currQuadratic, ptsBuffer, leftT);
            this->quadraticTo(leftJoinType, ptsBuffer, /*maxCurvatureT=*/1);
            if (rightT < 1) {
                rightT = (rightT - leftT) / (1 - leftT);
            }
            currQuadratic = ptsBuffer + 2;
        } else {
            this->rotateTo(leftJoinType, normals[0]);
        }

        if (rightT < 1) {
            SkChopQuadAt(currQuadratic, ptsBuffer, rightT);
            this->lineTo(SkPaint::kRound_Join, ptsBuffer[2]);
            this->quadraticTo(SkPaint::kRound_Join, ptsBuffer + 2, /*maxCurvatureT=*/0);
        } else {
            this->lineTo(SkPaint::kRound_Join, currQuadratic[2]);
            this->rotateTo(SkPaint::kRound_Join, normals[1]);
        }
        return;
    }

    this->recordLeftJoinIfNotEmpty(leftJoinType, normals[0]);
    fNormals.push_back_n(2, normals);

    this->recordStroke(Verb::kQuadraticStroke, SkNextLog2(numSegments));
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
}

// Wang's formula for cubics (1985) gives us the number of evenly spaced (in the parametric sense)
// line segments that are guaranteed to be within a distance of "kMaxErrorFromLinearization"
// from the actual curve.
static inline float wangs_formula_cubic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2,
                                        const Sk2f& p3) {
    static constexpr float k = (3 * 2) / (8 * kMaxErrorFromLinearization);
    float f = SkScalarSqrt(k * length(Sk2f::Max((p2 - p1*2 + p0).abs(),
                                                (p3 - p2*2 + p1).abs())));
    return SkScalarCeilToInt(f);
}

void GrCCStrokeGeometry::cubicTo(SkPaint::Join leftJoinType, const SkPoint P[4],
                                 float maxCurvatureT, float leftMaxCurvatureT,
                                 float rightMaxCurvatureT) {
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);
    Sk2f p3 = Sk2f::Load(P+3);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p3 - p2;

    // Snap control points to endpoints if they are so close that FP error will become an issue.
    if ((tan0.abs() < SK_ScalarNearlyZero).allTrue()) {  // p0 ~= p1
        p1 = p0;
        tan0 = p2 - p0;
        if ((tan0.abs() < SK_ScalarNearlyZero).allTrue()) {  // p0 ~= p1 ~= p2
            this->lineTo(leftJoinType, P[3]);
            return;
        }
    }
    if ((tan1.abs() < SK_ScalarNearlyZero).allTrue()) {  // p2 ~= p3
        p2 = p3;
        tan1 = p3 - p1;
        if ((tan1.abs() < SK_ScalarNearlyZero).allTrue() ||  // p1 ~= p2 ~= p3
            (p0 == p1).allTrue()) {  // p0 ~= p1 AND p2 ~= p3
            this->lineTo(leftJoinType, P[3]);
            return;
        }
    }

    SkPoint normals[2];
    normalize2(tan0, tan1, normals);

    // Decide how many flat line segments to chop the curve into.
    int numSegments = wangs_formula_cubic(p0, p1, p2, p3);
    if (numSegments <= 1) {
        this->rotateTo(leftJoinType, normals[0]);
        this->lineTo(leftJoinType, P[3]);
        this->rotateTo(SkPaint::kRound_Join, normals[1]);
        return;
    }

    // At^2 + Bt + C gives a vector tangent to the cubic. (More specifically, it's the derivative
    // minus an irrelevant scale by 3, since all we care about is the direction.)
    Sk2f A = p3 + (p1 - p2)*3 - p0;
    Sk2f B = (p0 - p1*2 + p2)*2;
    Sk2f C = p1 - p0;

    // Find a line segment that crosses max curvature.
    float segmentWidth = 1.f/numSegments;
    float leftT = maxCurvatureT - segmentWidth/2;
    float rightT = maxCurvatureT + segmentWidth/2;
    Sk2f leftTan, rightTan;
    if (leftT <= 0) {
        leftT = 0;
        leftTan = tan0;
        rightT = segmentWidth;
        rightTan = A*rightT*rightT + B*rightT + C;
    } else if (rightT >= 1) {
        leftT = 1 - segmentWidth;
        leftTan = A*leftT*leftT + B*leftT + C;
        rightT = 1;
        rightTan = tan1;
    } else {
        leftTan = A*leftT*leftT + B*leftT + C;
        rightTan = A*rightT*rightT + B*rightT + C;
    }

    // Check if curvature is too strong for a triangle strip on the line segment that crosses max
    // curvature. If it is, we will chop and convert the segment to a "lineTo" with round joins.
    //
    // FIXME: This is quite costly and the vast majority of curves only have moderate curvature. We
    // would benefit significantly from a quick reject that detects curves that don't need special
    // treatment for strong curvature.
    bool isCurvatureTooStrong = calc_curvature_costheta(leftTan, rightTan) < fMaxCurvatureCosTheta;
    if (isCurvatureTooStrong) {
        SkPoint ptsBuffer[7];
        p0.store(ptsBuffer);
        p1.store(ptsBuffer + 1);
        p2.store(ptsBuffer + 2);
        p3.store(ptsBuffer + 3);
        const SkPoint* currCubic = ptsBuffer;

        if (leftT > 0) {
            SkChopCubicAt(currCubic, ptsBuffer, leftT);
            this->cubicTo(leftJoinType, ptsBuffer, /*maxCurvatureT=*/1,
                          (kLeftMaxCurvatureNone != leftMaxCurvatureT)
                                  ? leftMaxCurvatureT/leftT : kLeftMaxCurvatureNone,
                          kRightMaxCurvatureNone);
            if (rightT < 1) {
                rightT = (rightT - leftT) / (1 - leftT);
            }
            if (rightMaxCurvatureT < 1 && kRightMaxCurvatureNone != rightMaxCurvatureT) {
                rightMaxCurvatureT = (rightMaxCurvatureT - leftT) / (1 - leftT);
            }
            currCubic = ptsBuffer + 3;
        } else {
            this->rotateTo(leftJoinType, normals[0]);
        }

        if (rightT < 1) {
            SkChopCubicAt(currCubic, ptsBuffer, rightT);
            this->lineTo(SkPaint::kRound_Join, ptsBuffer[3]);
            currCubic = ptsBuffer + 3;
            this->cubicTo(SkPaint::kRound_Join, currCubic, /*maxCurvatureT=*/0,
                          kLeftMaxCurvatureNone,
                          kRightMaxCurvatureNone);
        } else {
            this->lineTo(SkPaint::kRound_Join, currCubic[3]);
            this->rotateTo(SkPaint::kRound_Join, normals[1]);
        }
        return;
    }

    // Recurse and check the other two points of max curvature, if any.
    if (kRightMaxCurvatureNone != rightMaxCurvatureT) {
        this->cubicTo(leftJoinType, P, rightMaxCurvatureT, leftMaxCurvatureT,
                      kRightMaxCurvatureNone);
        return;
    }
    if (kLeftMaxCurvatureNone != leftMaxCurvatureT) {
        SkASSERT(kRightMaxCurvatureNone == rightMaxCurvatureT);
        this->cubicTo(leftJoinType, P, leftMaxCurvatureT, kLeftMaxCurvatureNone,
                      kRightMaxCurvatureNone);
        return;
    }

    this->recordLeftJoinIfNotEmpty(leftJoinType, normals[0]);
    fNormals.push_back_n(2, normals);

    this->recordStroke(Verb::kCubicStroke, SkNextLog2(numSegments));
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
    p3.store(&fPoints.push_back());
}

void GrCCStrokeGeometry::recordStroke(Verb verb, int numSegmentsLog2) {
    SkASSERT(Verb::kLinearStroke != verb || 0 == numSegmentsLog2);
    SkASSERT(numSegmentsLog2 <= kMaxNumLinearSegmentsLog2);
    fVerbs.push_back(verb);
    if (Verb::kLinearStroke != verb) {
        SkASSERT(numSegmentsLog2 > 0);
        fParams.push_back().fNumLinearSegmentsLog2 = numSegmentsLog2;
    }
    ++fCurrStrokeTallies->fStrokes[numSegmentsLog2];
}

void GrCCStrokeGeometry::recordLeftJoinIfNotEmpty(SkPaint::Join joinType,
                                                  const SkVector& nextNormal) {
    if (fNormals.count() <= fCurrContourFirstNormalIdx) {
        // The contour is empty. Nothing to join with.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        return;
    }

    if (SkPaint::kBevel_Join == joinType) {
        this->recordBevelJoin();
        return;
    }

    Sk2f n0 = Sk2f::Load(&fNormals.back());
    Sk2f n1 = Sk2f::Load(&nextNormal);
    Sk2f base = n1 - n0;
    if ((base.abs() * fCurrStrokeRadius < kMaxErrorFromLinearization).allTrue()) {
        // Treat any join as a bevel when the outside corners of the two adjoining strokes are
        // close enough to each other. This is important because "miterCapHeightOverWidth" becomes
        // unstable when n0 and n1 are nearly equal.
        this->recordBevelJoin();
        return;
    }

    // We implement miters and round joins by placing a triangle-shaped cap on top of a bevel join.
    // (For round joins this triangle cap is the conic control points.) Find how tall to make this
    // triangle cap, relative its width.
    Sk2f cross = base * SkNx_shuffle<1,0>(n0);
    Sk2f dot = base * n0;
    float miterCapHeightOverWidth = .5f * ((dot[0] + dot[1]) / (cross[0] - cross[1]));

    if (SkPaint::kMiter_Join == joinType) {
        if (SkScalarAbs(miterCapHeightOverWidth) > fMiterMaxCapHeightOverWidth) {
            // This join is tighter than the miter limit. Treat it as a bevel.
            this->recordBevelJoin();
            return;
        }
        this->recordMiterJoin(miterCapHeightOverWidth);
        return;
    }

    SkASSERT(SkPaint::kRound_Join == joinType);

    if (SkScalarAbs(miterCapHeightOverWidth) > 10) {
        // It's not possible to create a 180 degree arc with a conic. When the conic control
        // point begins shooting off to infinity, chop the conic in two.
        Sk2f bisect = normalize(n0 - n1);
        this->rotateTo(SkPaint::kRound_Join, SkVector::Make(-bisect[1], bisect[0]));
        this->recordLeftJoinIfNotEmpty(SkPaint::kRound_Join, nextNormal);
        return;
    }

    // Find the heights of this round join's conic control point as well as the arc itself.
    Sk2f X, Y;
    transpose(base * base, n0 * n1, &X, &Y);
    Sk2f r = Sk2f::Max(X + Y + Sk2f(0, 1), 0.f).sqrt();
    Sk2f heights = SkNx_fma(r, Sk2f(miterCapHeightOverWidth, -SK_ScalarRoot2Over2), Sk2f(0, 1));
    float controlPointHeight = SkScalarAbs(heights[0]);
    float curveHeight = heights[1];
    if (curveHeight * fCurrStrokeRadius < kMaxErrorFromLinearization) {
        // Treat round joins as bevels when their curvature is nearly flat.
        this->recordBevelJoin();
        return;
    }

    float w = curveHeight / (controlPointHeight - curveHeight);
    this->recordRoundJoin(miterCapHeightOverWidth, w);
}

void GrCCStrokeGeometry::recordBevelJoin() {
    fVerbs.push_back(Verb::kBevelJoin);
    fCurrStrokeTallies->fTriangles += 2;
}

void GrCCStrokeGeometry::recordMiterJoin(float miterCapHeightOverWidth) {
    fVerbs.push_back(Verb::kMiterJoin);
    fParams.push_back().fMiterCapHeightOverWidth = miterCapHeightOverWidth;
    fCurrStrokeTallies->fTriangles += 3;
}

void GrCCStrokeGeometry::recordRoundJoin(float miterCapHeightOverWidth, float conicWeight) {
    fVerbs.push_back(Verb::kRoundJoin);
    fParams.push_back().fConicWeight = conicWeight;
    fParams.push_back().fMiterCapHeightOverWidth = miterCapHeightOverWidth;
    fCurrStrokeTallies->fTriangles += 2;
    ++fCurrStrokeTallies->fConics;
}
