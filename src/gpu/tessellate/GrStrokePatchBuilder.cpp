/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokePatchBuilder.h"

#include "include/core/SkStrokeRec.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkNx.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrVectorXform.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

constexpr static float kStandardCubicType = GrStrokeTessellateShader::kStandardCubicType;
constexpr static float kDoubleSidedRoundJoinType = -GrStrokeTessellateShader::kRoundJoinType;

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
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

void GrStrokePatchBuilder::writeCubicSegment(float prevJoinType, const SkPoint pts[4]) {
    SkPoint c1 = (pts[1] == pts[0]) ? pts[2] : pts[1];
    SkPoint c2 = (pts[2] == pts[3]) ? pts[1] : pts[2];

    if (fHasPreviousSegment) {
        this->writeJoin(prevJoinType, fLastControlPoint, pts[0], c1);
    } else {
        fCurrContourFirstControlPoint = c1;
        fHasPreviousSegment = true;
    }

    if (SkPoint* patch = this->reservePatch()) {
        memcpy(patch, pts, sizeof(SkPoint) * 4);
        patch[4].set(kStandardCubicType, fCurrStrokeRadius);
    }

    fLastControlPoint = c2;
    fCurrentPoint = pts[3];
}

void GrStrokePatchBuilder::writeJoin(float joinType, const SkPoint& prevControlPoint,
                                     const SkPoint& anchorPoint, const SkPoint& nextControlPoint) {
    if (SkPoint* joinPatch = this->reservePatch()) {
        joinPatch[0] = prevControlPoint;
        joinPatch[1] = anchorPoint;
        joinPatch[2] = anchorPoint;
        joinPatch[3] = nextControlPoint;
        joinPatch[4].set(joinType, fCurrStrokeRadius);
    }
}

void GrStrokePatchBuilder::writeSquareCap(const SkPoint& endPoint, const SkPoint& controlPoint) {
    SkVector v = (endPoint - controlPoint);
    v.normalize();
    SkPoint capPoint = endPoint + v*fCurrStrokeRadius;
    // Add a join to guarantee we get water tight seaming. Make the join type negative so it's
    // double sided.
    this->writeJoin(-fCurrStrokeJoinType, controlPoint, endPoint, capPoint);
    if (SkPoint* capPatch = this->reservePatch()) {
        capPatch[0] = endPoint;
        capPatch[1] = endPoint;
        capPatch[2] = capPoint;
        capPatch[3] = capPoint;
        capPatch[4].set(kStandardCubicType, fCurrStrokeRadius);
    }
}

void GrStrokePatchBuilder::writeCaps(SkPaint::Cap capType) {
    if (!fHasPreviousSegment) {
        // We don't have any control points to orient the caps. In this case, square and round caps
        // are specified to be drawn as an axis-aligned square or circle respectively. Assign
        // default control points that achieve this.
        fCurrContourFirstControlPoint = fCurrContourStartPoint - SkPoint{1,0};
        fLastControlPoint = fCurrContourStartPoint + SkPoint{1,0};
        fCurrentPoint = fCurrContourStartPoint;
    }

    switch (capType) {
        case SkPaint::kButt_Cap:
            break;
        case SkPaint::kRound_Cap:
            // A round cap is the same thing as a 180-degree round join.
            this->writeJoin(GrStrokeTessellateShader::kRoundJoinType, fCurrContourFirstControlPoint,
                            fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->writeJoin(GrStrokeTessellateShader::kRoundJoinType, fLastControlPoint,
                            fCurrentPoint, fLastControlPoint);
            break;
        case SkPaint::kSquare_Cap:
            this->writeSquareCap(fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->writeSquareCap(fCurrentPoint, fLastControlPoint);
            break;
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

void GrStrokePatchBuilder::addPath(const SkPath& path, const SkStrokeRec& stroke) {
    // We don't support hairline strokes. For now, the client can transform the path into device
    // space and then use a stroke width of 1.
    SkASSERT(stroke.getWidth() > 0);

    fCurrStrokeRadius = stroke.getWidth()/2;
    fCurrStrokeJoinType = join_type_from_join(stroke.getJoin());

    // This is the number of radial segments we need to add to a triangle strip for each radian of
    // rotation, given the current stroke radius. Any fewer radial segments and our error would fall
    // outside the linearization tolerance.
    fNumRadialSegmentsPerRad = 1 / std::acos(
            std::max(1 - 1 / (fLinearizationIntolerance * fCurrStrokeRadius), -1.f));

    // Calculate the worst-case numbers of parametric segments our hardware can support for the
    // current stroke radius, in the event that there are also enough radial segments to rotate 180
    // and 360 degrees respectively. These are used for "quick accepts" that allow us to send almost
    // all curves directly to the hardware without having to chop or think any further.
    fMaxParametricSegments180 = fMaxTessellationSegments + 1 - std::max(std::ceil(
                                        SK_ScalarPI * fNumRadialSegmentsPerRad), 1.f);
    fMaxParametricSegments360 = fMaxTessellationSegments + 1 - std::max(std::ceil(
                                        2*SK_ScalarPI * fNumRadialSegmentsPerRad), 1.f);

    fHasPreviousSegment = false;
    SkPathVerb previousVerb = SkPathVerb::kClose;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                // "A subpath ... consisting of a single moveto shall not be stroked."
                // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
                    this->writeCaps(stroke.getCap());
                }
                this->moveTo(pts[0]);
                break;
            case SkPathVerb::kClose:
                this->close(stroke.getCap());
                break;
            case SkPathVerb::kLine:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->lineTo(fCurrStrokeJoinType, pts[0], pts[1]);
                break;
            case SkPathVerb::kQuad:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->quadraticTo(fCurrStrokeJoinType, pts);
                break;
            case SkPathVerb::kCubic:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                this->cubicTo(fCurrStrokeJoinType, pts);
                break;
            case SkPathVerb::kConic:
                SkASSERT(previousVerb != SkPathVerb::kClose);
                SkUNREACHABLE;
        }
        previousVerb = verb;
    }
    if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
        this->writeCaps(stroke.getCap());
    }
}

void GrStrokePatchBuilder::moveTo(const SkPoint& pt) {
    fHasPreviousSegment = false;
    fCurrContourStartPoint = pt;
}

void GrStrokePatchBuilder::lineTo(float prevJoinType, const SkPoint& p0, const SkPoint& p1) {
    // Zero-length paths need special treatment because they are spec'd to behave differently.
    if (p0 == p1) {
        return;
    }

    SkPoint cubic[4] = {p0, p0, p1, p1};
    this->writeCubicSegment(prevJoinType, cubic);
}

void GrStrokePatchBuilder::quadraticTo(float prevJoinType, const SkPoint p[3], int maxDepth) {
    // The stroker relies on p1 to find tangents at the endpoints. (We have to treat the endpoint
    // tangents carefully in order to get water tight seams with the join segments.) If p1 is
    // colocated on an endpoint then we need to draw this quadratic as a line instead.
    if (p[1] == p[0] || p[1] == p[2]) {
        this->lineTo(prevJoinType, p[0], p[2]);
        return;
    }

    // Ensure our hardware supports enough tessellation segments to render the curve. The first
    // branch assumes a worst-case rotation of 180 degrees and checks if even then we have enough.
    // In practice it is rare to take even the first branch.
    float numParametricSegments = GrWangsFormula::quadratic(fLinearizationIntolerance, p);
    if (numParametricSegments > fMaxParametricSegments180 && maxDepth != 0) {
        // We still might have enough tessellation segments to render the curve. Check again with
        // the actual rotation.
        float numRadialSegments = SkMeasureQuadRotation(p) * fNumRadialSegmentsPerRad;
        numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
        numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
        if (numParametricSegments + numRadialSegments - 1 > fMaxTessellationSegments) {
            // The hardware doesn't support enough segments for this curve. Chop and recurse.
            if (maxDepth < 0) {
                // Decide on an extremely conservative upper bound for when to quit chopping. This
                // is solely to protect us from infinite recursion in instances where FP error
                // prevents us from chopping at the correct midtangent.
                maxDepth = sk_float_nextlog2(numParametricSegments) +
                           sk_float_nextlog2(numRadialSegments) + 1;
                SkASSERT(maxDepth >= 1);
            }
            SkPoint chopped[5];
            if (numParametricSegments >= numRadialSegments) {
                SkChopQuadAtHalf(p, chopped);
            } else {
                SkChopQuadAtMidTangent(p, chopped);
            }
            this->quadraticTo(prevJoinType, chopped, maxDepth - 1);
            // Use kDoubleSidedRoundJoinType in case we happened to chop at the exact turnaround
            // point of a flat quadratic, in which case we would lose 180 degrees of rotation.
            this->quadraticTo(kDoubleSidedRoundJoinType, chopped + 2, maxDepth - 1);
            return;
        }
    }

    SkPoint cubic[4] = {p[0], lerp(p[0], p[1], 2/3.f), lerp(p[1], p[2], 1/3.f), p[2]};
    this->writeCubicSegment(prevJoinType, cubic);
}

void GrStrokePatchBuilder::cubicTo(float prevJoinType, const SkPoint inputPts[4]) {
    const SkPoint* p = inputPts;
    int numCubics = 1;
    SkPoint chopped[10];
    double tt[2], ss[2];
    if (SkClassifyCubic(p, tt, ss) == SkCubicType::kSerpentine) {
        // TEMPORARY: Don't allow cubics to have inflection points.
        // TODO: This will soon be moved into the GPU tessellation pipeline and handled more
        // elegantly.
        float t[2] = {(float)(tt[0]/ss[0]), (float)(tt[1]/ss[1])};
        const float* begin = (t[0] > 0 && t[0] < 1) ? t : t+1;
        const float* end = (t[1] > 0 && t[1] > t[0] && t[1] < 1) ? t+2 : t+1;
        numCubics = (end - begin) + 1;
        if (numCubics > 1) {
            SkChopCubicAt(p, chopped, begin, end - begin);
            p = chopped;
        }
    } else if (SkMeasureNonInflectCubicRotation(p) > SK_ScalarPI*15/16) {
        // TEMPORARY: Don't allow cubics to turn more than 180 degrees. We chop them when they get
        // close, just to be sure.
        // TODO: This will soon be moved into the GPU tessellation pipeline and handled more
        // elegantly.
        SkChopCubicAtMidTangent(p, chopped);
        p = chopped;
        numCubics = 2;
    }
    for (int i = 0; i < numCubics; ++i) {
        this->nonInflectCubicTo(prevJoinType, p + i*3);
        // Use kDoubleSidedRoundJoinType in case we happened to chop at an exact cusp or turnaround
        // point of a flat cubic, in which case we would lose 180 degrees of rotation.
        prevJoinType = kDoubleSidedRoundJoinType;
    }
}

void GrStrokePatchBuilder::nonInflectCubicTo(float prevJoinType, const SkPoint p[4], int maxDepth) {
    // The stroker relies on p1 and p2 to find tangents at the endpoints. (We have to treat the
    // endpoint tangents carefully in order to get water tight seams with the join segments.) If p0
    // and p1 are both colocated on an endpoint then we need to draw this cubic as a line instead.
    if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
        this->lineTo(prevJoinType, p[0], p[3]);
        return;
    }

    // Ensure our hardware supports enough tessellation segments to render the curve. The first
    // branch assumes a worst-case rotation of 360 degrees and checks if even then we have enough.
    // In practice it is rare to take even the first branch.
    //
    // NOTE: We could technically assume a worst-case rotation of 180 because cubicTo() chops at
    // midtangents and inflections. However, this is only temporary so we leave it at 360 where it
    // will arrive at in the future.
    float numParametricSegments = GrWangsFormula::cubic(fLinearizationIntolerance, p);
    if (numParametricSegments > fMaxParametricSegments360 && maxDepth != 0) {
        // We still might have enough tessellation segments to render the curve. Check again with
        // the actual rotation.
        float numRadialSegments = SkMeasureNonInflectCubicRotation(p) * fNumRadialSegmentsPerRad;
        numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
        numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
        if (numParametricSegments + numRadialSegments - 1 > fMaxTessellationSegments) {
            // The hardware doesn't support enough segments for this curve. Chop and recurse.
            if (maxDepth < 0) {
                // Decide on an extremely conservative upper bound for when to quit chopping. This
                // is solely to protect us from infinite recursion in instances where FP error
                // prevents us from chopping at the correct midtangent.
                maxDepth = sk_float_nextlog2(numParametricSegments) +
                           sk_float_nextlog2(numRadialSegments) + 1;
                SkASSERT(maxDepth >= 1);
            }
            SkPoint chopped[7];
            if (numParametricSegments >= numRadialSegments) {
                SkChopCubicAtHalf(p, chopped);
            } else {
                SkChopCubicAtMidTangent(p, chopped);
            }
            this->nonInflectCubicTo(prevJoinType, chopped, maxDepth - 1);
            // Use kDoubleSidedRoundJoinType in case we happened to chop at an exact cusp or
            // turnaround point of a flat cubic, in which case we would lose 180 degrees of
            // rotation.
            this->nonInflectCubicTo(kDoubleSidedRoundJoinType, chopped + 3, maxDepth - 1);
            return;
        }
    }

    this->writeCubicSegment(prevJoinType, p);
}

void GrStrokePatchBuilder::close(SkPaint::Cap capType) {
    if (!fHasPreviousSegment) {
        // Draw caps instead of closing if the subpath is zero length:
        //
        //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has a
        //   value of round or square producing respectively a circle or a square."
        //
        //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
        //
        this->writeCaps(capType);
        return;
    }

    // Draw a line back to the beginning. (This will be discarded if
    // fCurrentPoint == fCurrContourStartPoint.)
    this->lineTo(fCurrStrokeJoinType, fCurrentPoint, fCurrContourStartPoint);
    this->writeJoin(fCurrStrokeJoinType, fLastControlPoint, fCurrContourStartPoint,
                    fCurrContourFirstControlPoint);
}
