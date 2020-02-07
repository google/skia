/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCStrokeGeometry.h"

#include "include/core/SkStrokeRec.h"
#include "include/private/SkNx.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMathPriv.h"

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

static GrCCStrokeGeometry::Verb join_verb_from_join(SkPaint::Join join) {
    using Verb = GrCCStrokeGeometry::Verb;
    switch (join) {
        case SkPaint::kBevel_Join:
            return Verb::kBevelJoin;
        case SkPaint::kMiter_Join:
            return Verb::kMiterJoin;
        case SkPaint::kRound_Join:
            return Verb::kRoundJoin;
    }
    SK_ABORT("Invalid SkPaint::Join.");
}

void GrCCStrokeGeometry::beginPath(const SkStrokeRec& stroke, float strokeDevWidth,
                                   InstanceTallies* tallies) {
    SkASSERT(!fInsideContour);
    // Client should have already converted the stroke to device space (i.e. width=1 for hairline).
    SkASSERT(strokeDevWidth > 0);

    fCurrStrokeRadius = strokeDevWidth/2;
    fCurrStrokeJoinVerb = join_verb_from_join(stroke.getJoin());
    fCurrStrokeCapType = stroke.getCap();
    fCurrStrokeTallies = tallies;

    if (Verb::kMiterJoin == fCurrStrokeJoinVerb) {
        // We implement miters by placing a triangle-shaped cap on top of a bevel join. Convert the
        // "miter limit" to how tall that triangle cap can be.
        float m = stroke.getMiter();
        fMiterMaxCapHeightOverWidth = .5f * SkScalarSqrt(m*m - 1);
    }

    // Find the angle of curvature where the arc height above a simple line from point A to point B
    // is equal to kMaxErrorFromLinearization.
    float r = std::max(1 - kMaxErrorFromLinearization / fCurrStrokeRadius, 0.f);
    fMaxCurvatureCosTheta = 2*r*r - 1;

    fCurrContourFirstPtIdx = -1;
    fCurrContourFirstNormalIdx = -1;

    fVerbs.push_back(Verb::kBeginPath);
}

void GrCCStrokeGeometry::moveTo(SkPoint pt) {
    SkASSERT(!fInsideContour);
    fCurrContourFirstPtIdx = fPoints.count();
    fCurrContourFirstNormalIdx = fNormals.count();
    fPoints.push_back(pt);
    SkDEBUGCODE(fInsideContour = true);
}

void GrCCStrokeGeometry::lineTo(SkPoint pt) {
    SkASSERT(fInsideContour);
    this->lineTo(fCurrStrokeJoinVerb, pt);
}

void GrCCStrokeGeometry::lineTo(Verb leftJoinVerb, SkPoint pt) {
    Sk2f tan = Sk2f::Load(&pt) - Sk2f::Load(&fPoints.back());
    if ((tan == 0).allTrue()) {
        return;
    }

    tan = normalize(tan);
    SkVector n = SkVector::Make(tan[1], -tan[0]);

    this->recordLeftJoinIfNotEmpty(leftJoinVerb, n);
    fNormals.push_back(n);

    this->recordStroke(Verb::kLinearStroke, 0);
    fPoints.push_back(pt);
}

void GrCCStrokeGeometry::quadraticTo(const SkPoint P[3]) {
    SkASSERT(fInsideContour);
    this->quadraticTo(fCurrStrokeJoinVerb, P, SkFindQuadMaxCurvature(P));
}

// Wang's formula for quadratics (1985) gives us the number of evenly spaced (in the parametric
// sense) line segments that are guaranteed to be within a distance of "kMaxErrorFromLinearization"
// from the actual curve.
static inline float wangs_formula_quadratic(const Sk2f& p0, const Sk2f& p1, const Sk2f& p2) {
    static constexpr float k = 2 / (8 * kMaxErrorFromLinearization);
    float f = SkScalarSqrt(k * length(p2 - p1*2 + p0));
    return SkScalarCeilToInt(f);
}

void GrCCStrokeGeometry::quadraticTo(Verb leftJoinVerb, const SkPoint P[3], float maxCurvatureT) {
    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    // Snap to a "lineTo" if the control point is so close to an endpoint that FP error will become
    // an issue.
    if ((tan0.abs() < SK_ScalarNearlyZero).allTrue() ||  // p0 ~= p1
        (tan1.abs() < SK_ScalarNearlyZero).allTrue()) {  // p1 ~= p2
        this->lineTo(leftJoinVerb, P[2]);
        return;
    }

    SkPoint normals[2];
    normalize2(tan0, tan1, normals);

    // Decide how many flat line segments to chop the curve into.
    int numSegments = wangs_formula_quadratic(p0, p1, p2);
    numSegments = std::min(numSegments, 1 << kMaxNumLinearSegmentsLog2);
    if (numSegments <= 1) {
        this->rotateTo(leftJoinVerb, normals[0]);
        this->lineTo(Verb::kInternalRoundJoin, P[2]);
        this->rotateTo(Verb::kInternalRoundJoin, normals[1]);
        return;
    }

    // At + B gives a vector tangent to the quadratic.
    Sk2f A = p0 - p1*2 + p2;
    Sk2f B = p1 - p0;

    // Find a line segment that crosses max curvature.
    float segmentLength = SkScalarInvert(numSegments);
    float leftT = maxCurvatureT - segmentLength/2;
    float rightT = maxCurvatureT + segmentLength/2;
    Sk2f leftTan, rightTan;
    if (leftT <= 0) {
        leftT = 0;
        leftTan = tan0;
        rightT = segmentLength;
        rightTan = A*rightT + B;
    } else if (rightT >= 1) {
        leftT = 1 - segmentLength;
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
            this->quadraticTo(leftJoinVerb, ptsBuffer, /*maxCurvatureT=*/1);
            if (rightT < 1) {
                rightT = (rightT - leftT) / (1 - leftT);
            }
            currQuadratic = ptsBuffer + 2;
        } else {
            this->rotateTo(leftJoinVerb, normals[0]);
        }

        if (rightT < 1) {
            SkChopQuadAt(currQuadratic, ptsBuffer, rightT);
            this->lineTo(Verb::kInternalRoundJoin, ptsBuffer[2]);
            this->quadraticTo(Verb::kInternalRoundJoin, ptsBuffer + 2, /*maxCurvatureT=*/0);
        } else {
            this->lineTo(Verb::kInternalRoundJoin, currQuadratic[2]);
            this->rotateTo(Verb::kInternalRoundJoin, normals[1]);
        }
        return;
    }

    this->recordLeftJoinIfNotEmpty(leftJoinVerb, normals[0]);
    fNormals.push_back_n(2, normals);

    this->recordStroke(Verb::kQuadraticStroke, SkNextLog2(numSegments));
    p1.store(&fPoints.push_back());
    p2.store(&fPoints.push_back());
}

void GrCCStrokeGeometry::cubicTo(const SkPoint P[4]) {
    SkASSERT(fInsideContour);
    float roots[3];
    int numRoots = SkFindCubicMaxCurvature(P, roots);
    this->cubicTo(fCurrStrokeJoinVerb, P,
                  numRoots > 0 ? roots[numRoots/2] : 0,
                  numRoots > 1 ? roots[0] : kLeftMaxCurvatureNone,
                  numRoots > 2 ? roots[2] : kRightMaxCurvatureNone);
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

void GrCCStrokeGeometry::cubicTo(Verb leftJoinVerb, const SkPoint P[4], float maxCurvatureT,
                                 float leftMaxCurvatureT, float rightMaxCurvatureT) {
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
            this->lineTo(leftJoinVerb, P[3]);
            return;
        }
    }
    if ((tan1.abs() < SK_ScalarNearlyZero).allTrue()) {  // p2 ~= p3
        p2 = p3;
        tan1 = p3 - p1;
        if ((tan1.abs() < SK_ScalarNearlyZero).allTrue() ||  // p1 ~= p2 ~= p3
            (p0 == p1).allTrue()) {  // p0 ~= p1 AND p2 ~= p3
            this->lineTo(leftJoinVerb, P[3]);
            return;
        }
    }

    SkPoint normals[2];
    normalize2(tan0, tan1, normals);

    // Decide how many flat line segments to chop the curve into.
    int numSegments = wangs_formula_cubic(p0, p1, p2, p3);
    numSegments = std::min(numSegments, 1 << kMaxNumLinearSegmentsLog2);
    if (numSegments <= 1) {
        this->rotateTo(leftJoinVerb, normals[0]);
        this->lineTo(leftJoinVerb, P[3]);
        this->rotateTo(Verb::kInternalRoundJoin, normals[1]);
        return;
    }

    // At^2 + Bt + C gives a vector tangent to the cubic. (More specifically, it's the derivative
    // minus an irrelevant scale by 3, since all we care about is the direction.)
    Sk2f A = p3 + (p1 - p2)*3 - p0;
    Sk2f B = (p0 - p1*2 + p2)*2;
    Sk2f C = p1 - p0;

    // Find a line segment that crosses max curvature.
    float segmentLength = SkScalarInvert(numSegments);
    float leftT = maxCurvatureT - segmentLength/2;
    float rightT = maxCurvatureT + segmentLength/2;
    Sk2f leftTan, rightTan;
    if (leftT <= 0) {
        leftT = 0;
        leftTan = tan0;
        rightT = segmentLength;
        rightTan = A*rightT*rightT + B*rightT + C;
    } else if (rightT >= 1) {
        leftT = 1 - segmentLength;
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
            this->cubicTo(leftJoinVerb, ptsBuffer, /*maxCurvatureT=*/1,
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
            this->rotateTo(leftJoinVerb, normals[0]);
        }

        if (rightT < 1) {
            SkChopCubicAt(currCubic, ptsBuffer, rightT);
            this->lineTo(Verb::kInternalRoundJoin, ptsBuffer[3]);
            currCubic = ptsBuffer + 3;
            this->cubicTo(Verb::kInternalRoundJoin, currCubic, /*maxCurvatureT=*/0,
                          kLeftMaxCurvatureNone, kRightMaxCurvatureNone);
        } else {
            this->lineTo(Verb::kInternalRoundJoin, currCubic[3]);
            this->rotateTo(Verb::kInternalRoundJoin, normals[1]);
        }
        return;
    }

    // Recurse and check the other two points of max curvature, if any.
    if (kRightMaxCurvatureNone != rightMaxCurvatureT) {
        this->cubicTo(leftJoinVerb, P, rightMaxCurvatureT, leftMaxCurvatureT,
                      kRightMaxCurvatureNone);
        return;
    }
    if (kLeftMaxCurvatureNone != leftMaxCurvatureT) {
        SkASSERT(kRightMaxCurvatureNone == rightMaxCurvatureT);
        this->cubicTo(leftJoinVerb, P, leftMaxCurvatureT, kLeftMaxCurvatureNone,
                      kRightMaxCurvatureNone);
        return;
    }

    this->recordLeftJoinIfNotEmpty(leftJoinVerb, normals[0]);
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

void GrCCStrokeGeometry::rotateTo(Verb leftJoinVerb, SkVector normal) {
    this->recordLeftJoinIfNotEmpty(leftJoinVerb, normal);
    fNormals.push_back(normal);
}

void GrCCStrokeGeometry::recordLeftJoinIfNotEmpty(Verb joinVerb, SkVector nextNormal) {
    if (fNormals.count() <= fCurrContourFirstNormalIdx) {
        // The contour is empty. Nothing to join with.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        return;
    }

    if (Verb::kBevelJoin == joinVerb) {
        this->recordBevelJoin(Verb::kBevelJoin);
        return;
    }

    Sk2f n0 = Sk2f::Load(&fNormals.back());
    Sk2f n1 = Sk2f::Load(&nextNormal);
    Sk2f base = n1 - n0;
    if ((base.abs() * fCurrStrokeRadius < kMaxErrorFromLinearization).allTrue()) {
        // Treat any join as a bevel when the outside corners of the two adjoining strokes are
        // close enough to each other. This is important because "miterCapHeightOverWidth" becomes
        // unstable when n0 and n1 are nearly equal.
        this->recordBevelJoin(joinVerb);
        return;
    }

    // We implement miters and round joins by placing a triangle-shaped cap on top of a bevel join.
    // (For round joins this triangle cap comprises the conic control points.) Find how tall to make
    // this triangle cap, relative to its width.
    //
    // NOTE: This value would be infinite at 180 degrees, but we clamp miterCapHeightOverWidth at
    // near-infinity. 180-degree round joins still look perfectly acceptable like this (though
    // technically not pure arcs).
    Sk2f cross = base * SkNx_shuffle<1,0>(n0);
    Sk2f dot = base * n0;
    float miterCapHeight = SkScalarAbs(dot[0] + dot[1]);
    float miterCapWidth = SkScalarAbs(cross[0] - cross[1]) * 2;

    if (Verb::kMiterJoin == joinVerb) {
        if (miterCapHeight > fMiterMaxCapHeightOverWidth * miterCapWidth) {
            // This join is tighter than the miter limit. Treat it as a bevel.
            this->recordBevelJoin(Verb::kMiterJoin);
            return;
        }
        this->recordMiterJoin(miterCapHeight / miterCapWidth);
        return;
    }

    SkASSERT(Verb::kRoundJoin == joinVerb || Verb::kInternalRoundJoin == joinVerb);

    // Conic arcs become unstable when they approach 180 degrees. When the conic control point
    // begins shooting off to infinity (i.e., height/width > 32), split the conic into two.
    static constexpr float kAlmost180Degrees = 32;
    if (miterCapHeight > kAlmost180Degrees * miterCapWidth) {
        Sk2f bisect = normalize(n0 - n1);
        this->rotateTo(joinVerb, SkVector::Make(-bisect[1], bisect[0]));
        this->recordLeftJoinIfNotEmpty(joinVerb, nextNormal);
        return;
    }

    float miterCapHeightOverWidth = miterCapHeight / miterCapWidth;

    // Find the heights of this round join's conic control point as well as the arc itself.
    Sk2f X, Y;
    transpose(base * base, n0 * n1, &X, &Y);
    Sk2f r = Sk2f::Max(X + Y + Sk2f(0, 1), 0.f).sqrt();
    Sk2f heights = SkNx_fma(r, Sk2f(miterCapHeightOverWidth, -SK_ScalarRoot2Over2), Sk2f(0, 1));
    float controlPointHeight = SkScalarAbs(heights[0]);
    float curveHeight = heights[1];
    if (curveHeight * fCurrStrokeRadius < kMaxErrorFromLinearization) {
        // Treat round joins as bevels when their curvature is nearly flat.
        this->recordBevelJoin(joinVerb);
        return;
    }

    float w = curveHeight / (controlPointHeight - curveHeight);
    this->recordRoundJoin(joinVerb, miterCapHeightOverWidth, w);
}

void GrCCStrokeGeometry::recordBevelJoin(Verb originalJoinVerb) {
    if (!IsInternalJoinVerb(originalJoinVerb)) {
        fVerbs.push_back(Verb::kBevelJoin);
        ++fCurrStrokeTallies->fTriangles;
    } else {
        fVerbs.push_back(Verb::kInternalBevelJoin);
        fCurrStrokeTallies->fTriangles += 2;
    }
}

void GrCCStrokeGeometry::recordMiterJoin(float miterCapHeightOverWidth) {
    fVerbs.push_back(Verb::kMiterJoin);
    fParams.push_back().fMiterCapHeightOverWidth = miterCapHeightOverWidth;
    fCurrStrokeTallies->fTriangles += 2;
}

void GrCCStrokeGeometry::recordRoundJoin(Verb joinVerb, float miterCapHeightOverWidth,
                                         float conicWeight) {
    fVerbs.push_back(joinVerb);
    fParams.push_back().fConicWeight = conicWeight;
    fParams.push_back().fMiterCapHeightOverWidth = miterCapHeightOverWidth;
    if (Verb::kRoundJoin == joinVerb) {
        ++fCurrStrokeTallies->fTriangles;
        ++fCurrStrokeTallies->fConics;
    } else {
        SkASSERT(Verb::kInternalRoundJoin == joinVerb);
        fCurrStrokeTallies->fTriangles += 2;
        fCurrStrokeTallies->fConics += 2;
    }
}

void GrCCStrokeGeometry::closeContour() {
    SkASSERT(fInsideContour);
    SkASSERT(fPoints.count() > fCurrContourFirstPtIdx);
    if (fPoints.back() != fPoints[fCurrContourFirstPtIdx]) {
        // Draw a line back to the beginning.
        this->lineTo(fCurrStrokeJoinVerb, fPoints[fCurrContourFirstPtIdx]);
    }
    if (fNormals.count() > fCurrContourFirstNormalIdx) {
        // Join the first and last lines.
        this->rotateTo(fCurrStrokeJoinVerb,fNormals[fCurrContourFirstNormalIdx]);
    } else {
        // This contour is empty. Add a bogus normal since the iterator always expects one.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        fNormals.push_back({0, 0});
    }
    fVerbs.push_back(Verb::kEndContour);
    SkDEBUGCODE(fInsideContour = false);
}

void GrCCStrokeGeometry::capContourAndExit() {
    SkASSERT(fInsideContour);
    if (fCurrContourFirstNormalIdx >= fNormals.count()) {
        // This contour is empty. Add a normal in the direction that caps orient on empty geometry.
        SkASSERT(fNormals.count() == fCurrContourFirstNormalIdx);
        fNormals.push_back({1, 0});
    }

    this->recordCapsIfAny();
    fVerbs.push_back(Verb::kEndContour);

    SkDEBUGCODE(fInsideContour = false);
}

void GrCCStrokeGeometry::recordCapsIfAny() {
    SkASSERT(fInsideContour);
    SkASSERT(fCurrContourFirstNormalIdx < fNormals.count());

    if (SkPaint::kButt_Cap == fCurrStrokeCapType) {
        return;
    }

    Verb capVerb;
    if (SkPaint::kSquare_Cap == fCurrStrokeCapType) {
        if (fCurrStrokeRadius * SK_ScalarRoot2Over2 < kMaxErrorFromLinearization) {
            return;
        }
        capVerb = Verb::kSquareCap;
        fCurrStrokeTallies->fStrokes[0] += 2;
    } else {
        SkASSERT(SkPaint::kRound_Cap == fCurrStrokeCapType);
        if (fCurrStrokeRadius < kMaxErrorFromLinearization) {
            return;
        }
        capVerb = Verb::kRoundCap;
        fCurrStrokeTallies->fTriangles += 2;
        fCurrStrokeTallies->fConics += 4;
    }

    fVerbs.push_back(capVerb);
    fVerbs.push_back(Verb::kEndContour);

    fVerbs.push_back(capVerb);

    // Reserve the space first, since push_back() takes the point by reference and might
    // invalidate the reference if the array grows.
    fPoints.reserve(fPoints.count() + 1);
    fPoints.push_back(fPoints[fCurrContourFirstPtIdx]);

    // Reserve the space first, since push_back() takes the normal by reference and might
    // invalidate the reference if the array grows. (Although in this case we should be fine
    // since there is a negate operator.)
    fNormals.reserve(fNormals.count() + 1);
    fNormals.push_back(-fNormals[fCurrContourFirstNormalIdx]);
}
