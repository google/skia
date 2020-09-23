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

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static float num_combined_segments(float numParametricSegments, float numRadialSegments) {
    // The first and last edges are shared by both the parametric and radial sets of edges, so the
    // total number of edges is:
    //
    //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
    //
    // It's also important to differentiate between the number of edges and segments in a strip:
    //
    //   numCombinedSegments = numCombinedEdges - 1
    //
    // So the total number of segments in the combined strip is:
    //
    //   numCombinedSegments = numParametricEdges + numRadialEdges - 2 - 1
    //                       = numParametricSegments + 1 + numRadialSegments + 1 - 2 - 1
    //                       = numParametricSegments + numRadialSegments - 1
    //
    return numParametricSegments + numRadialSegments - 1;
}

static float num_parametric_segments(float numCombinedSegments, float numRadialSegments) {
    // numCombinedSegments = numParametricSegments + numRadialSegments - 1.
    // (See num_combined_segments()).
    return std::max(numCombinedSegments + 1 - numRadialSegments, 0.f);
}

static float pow4(float x) {
    float xx = x*x;
    return xx*xx;
}

GrStrokePatchBuilder::GrStrokePatchBuilder(GrMeshDrawOp::Target* target,
                                           SkTArray<PatchChunk>* patchChunkArray, float matrixScale,
                                           const SkStrokeRec& stroke, int totalCombinedVerbCnt)
        : fTarget(target)
        , fPatchChunkArray(patchChunkArray)
        , fMaxTessellationSegments(target->caps().shaderCaps()->maxTessellationSegments())
        , fLinearizationIntolerance(matrixScale *
                                    GrTessellationPathRenderer::kLinearizationIntolerance)
        , fStroke(stroke) {
    // We don't support hairline strokes. For now, the client can transform the path into device
    // space and then use a stroke width of 1.
    SkASSERT(fStroke.getWidth() > 0);

    // This is the number of radial segments we need to add to a triangle strip for each radian
    // of rotation, given the current stroke radius. Any fewer radial segments and our error
    // would fall outside the linearization tolerance.
    fNumRadialSegmentsPerRadian = 1 / std::acos(
            std::max(1 - 1 / (fLinearizationIntolerance * fStroke.getWidth() * .5f), -1.f));

    // Calculate the worst-case numbers of parametric segments our hardware can support for the
    // current stroke radius, in the event that there are also enough radial segments to rotate
    // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to
    // send almost all curves directly to the hardware without having to chop.
    float numRadialSegments180 = std::max(std::ceil(
            SK_ScalarPI * fNumRadialSegmentsPerRadian), 1.f);
    float maxParametricSegments180 = num_parametric_segments(fMaxTessellationSegments,
                                                             numRadialSegments180);
    fMaxParametricSegments180_pow4 = pow4(maxParametricSegments180);

    float numRadialSegments360 = std::max(std::ceil(
            2*SK_ScalarPI * fNumRadialSegmentsPerRadian), 1.f);
    float maxParametricSegments360 = num_parametric_segments(fMaxTessellationSegments,
                                                             numRadialSegments360);
    fMaxParametricSegments360_pow4 = pow4(maxParametricSegments360);

    // Now calculate the worst-case numbers of parametric segments if we are to integrate a join
    // into the same patch as the curve.
    float maxNumSegmentsInJoin;
    switch (fStroke.getJoin()) {
        case SkPaint::kBevel_Join:
            maxNumSegmentsInJoin = 1;
            break;
        case SkPaint::kMiter_Join:
            maxNumSegmentsInJoin = 2;
            break;
        case SkPaint::kRound_Join:
            // 180-degree round join.
            maxNumSegmentsInJoin = numRadialSegments180;
            break;
    }
    // Subtract an extra 1 off the end because when we integrate a join, the tessellator has to add
    // a redundant edge between the join and curve.
    fMaxParametricSegments180_pow4_withJoin = pow4(std::max(
            maxParametricSegments180 - maxNumSegmentsInJoin - 1, 0.f));
    fMaxParametricSegments360_pow4_withJoin = pow4(std::max(
            maxParametricSegments360 - maxNumSegmentsInJoin - 1, 0.f));
    fMaxCombinedSegments_withJoin = fMaxTessellationSegments - maxNumSegmentsInJoin - 1;
    fSoloRoundJoinAlwaysFitsInPatch = (numRadialSegments180 <= fMaxTessellationSegments);

    // Pre-allocate at least enough vertex space for 1 in 4 strokes to chop, and for 8 caps.
    int strokePreallocCount = totalCombinedVerbCnt * 5/4;
    int capPreallocCount = 8;
    this->allocPatchChunkAtLeast(strokePreallocCount + capPreallocCount);
}

void GrStrokePatchBuilder::addPath(const SkPath& path) {
    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = false;)
    SkPathVerb previousVerb = SkPathVerb::kClose;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                // "A subpath ... consisting of a single moveto shall not be stroked."
                // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
                    this->cap();
                }
                this->moveTo(pts[0]);
                break;
            case SkPathVerb::kLine:
                SkASSERT(fHasCurrentPoint);
                SkASSERT(pts[0] == fCurrentPoint);
                this->lineTo(pts[1]);
                break;
            case SkPathVerb::kQuad:
                this->quadraticTo(pts);
                break;
            case SkPathVerb::kCubic:
                this->cubicTo(pts);
                break;
            case SkPathVerb::kConic:
                SkUNREACHABLE;
            case SkPathVerb::kClose:
                this->close();
                break;
        }
        previousVerb = verb;
    }
    if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
        this->cap();
    }
}

void GrStrokePatchBuilder::moveTo(SkPoint pt) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokePatchBuilder::moveTo(SkPoint pt, SkPoint lastControlPoint) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fCurrContourFirstControlPoint = fLastControlPoint = lastControlPoint;
    fHasLastControlPoint = true;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokePatchBuilder::lineTo(SkPoint pt, JoinType prevJoinType) {
    SkASSERT(fHasCurrentPoint);

    // Zero-length paths need special treatment because they are spec'd to behave differently.
    if (pt == fCurrentPoint) {
        return;
    }

    if (fMaxCombinedSegments_withJoin < 1 || prevJoinType == JoinType::kCusp) {
        // Either the stroke has extremely thick round joins and there aren't enough guaranteed
        // segments to always combine a join with a line patch, or we need a cusp. Either way we
        // handle the join in its own separate patch.
        this->joinTo(prevJoinType, pt);
        prevJoinType = JoinType::kNone;
    }

    SkPoint asCubic[4] = {fCurrentPoint, fCurrentPoint, pt, pt};
    this->cubicToRaw(prevJoinType, asCubic);
}

static bool chop_pt_is_cusp(const SkPoint& prevControlPoint, const SkPoint& chopPoint,
                            const SkPoint& nextControlPoint) {
    // Adjacent chops should almost always be colinear. The only case where they will not be is a
    // cusp, which will rotate a minimum of 180 degrees.
    return (nextControlPoint - chopPoint).dot(chopPoint - prevControlPoint) <= 0;
}

static bool quad_chop_is_cusp(const SkPoint chops[5]) {
    SkPoint chopPt = chops[2];
    SkPoint prevCtrlPt = (chops[1] != chopPt) ? chops[1] : chops[0];
    SkPoint nextCtrlPt = (chops[3] != chopPt) ? chops[3] : chops[4];
    return chop_pt_is_cusp(prevCtrlPt, chopPt, nextCtrlPt);
}

void GrStrokePatchBuilder::quadraticTo(const SkPoint p[3], JoinType prevJoinType, int maxDepth) {
    SkASSERT(fHasCurrentPoint);
    SkASSERT(p[0] == fCurrentPoint);

    // Zero-length paths need special treatment because they are spec'd to behave differently. If
    // the control point is colocated on an endpoint then this might end up being the case. Fall
    // back on a lineTo and let it make the final check.
    if (p[1] == p[0] || p[1] == p[2]) {
        this->lineTo(p[2], prevJoinType);
        return;
    }

    // Convert to a cubic.
    SkPoint asCubic[4] = {p[0], lerp(p[0], p[1], 2/3.f), lerp(p[1], p[2], 1/3.f), p[2]};

    // Ensure our hardware supports enough tessellation segments to render the curve. This early out
    // assumes a worst-case quadratic rotation of 180 degrees and a worst-case number of segments in
    // the join.
    //
    // An informal survey of skottie animations and gms revealed that even with a bare minimum of 64
    // tessellation segments, 99.9%+ of quadratics take this early out.
    float numParametricSegments_pow4 = GrWangsFormula::quadratic_pow4(fLinearizationIntolerance, p);
    if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4_withJoin &&
        prevJoinType != JoinType::kCusp) {
        this->cubicToRaw(prevJoinType, asCubic);
        return;
    }

    if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4 || maxDepth == 0) {
        if (numParametricSegments_pow4 > fMaxParametricSegments180_pow4_withJoin ||
            prevJoinType == JoinType::kCusp) {
            // Either there aren't enough guaranteed segments to include the join in the quadratic's
            // patch, or we need a cusp. Emit a standalone patch for the join.
            this->joinTo(prevJoinType, asCubic);
            prevJoinType = JoinType::kNone;
        }
        this->cubicToRaw(prevJoinType, asCubic);
        return;
    }

    // We still might have enough tessellation segments to render the curve. Check again with the
    // actual rotation.
    float numRadialSegments = SkMeasureQuadRotation(p) * fNumRadialSegmentsPerRadian;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    float numParametricSegments = GrWangsFormula::root4(numParametricSegments_pow4);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    float numCombinedSegments = num_combined_segments(numParametricSegments, numRadialSegments);
    if (numCombinedSegments > fMaxTessellationSegments) {
        // The hardware doesn't support enough segments for this curve. Chop and recurse.
        if (maxDepth < 0) {
            // Decide on an extremely conservative upper bound for when to quit chopping. This
            // is solely to protect us from infinite recursion in instances where FP error
            // prevents us from chopping at the correct midtangent.
            maxDepth = sk_float_nextlog2(numParametricSegments) +
                       sk_float_nextlog2(numRadialSegments) + 1;
            maxDepth = std::max(maxDepth, 1);
        }
        SkPoint chops[5];
        if (numParametricSegments >= numRadialSegments) {
            SkChopQuadAtHalf(p, chops);
        } else {
            SkChopQuadAtMidTangent(p, chops);
        }
        this->quadraticTo(chops, prevJoinType, maxDepth - 1);
        // If we chopped at a cusp then rotation is not continuous between the two curves. Insert a
        // cusp to make up for lost rotation.
        JoinType nextJoinType = (quad_chop_is_cusp(chops)) ?
                JoinType::kCusp : JoinType::kFromStroke;
        this->quadraticTo(chops + 2, nextJoinType, maxDepth - 1);
        return;
    }

    if (numCombinedSegments > fMaxCombinedSegments_withJoin ||
        prevJoinType == JoinType::kCusp) {
        // Either there aren't enough guaranteed segments to include the join in the quadratic's
        // patch, or we need a cusp. Emit a standalone patch for the join.
        this->joinTo(prevJoinType, asCubic);
        prevJoinType = JoinType::kNone;
    }
    this->cubicToRaw(prevJoinType, asCubic);
}

static bool cubic_chop_is_cusp(const SkPoint chops[7]) {
    SkPoint chopPt = chops[3];
    auto prevCtrlPt = (chops[2] != chopPt) ? chops[2] : (chops[1] != chopPt) ? chops[1] : chops[0];
    auto nextCtrlPt = (chops[4] != chopPt) ? chops[4] : (chops[5] != chopPt) ? chops[5] : chops[6];
    return chop_pt_is_cusp(prevCtrlPt, chopPt, nextCtrlPt);
}

void GrStrokePatchBuilder::cubicTo(const SkPoint p[4], JoinType prevJoinType,
                                   Convex180Status convex180Status, int maxDepth) {
    SkASSERT(fHasCurrentPoint);
    SkASSERT(p[0] == fCurrentPoint);

    // The stroke tessellation shader assigns special meaning to p0==p1==p2 and p1==p2==p3. If this
    // is the case then we need to rewrite the cubic.
    if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
        this->lineTo(p[3], prevJoinType);
        return;
    }

    // Ensure our hardware supports enough tessellation segments to render the curve. This early out
    // assumes a worst-case cubic rotation of 360 degrees and a worst-case number of segments in the
    // join.
    //
    // An informal survey of skottie animations revealed that with a bare minimum of 64 tessellation
    // segments, 95% of cubics take this early out.
    float numParametricSegments_pow4 = GrWangsFormula::cubic_pow4(fLinearizationIntolerance, p);
    if (numParametricSegments_pow4 <= fMaxParametricSegments360_pow4_withJoin &&
        prevJoinType != JoinType::kCusp) {
        this->cubicToRaw(prevJoinType, p);
        return;
    }

    float maxParametricSegments_pow4 = (convex180Status == Convex180Status::kYes) ?
            fMaxParametricSegments180_pow4 : fMaxParametricSegments360_pow4;
    if (numParametricSegments_pow4 <= maxParametricSegments_pow4 || maxDepth == 0) {
        float maxParametricSegments_pow4_withJoin = (convex180Status == Convex180Status::kYes) ?
                fMaxParametricSegments180_pow4_withJoin : fMaxParametricSegments360_pow4_withJoin;
        if (numParametricSegments_pow4 > maxParametricSegments_pow4_withJoin ||
            prevJoinType == JoinType::kCusp) {
            // Either there aren't enough guaranteed segments to include the join in the cubic's
            // patch, or we need a cusp. Emit a standalone patch for the join.
            this->joinTo(prevJoinType, p);
            prevJoinType = JoinType::kNone;
        }
        this->cubicToRaw(prevJoinType, p);
        return;
    }

    // Ensure the curve does not inflect or rotate >180 degrees before we start subdividing and
    // measuring rotation.
    SkPoint chops[10];
    if (convex180Status == Convex180Status::kUnknown) {
        float chopT[2];
        int n = SkFindCubicInflections(p, chopT);
        if (n == 0) {
            // No inflections. Chop at midtangent to guarantee rotation <= 180 degrees.
            chopT[0] = SkFindCubicMidTangent(p);
            n = 1;
        }
        SkChopCubicAt(p, chops, chopT, n);
        this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth);
        for (int i = 1; i <= n; ++i) {
            // If we chopped at a cusp then rotation is not continuous between the two curves.
            // Insert a double cuspe up for lost rotation. (This should not be possible from a
            // purely mathematical standpoint, since an inflection is not a cusp, but we still check
            // for the sake of robust handling of FP32 precision issues.)
            JoinType nextJoinType = (cubic_chop_is_cusp(chops + (i - 1)*3)) ?
                    JoinType::kCusp : JoinType::kFromStroke;
            this->cubicTo(chops + i*3, nextJoinType, Convex180Status::kYes, maxDepth);
        }
        return;
    }

    // We still might have enough tessellation segments to render the curve. Check again with
    // its actual rotation.
    float numRadialSegments = SkMeasureNonInflectCubicRotation(p) * fNumRadialSegmentsPerRadian;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    float numParametricSegments = GrWangsFormula::root4(numParametricSegments_pow4);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    float numCombinedSegments = num_combined_segments(numParametricSegments, numRadialSegments);
    if (numCombinedSegments > fMaxTessellationSegments) {
        // The hardware doesn't support enough segments for this curve. Chop and recurse.
        if (maxDepth < 0) {
            // Decide on an extremely conservative upper bound for when to quit chopping. This
            // is solely to protect us from infinite recursion in instances where FP error
            // prevents us from chopping at the correct midtangent.
            maxDepth = sk_float_nextlog2(numParametricSegments) +
                       sk_float_nextlog2(numRadialSegments) + 1;
            maxDepth = std::max(maxDepth, 1);
        }
        if (numParametricSegments >= numRadialSegments) {
            SkChopCubicAtHalf(p, chops);
        } else {
            SkChopCubicAtMidTangent(p, chops);
        }
        // If we chopped at a cusp then rotation is not continuous between the two curves. Insert a
        // cusp to make up for lost rotation.
        JoinType nextJoinType = (cubic_chop_is_cusp(chops)) ?
                JoinType::kCusp : JoinType::kFromStroke;
        this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth - 1);
        this->cubicTo(chops + 3, nextJoinType, Convex180Status::kYes, maxDepth - 1);
        return;
    }

    if (numCombinedSegments > fMaxCombinedSegments_withJoin || prevJoinType == JoinType::kCusp) {
        // Either there aren't enough guaranteed segments to include the join in the cubic's patch,
        // or we need a cusp. Emit a standalone patch for the join.
        this->joinTo(prevJoinType, p);
        prevJoinType = JoinType::kNone;
    }
    this->cubicToRaw(prevJoinType, p);
}

void GrStrokePatchBuilder::joinTo(JoinType joinType, SkPoint nextControlPoint, int maxDepth) {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // The first stroke doesn't have a previous join.
        return;
    }

    if (!fSoloRoundJoinAlwaysFitsInPatch && maxDepth != 0 &&
        (fStroke.getJoin() == SkPaint::kRound_Join || joinType == JoinType::kCusp)) {
        SkVector tan0 = fCurrentPoint - fLastControlPoint;
        SkVector tan1 = nextControlPoint - fCurrentPoint;
        float rotation = SkMeasureAngleInsideVectors(tan0, tan1);
        float numRadialSegments = rotation * fNumRadialSegmentsPerRadian;
        if (numRadialSegments > fMaxTessellationSegments) {
            // This is a round join that requires more segments than the tessellator supports.
            // Split it and recurse.
            if (maxDepth < 0) {
                // Decide on an upper bound for when to quit chopping. This is solely to protect
                // us from infinite recursion due to FP precision issues.
                maxDepth = sk_float_nextlog2(numRadialSegments / fMaxTessellationSegments);
                maxDepth = std::max(maxDepth, 1);
            }
            // Find the bisector so we can split the join in half.
            SkPoint bisector = SkFindBisector(tan0, tan1);
            // c0 will be the "next" control point for the first join half, and c1 will be the
            // "previous" control point for the second join half.
            SkPoint c0, c1;
            // FIXME: This hack ensures "c0 - fCurrentPoint" gives the exact same ieee fp32 vector
            // as "-(c1 - fCurrentPoint)". If our current strategy of join chopping sticks, we may
            // want to think of a cleaner method to avoid T-junctions when we chop joins.
            int maxAttempts = 10;
            do {
                bisector = (fCurrentPoint + bisector) - (fCurrentPoint - bisector);
                c0 = fCurrentPoint + bisector;
                c1 = fCurrentPoint - bisector;
            } while (c0 - fCurrentPoint != -(c1 - fCurrentPoint) && --maxAttempts);
            this->joinTo(joinType, c0, maxDepth - 1);  // First join half.
            fLastControlPoint = c1;
            this->joinTo(joinType, nextControlPoint, maxDepth - 1);  // Second join half.
            return;
        }
    }

    this->joinToRaw(joinType, nextControlPoint);
}

void GrStrokePatchBuilder::close() {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // Draw caps instead of closing if the subpath is zero length:
        //
        //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has a
        //   value of round or square producing respectively a circle or a square."
        //
        //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
        //
        this->cap();
        return;
    }

    // Draw a line back to the beginning. (This will be discarded if
    // fCurrentPoint == fCurrContourStartPoint.)
    this->lineTo(fCurrContourStartPoint);
    this->joinTo(JoinType::kFromStroke, fCurrContourFirstControlPoint);

    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = false;)
}

static SkVector normalize(const SkVector& v) {
    SkVector norm = v;
    norm.normalize();
    return norm;
}

void GrStrokePatchBuilder::cap() {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // We don't have any control points to orient the caps. In this case, square and round caps
        // are specified to be drawn as an axis-aligned square or circle respectively. Assign
        // default control points that achieve this.
        fCurrContourFirstControlPoint = fCurrContourStartPoint - SkPoint{1,0};
        fLastControlPoint = fCurrContourStartPoint + SkPoint{1,0};
        fCurrentPoint = fCurrContourStartPoint;
        fHasLastControlPoint = true;
    }

    switch (fStroke.getCap()) {
        case SkPaint::kButt_Cap:
            break;
        case SkPaint::kRound_Cap: {
            // A round cap is the same thing as a 180-degree round join.
            // If our join type isn't round we can alternatively use a cusp.
            JoinType roundCapJoinType = (fStroke.getJoin() == SkPaint::kRound_Join) ?
                    JoinType::kFromStroke : JoinType::kCusp;
            this->joinTo(roundCapJoinType, fLastControlPoint);
            this->moveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->joinTo(roundCapJoinType, fCurrContourFirstControlPoint);
            break;
        }

        case SkPaint::kSquare_Cap: {
            // A square cap is the same as appending lineTos.
            float rad = fStroke.getWidth() * .5f;
            this->lineTo(fCurrentPoint + normalize(fCurrentPoint - fLastControlPoint) * rad);
            this->moveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
            this->lineTo(fCurrContourStartPoint +
                         normalize(fCurrContourStartPoint - fCurrContourFirstControlPoint) * rad);
            break;
        }
    }

    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = false;)
}

void GrStrokePatchBuilder::cubicToRaw(JoinType prevJoinType, const SkPoint pts[4]) {
    // Cusps can't be combined with a stroke patch. They need to have been written out already as
    // their own standalone patch.
    SkASSERT(prevJoinType != JoinType::kCusp);

    SkPoint c1 = (pts[1] == pts[0]) ? pts[2] : pts[1];
    SkPoint c2 = (pts[2] == pts[3]) ? pts[1] : pts[2];

    if (!fHasLastControlPoint) {
        // The first stroke doesn't have a previous join (yet). If the current contour ends up
        // closing itself, we will add that join as its own patch.
        // TODO: Consider deferring the first stroke until we know whether the contour will close.
        // This will allow us to use the closing join as the first patch's previous join.
        prevJoinType = JoinType::kNone;
        fCurrContourFirstControlPoint = c1;
        fHasLastControlPoint = true;
    } else {
        // By using JoinType::kNone, the caller promises to have written out their own join that
        // seams exactly with this curve.
        SkASSERT((prevJoinType != JoinType::kNone) || fLastControlPoint == c1);
    }

    if (Patch* patch = this->reservePatch()) {
        // Disable the join section of this patch if prevJoinType is kNone by setting the previous
        // control point equal to p0.
        patch->fPrevControlPoint = (prevJoinType == JoinType::kNone) ? pts[0] : fLastControlPoint;
        patch->fPts = {pts[0], pts[1], pts[2], pts[3]};
    }

    fLastControlPoint = c2;
    fCurrentPoint = pts[3];
}

void GrStrokePatchBuilder::joinToRaw(JoinType joinType, SkPoint nextControlPoint) {
    // We should never write out joins before the first curve.
    SkASSERT(fHasLastControlPoint);
    SkASSERT(fHasCurrentPoint);

    if (Patch* joinPatch = this->reservePatch()) {
        joinPatch->fPrevControlPoint = fLastControlPoint;
        joinPatch->fPts[0] = fCurrentPoint;
        if (joinType == JoinType::kFromStroke) {
            // [p0, p3, p3, p3] is a reserved pattern that means this patch is a join only (no cubic
            // sections in the patch).
            joinPatch->fPts[1] = joinPatch->fPts[2] = nextControlPoint;
        } else {
            SkASSERT(joinType == JoinType::kCusp);
            // [p0, p0, p0, p3] is a reserved pattern that means this patch is a cusp point.
            joinPatch->fPts[1] = joinPatch->fPts[2] = fCurrentPoint;
        }
        joinPatch->fPts[3] = nextControlPoint;
    }

    fLastControlPoint = nextControlPoint;
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

void GrStrokePatchBuilder::allocPatchChunkAtLeast(int minPatchAllocCount) {
    PatchChunk* chunk = &fPatchChunkArray->push_back();
    fCurrChunkPatchData = (Patch*)fTarget->makeVertexSpaceAtLeast(sizeof(Patch), minPatchAllocCount,
                                                                  minPatchAllocCount,
                                                                  &chunk->fPatchBuffer,
                                                                  &chunk->fBasePatch,
                                                                  &fCurrChunkPatchCapacity);
    fCurrChunkMinPatchAllocCount = minPatchAllocCount;
}
