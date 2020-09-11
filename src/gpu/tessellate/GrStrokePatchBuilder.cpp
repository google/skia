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

using Patch = GrStrokeTessellateShader::Patch;

constexpr static float kDoubleSidedRoundJoinType = -Patch::kRoundJoinType;

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

void GrStrokePatchBuilder::allocPatchChunkAtLeast(int minPatchAllocCount) {
    PatchChunk* chunk = &fPatchChunkArray->push_back();
    fCurrChunkPatchData = (Patch*)fTarget->makeVertexSpaceAtLeast(sizeof(Patch), minPatchAllocCount,
                                                                  minPatchAllocCount,
                                                                  &chunk->fPatchBuffer,
                                                                  &chunk->fBasePatch,
                                                                  &fCurrChunkPatchCapacity);
    fCurrChunkMinPatchAllocCount = minPatchAllocCount;
}

Patch* GrStrokePatchBuilder::reservePatch() {
    if (fPatchChunkArray->back().fPatchCount >= fCurrChunkPatchCapacity) {
        // The current chunk is full. Time to allocate a new one. (And no need to put back vertices;
        // the buffer is full.)
        this->allocPatchChunkAtLeast(fCurrChunkMinPatchAllocCount * 2);
    }
    if (!fCurrChunkPatchData) {
        SkDebugf("WARNING: Failed to allocate vertex buffer for tessellated stroke.");
        return nullptr;
    }
    SkASSERT(fPatchChunkArray->back().fPatchCount <= fCurrChunkPatchCapacity);
    Patch* patch = fCurrChunkPatchData + fPatchChunkArray->back().fPatchCount;
    ++fPatchChunkArray->back().fPatchCount;
    return patch;
}

void GrStrokePatchBuilder::writeCubicSegment(float prevJoinType, const SkPoint pts[4],
                                             float cubicType) {
    SkPoint c1 = (pts[1] == pts[0]) ? pts[2] : pts[1];
    SkPoint c2 = (pts[2] == pts[3]) ? pts[1] : pts[2];

    if (fHasPreviousSegment) {
        this->writeJoin(prevJoinType, fLastControlPoint, pts[0], c1);
    } else {
        fCurrContourFirstControlPoint = c1;
        fHasPreviousSegment = true;
    }

    SkASSERT(cubicType == Patch::kStandardCubicType || cubicType == Patch::kFlatLineType);
    if (Patch* patch = this->reservePatch()) {
        memcpy(patch->fPts.data(), pts, sizeof(patch->fPts));
        patch->fPatchType = cubicType;
        patch->fStrokeRadius = fCurrStrokeRadius;
    }

    fLastControlPoint = c2;
    fCurrentPoint = pts[3];
}

void GrStrokePatchBuilder::writeJoin(float joinType, const SkPoint& prevControlPoint,
                                     const SkPoint& anchorPoint, const SkPoint& nextControlPoint) {
    SkASSERT(SkScalarAbs(joinType) == Patch::kRoundJoinType ||
             SkScalarAbs(joinType) == Patch::kMiterJoinType ||
             SkScalarAbs(joinType) == Patch::kBevelJoinType);
    if (Patch* joinPatch = this->reservePatch()) {
        joinPatch->fPts = {{prevControlPoint, anchorPoint, anchorPoint, nextControlPoint}};
        joinPatch->fPatchType = joinType;
        joinPatch->fStrokeRadius = fCurrStrokeRadius;
    }
}

void GrStrokePatchBuilder::writeSquareCap(const SkPoint& endPoint, const SkPoint& controlPoint) {
    SkVector v = (endPoint - controlPoint);
    v.normalize();
    SkPoint capPoint = endPoint + v*fCurrStrokeRadius;
    // Add a join to guarantee we get water tight seaming. Make the join type negative so it's
    // double sided.
    this->writeJoin(-fCurrStrokeJoinType, controlPoint, endPoint, capPoint);
    if (Patch* capPatch = this->reservePatch()) {
        capPatch->fPts = {{endPoint, endPoint, capPoint, capPoint}};
        capPatch->fPatchType = Patch::kFlatLineType;
        capPatch->fStrokeRadius = fCurrStrokeRadius;
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
            this->writeJoin(Patch::kRoundJoinType, fCurrContourFirstControlPoint,
                            fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->writeJoin(Patch::kRoundJoinType, fLastControlPoint, fCurrentPoint,
                            fLastControlPoint);
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
            return GrStrokeTessellateShader::Patch::kBevelJoinType;
        case SkPaint::kMiter_Join:
            return GrStrokeTessellateShader::Patch::kMiterJoinType;
        case SkPaint::kRound_Join:
            return GrStrokeTessellateShader::Patch::kRoundJoinType;
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
    this->writeCubicSegment(prevJoinType, cubic, Patch::kFlatLineType);
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
    this->writeCubicSegment(prevJoinType, cubic, Patch::kStandardCubicType);
}

void GrStrokePatchBuilder::cubicTo(float prevJoinType, const SkPoint p[4], int maxDepth,
                                   bool mightInflect) {
    // The stroker relies on p1 and p2 to find tangents at the endpoints. (We have to treat the
    // endpoint tangents carefully in order to get water tight seams with the join segments.) If p0
    // and p1 are both colocated on an endpoint then we need to draw this cubic as a line instead.
    if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
        this->lineTo(prevJoinType, p[0], p[3]);
        return;
    }

    // Early-out if by conservative estimate we can ensure our hardware supports enough tessellation
    // segments to render the curve. In practice we almost always take this branch.
    float numParametricSegments = GrWangsFormula::cubic(fLinearizationIntolerance, p);
    if (numParametricSegments <= fMaxParametricSegments360 || maxDepth == 0) {
        this->writeCubicSegment(prevJoinType, p, Patch::kStandardCubicType);
        return;
    }

    // Ensure the curve does not inflect before we attempt to measure its rotation.
    SkPoint chopped[10];
    if (mightInflect) {
        float inflectT[2];
        if (int n = SkFindCubicInflections(p, inflectT)) {
            SkChopCubicAt(p, chopped, inflectT, n);
            for (int i = 0; i <= n; ++i) {
                this->cubicTo(prevJoinType, chopped + i*3, maxDepth, false);
                // Switch to kDoubleSidedRoundJoinType in case we happened to chop at an exact cusp
                // or turnaround point of a flat cubic, in which case we would lose 180 degrees of
                // rotation.
                prevJoinType = kDoubleSidedRoundJoinType;
            }
            return;
        }
    }

    // We still might have enough tessellation segments to render the curve. Check again with
    // its actual rotation.
    float numRadialSegments = SkMeasureNonInflectCubicRotation(p) * fNumRadialSegmentsPerRad;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    if (numParametricSegments + numRadialSegments - 1 <= fMaxTessellationSegments) {
        this->writeCubicSegment(prevJoinType, p, Patch::kStandardCubicType);
        return;
    }

    // The hardware doesn't support enough segments to tessellate this curve. Chop and recurse.
    if (numParametricSegments >= numRadialSegments) {
        SkChopCubicAtHalf(p, chopped);
    } else {
        SkChopCubicAtMidTangent(p, chopped);
    }

    if (maxDepth < 0) {
        // Decide on an extremely conservative upper bound for when to quit chopping. This
        // is solely to protect us from infinite recursion in instances where FP error
        // prevents us from chopping at the correct midtangent.
        maxDepth = sk_float_nextlog2(numParametricSegments) +
                   sk_float_nextlog2(numRadialSegments) + 1;
        SkASSERT(maxDepth >= 1);
    }

    this->cubicTo(prevJoinType, chopped, maxDepth - 1, false);
    // Use kDoubleSidedRoundJoinType in case we happened to chop at an exact cusp or
    // turnaround point of a flat cubic, in which case we would lose 180 degrees of
    // rotation.
    this->cubicTo(kDoubleSidedRoundJoinType, chopped + 3, maxDepth - 1, false);
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
