/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokePatchBuilder.h"

#include "include/core/SkStrokeRec.h"
#include "include/private/SkNx.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrVectorXform.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

// This is the maximum distance in pixels that we can stray from the edge of a stroke when
// converting it to flat line segments.
static constexpr float kLinearizationIntolerance = 8;  // 1/8 pixel.

constexpr static float kInternalRoundJoinType = GrStrokeTessellateShader::kInternalRoundJoinType;

static Sk2f lerp(const Sk2f& a, const Sk2f& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static inline void transpose(const Sk2f& a, const Sk2f& b, Sk2f* X, Sk2f* Y) {
    float transpose[4];
    a.store(transpose);
    b.store(transpose+2);
    Sk2f::Load2(transpose, X, Y);
}

static inline float calc_curvature_costheta(const Sk2f& leftTan, const Sk2f& rightTan) {
    Sk2f X, Y;
    transpose(leftTan, rightTan, &X, &Y);
    Sk2f invlength = (X*X + Y*Y).rsqrt();
    Sk2f dotprod = leftTan * rightTan;
    return (dotprod[0] + dotprod[1]) * invlength[0] * invlength[1];
}

void GrStrokePatchBuilder::allocVertexChunk(int minVertexAllocCount) {
    VertexChunk* chunk = &fVertexChunkArray->push_back();
    fCurrChunkVertexData = (SkPoint*)fTarget->makeVertexSpaceAtLeast(
            sizeof(SkPoint), minVertexAllocCount, minVertexAllocCount, &chunk->fVertexBuffer,
            &chunk->fBaseVertex, &fCurrChunkVertexCapacity);
    fCurrChunkMinVertexAllocCount = minVertexAllocCount;
}

SkPoint* GrStrokePatchBuilder::reservePatch() {
    constexpr static int kNumVerticesPerPatch = GrStrokeTessellateShader::kNumVerticesPerPatch;
    if (fVertexChunkArray->back().fVertexCount + kNumVerticesPerPatch > fCurrChunkVertexCapacity) {
        // The current chunk is full. Time to allocate a new one. (And no need to put back vertices;
        // the buffer is full.)
        this->allocVertexChunk(fCurrChunkMinVertexAllocCount * 2);
    }
    if (!fCurrChunkVertexData) {
        SkDebugf("WARNING: Failed to allocate vertex buffer for tessellated stroke.");
        return nullptr;
    }
    SkASSERT(fVertexChunkArray->back().fVertexCount + kNumVerticesPerPatch <=
             fCurrChunkVertexCapacity);
    SkPoint* patch = fCurrChunkVertexData + fVertexChunkArray->back().fVertexCount;
    fVertexChunkArray->back().fVertexCount += kNumVerticesPerPatch;
    return patch;
}

void GrStrokePatchBuilder::writeCubicSegment(float leftJoinType, const SkPoint pts[4],
                                             float overrideNumSegments) {
    SkPoint c1 = (pts[1] == pts[0]) ? pts[2] : pts[1];
    SkPoint c2 = (pts[2] == pts[3]) ? pts[1] : pts[2];

    if (fHasPreviousSegment) {
        this->writeJoin(leftJoinType, pts[0], fLastControlPoint, c1);
    } else {
        fCurrContourFirstControlPoint = c1;
        fHasPreviousSegment = true;
    }

    if (SkPoint* patch = this->reservePatch()) {
        memcpy(patch, pts, sizeof(SkPoint) * 4);
        patch[4].set(overrideNumSegments, fCurrStrokeRadius);
    }

    fLastControlPoint = c2;
    fCurrentPoint = pts[3];
}

void GrStrokePatchBuilder::writeJoin(float joinType, const SkPoint& anchorPoint,
                                     const SkPoint& prevControlPoint,
                                     const SkPoint& nextControlPoint) {
    if (SkPoint* joinPatch = this->reservePatch()) {
        joinPatch[0] = anchorPoint;
        joinPatch[1] = prevControlPoint;
        joinPatch[2] = nextControlPoint;
        joinPatch[3] = anchorPoint;
        joinPatch[4].set(joinType, fCurrStrokeRadius);
    }
}

void GrStrokePatchBuilder::writeSquareCap(const SkPoint& endPoint, const SkPoint& controlPoint) {
    SkVector v = (endPoint - controlPoint);
    v.normalize();
    SkPoint capPoint = endPoint + v*fCurrStrokeRadius;
    // Construct a line that incorporates controlPoint so we get a water tight edge with the rest of
    // the stroke. The cubic will technically step outside the cap, but we will force it to only
    // have one segment, giving edges only at the endpoints.
    if (SkPoint* capPatch = this->reservePatch()) {
        capPatch[0] = endPoint;
        capPatch[1] = controlPoint;
        // Straddle the midpoint of the cap because the tessellated geometry emits a center point at
        // T=.5, and we need to ensure that point stays inside the cap.
        capPatch[2] = endPoint + capPoint - controlPoint;
        capPatch[3] = capPoint;
        capPatch[4].set(1, fCurrStrokeRadius);
    }
}

void GrStrokePatchBuilder::writeCaps() {
    if (!fHasPreviousSegment) {
        // We don't have any control points to orient the caps. In this case, square and round caps
        // are specified to be drawn as an axis-aligned square or circle respectively. Assign
        // default control points that achieve this.
        fCurrContourFirstControlPoint = fCurrContourStartPoint - SkPoint{1,0};
        fLastControlPoint = fCurrContourStartPoint + SkPoint{1,0};
        fCurrentPoint = fCurrContourStartPoint;
    }

    switch (fCurrStrokeCapType) {
        case SkPaint::kButt_Cap:
            break;
        case SkPaint::kRound_Cap:
            // A round cap is the same thing as a 180-degree round join.
            this->writeJoin(GrStrokeTessellateShader::kRoundJoinType, fCurrContourStartPoint,
                            fCurrContourFirstControlPoint, fCurrContourFirstControlPoint);
            this->writeJoin(GrStrokeTessellateShader::kRoundJoinType, fCurrentPoint,
                            fLastControlPoint, fLastControlPoint);
            break;
        case SkPaint::kSquare_Cap:
            this->writeSquareCap(fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->writeSquareCap(fCurrentPoint, fLastControlPoint);
            break;
    }
}

void GrStrokePatchBuilder::addPath(const SkPath& path, const SkStrokeRec& stroke) {
    this->beginPath(stroke);
    SkPathVerb previousVerb = SkPathVerb::kClose;
    for (auto [verb, rawPts, w] : SkPathPriv::Iterate(path)) {
        SkPoint pts[4];
        int numPtsInVerb = SkPathPriv::PtsInIter((unsigned)verb);
        for (int i = 0; i < numPtsInVerb; ++i) {
            // TEMPORORY: Scale all the points up front. SkFind*MaxCurvature and GrWangsFormula::*
            // both expect arrays of points. As we refine this class and its math, this scale will
            // hopefully be integrated more efficiently.
            pts[i] = rawPts[i] * fMatrixScale;
        }
        switch (verb) {
            case SkPathVerb::kMove:
                // "A subpath ... consisting of a single moveto shall not be stroked."
                // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
                    this->writeCaps();
                }
                this->moveTo(pts[0]);
                break;
            case SkPathVerb::kClose:
                this->close();
                break;
            case SkPathVerb::kLine:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->lineTo(pts[0], pts[1]);
                break;
            case SkPathVerb::kQuad:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->quadraticTo(pts);
                break;
            case SkPathVerb::kCubic:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->cubicTo(pts);
                break;
            case SkPathVerb::kConic:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                SkUNREACHABLE;
        }
        previousVerb = verb;
    }
    if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
        this->writeCaps();
    }
}

static float join_type_from_join(SkPaint::Join join) {
    switch (join) {
        case SkPaint::kBevel_Join:
            return GrStrokeTessellateShader::kBevelJoinType;
        case SkPaint::kMiter_Join:
            return GrStrokeTessellateShader::kMiterJoinType;
        case SkPaint::kRound_Join:
            return GrStrokeTessellateShader::kRoundJoinType;
    }
    SkUNREACHABLE;
}

void GrStrokePatchBuilder::beginPath(const SkStrokeRec& stroke) {
    // We don't support hairline strokes. For now, the client can transform the path into device
    // space and then use a stroke width of 1.
    SkASSERT(stroke.getWidth() > 0);

    fCurrStrokeRadius = stroke.getWidth()/2 * fMatrixScale;
    fCurrStrokeJoinType = join_type_from_join(stroke.getJoin());
    fCurrStrokeCapType = stroke.getCap();

    // Find the angle of curvature where the arc height above a simple line from point A to point B
    // is equal to 1/kLinearizationIntolerance. (The arc height is always the same no matter how
    // long the line is. What we are interested in is the difference in height between the part of
    // the stroke whose normal is orthogonal to the line, vs the heights at the endpoints.)
    float r = std::max(1 - 1/(fCurrStrokeRadius * kLinearizationIntolerance), 0.f);
    fMaxCurvatureCosTheta = 2*r*r - 1;

    fHasPreviousSegment = false;
}

void GrStrokePatchBuilder::moveTo(const SkPoint& pt) {
    fHasPreviousSegment = false;
    fCurrContourStartPoint = pt;
}

void GrStrokePatchBuilder::lineTo(const SkPoint& p0, const SkPoint& p1) {
    this->lineTo(fCurrStrokeJoinType, p0, p1);
}

void GrStrokePatchBuilder::lineTo(float leftJoinType, const SkPoint& pt0, const SkPoint& pt1) {
    Sk2f p0 = Sk2f::Load(&pt0);
    Sk2f p1 = Sk2f::Load(&pt1);
    if ((p0 == p1).allTrue()) {
        return;
    }
    this->writeCubicSegment(leftJoinType, p0, lerp(p0, p1, 1/3.f), lerp(p0, p1, 2/3.f), p1, 1);
}

void GrStrokePatchBuilder::quadraticTo(const SkPoint P[3]) {
    this->quadraticTo(fCurrStrokeJoinType, P, SkFindQuadMaxCurvature(P));
}

void GrStrokePatchBuilder::quadraticTo(float leftJoinType, const SkPoint P[3],
                                       float maxCurvatureT) {
    if (P[1] == P[0] || P[1] == P[2]) {
        this->lineTo(leftJoinType, P[0], P[2]);
        return;
    }

    // Decide a lower bound on the length (in parametric sense) of linear segments the curve will be
    // chopped into.
    int numSegments = 1 << GrWangsFormula::quadratic_log2(kLinearizationIntolerance, P);
    float segmentLength = SkScalarInvert(numSegments);

    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;

    // At + B gives a vector tangent to the quadratic.
    Sk2f A = p0 - p1*2 + p2;
    Sk2f B = p1 - p0;

    // Find a line segment that crosses max curvature.
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
    if (numSegments > 1 && calc_curvature_costheta(leftTan, rightTan) < fMaxCurvatureCosTheta) {
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
            this->rotateTo(leftJoinType, currQuadratic[0], currQuadratic[1]);
        }

        if (rightT < 1) {
            SkChopQuadAt(currQuadratic, ptsBuffer, rightT);
            this->lineTo(kInternalRoundJoinType, ptsBuffer[0], ptsBuffer[2]);
            this->quadraticTo(kInternalRoundJoinType, ptsBuffer + 2, /*maxCurvatureT=*/0);
        } else {
            this->lineTo(kInternalRoundJoinType, currQuadratic[0], currQuadratic[2]);
            this->rotateTo(kInternalRoundJoinType, currQuadratic[2],
                           currQuadratic[2]*2 - currQuadratic[1]);
        }
        return;
    }
    if (numSegments > fMaxTessellationSegments) {
        SkPoint ptsBuffer[5];
        SkChopQuadAt(P, ptsBuffer, 0.5f);
        this->quadraticTo(leftJoinType, ptsBuffer, 0);
        this->quadraticTo(kInternalRoundJoinType, ptsBuffer + 3, 0);
        return;
    }

    this->writeCubicSegment(leftJoinType, p0, lerp(p0, p1, 2/3.f), lerp(p1, p2, 1/3.f), p2);
}

void GrStrokePatchBuilder::cubicTo(const SkPoint P[4]) {
    float roots[3];
    int numRoots = SkFindCubicMaxCurvature(P, roots);
    this->cubicTo(fCurrStrokeJoinType, P,
                  numRoots > 0 ? roots[numRoots/2] : 0,
                  numRoots > 1 ? roots[0] : kLeftMaxCurvatureNone,
                  numRoots > 2 ? roots[2] : kRightMaxCurvatureNone);
}

void GrStrokePatchBuilder::cubicTo(float leftJoinType, const SkPoint P[4], float maxCurvatureT,
                                   float leftMaxCurvatureT, float rightMaxCurvatureT) {
    if (P[1] == P[2] && (P[1] == P[0] || P[1] == P[3])) {
        this->lineTo(leftJoinType, P[0], P[3]);
        return;
    }

    // Decide a lower bound on the length (in parametric sense) of linear segments the curve will be
    // chopped into.
    int numSegments = 1 << GrWangsFormula::cubic_log2(kLinearizationIntolerance, P);
    float segmentLength = SkScalarInvert(numSegments);

    Sk2f p0 = Sk2f::Load(P);
    Sk2f p1 = Sk2f::Load(P+1);
    Sk2f p2 = Sk2f::Load(P+2);
    Sk2f p3 = Sk2f::Load(P+3);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p3 - p2;

    // At^2 + Bt + C gives a vector tangent to the cubic. (More specifically, it's the derivative
    // minus an irrelevant scale by 3, since all we care about is the direction.)
    Sk2f A = p3 + (p1 - p2)*3 - p0;
    Sk2f B = (p0 - p1*2 + p2)*2;
    Sk2f C = p1 - p0;

    // Find a line segment that crosses max curvature.
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
    if (numSegments > 1 && calc_curvature_costheta(leftTan, rightTan) < fMaxCurvatureCosTheta) {
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
            SkPoint c1 = (ptsBuffer[1] == ptsBuffer[0]) ? ptsBuffer[2] : ptsBuffer[1];
            this->rotateTo(leftJoinType, ptsBuffer[0], c1);
        }

        if (rightT < 1) {
            SkChopCubicAt(currCubic, ptsBuffer, rightT);
            this->lineTo(kInternalRoundJoinType, ptsBuffer[0], ptsBuffer[3]);
            currCubic = ptsBuffer + 3;
            this->cubicTo(kInternalRoundJoinType, currCubic, /*maxCurvatureT=*/0,
                          kLeftMaxCurvatureNone, kRightMaxCurvatureNone);
        } else {
            this->lineTo(kInternalRoundJoinType, currCubic[0], currCubic[3]);
            SkPoint c2 = (currCubic[2] == currCubic[3]) ? currCubic[1] : currCubic[2];
            this->rotateTo(kInternalRoundJoinType, currCubic[3], currCubic[3]*2 - c2);
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
    if (numSegments > fMaxTessellationSegments) {
        SkPoint ptsBuffer[7];
        SkChopCubicAt(P, ptsBuffer, 0.5f);
        this->cubicTo(leftJoinType, ptsBuffer, 0, kLeftMaxCurvatureNone, kRightMaxCurvatureNone);
        this->cubicTo(kInternalRoundJoinType, ptsBuffer + 3, 0, kLeftMaxCurvatureNone,
                      kRightMaxCurvatureNone);
        return;
    }

    this->writeCubicSegment(leftJoinType, p0, p1, p2, p3);
}

void GrStrokePatchBuilder::rotateTo(float leftJoinType, const SkPoint& anchorPoint,
                                    const SkPoint& controlPoint) {
    // Effectively rotate the current normal by drawing a zero length, 1-segment cubic.
    // writeCubicSegment automatically adds the necessary join and the zero length cubic serves as
    // a glue that guarantees a water tight rasterized edge between the new join and the segment
    // that comes after the rotate.
    SkPoint pts[4] = {anchorPoint, controlPoint, anchorPoint*2 - controlPoint, anchorPoint};
    this->writeCubicSegment(leftJoinType, pts, 1);
}

void GrStrokePatchBuilder::close() {
    if (!fHasPreviousSegment) {
        // Draw caps instead of closing if the subpath is zero length:
        //
        //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has a
        //   value of round or square producing respectively a circle or a square."
        //
        //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
        //
        this->writeCaps();
        return;
    }

    // Draw a line back to the beginning. (This will be discarded if
    // fCurrentPoint == fCurrContourStartPoint.)
    this->lineTo(fCurrStrokeJoinType, fCurrentPoint, fCurrContourStartPoint);
    this->writeJoin(fCurrStrokeJoinType, fCurrContourStartPoint, fLastControlPoint,
                    fCurrContourFirstControlPoint);
}
