/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

using Patch = GrStrokeTessellateShader::Patch;

void GrStrokeTessellateOp::onPrePrepare(GrRecordingContext* context,
                                        const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                        const GrXferProcessor::DstProxyView& dstProxyView,
                                        GrXferBarrierFlags renderPassXferBarriers,
                                        GrLoadOp colorLoadOp) {
    this->prePreparePrograms(GrStrokeTessellateShader::Mode::kTessellation,
                             context->priv().recordTimeAllocator(), writeView,
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled(), dstProxyView,
                             renderPassXferBarriers, colorLoadOp, *context->priv().caps());
    if (fStencilProgram) {
        context->priv().recordProgramInfo(fStencilProgram);
    }
    if (fFillProgram) {
        context->priv().recordProgramInfo(fFillProgram);
    }
}

void GrStrokeTessellateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fFillProgram && !fStencilProgram) {
        this->prePreparePrograms(GrStrokeTessellateShader::Mode::kTessellation,
                                 flushState->allocator(), flushState->writeView(),
                                 flushState->detachAppliedClip(), flushState->dstProxyView(),
                                 flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                 flushState->caps());
    }
    SkASSERT(fFillProgram || fStencilProgram);

    fTarget = flushState;
    this->prepareBuffers();
    fTarget = nullptr;
}

static float pow4(float x) {
    float xx = x*x;
    return xx*xx;
}

void GrStrokeTessellateOp::prepareBuffers() {
    SkASSERT(fTarget);

    // Subtract 2 because the tessellation shader chops every cubic at two locations, and each chop
    // has the potential to introduce an extra segment.
    fMaxTessellationSegments = fTarget->caps().shaderCaps()->maxTessellationSegments() - 2;

    fTolerances = this->preTransformTolerances();

    // Calculate the worst-case numbers of parametric segments our hardware can support for the
    // current stroke radius, in the event that there are also enough radial segments to rotate
    // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to
    // send almost all curves directly to the hardware without having to chop.
    float numRadialSegments180 = std::max(std::ceil(
            SK_ScalarPI * fTolerances.fNumRadialSegmentsPerRadian), 1.f);
    float maxParametricSegments180 = NumParametricSegments(fMaxTessellationSegments,
                                                           numRadialSegments180);
    fMaxParametricSegments180_pow4 = pow4(maxParametricSegments180);

    float numRadialSegments360 = std::max(std::ceil(
            2*SK_ScalarPI * fTolerances.fNumRadialSegmentsPerRadian), 1.f);
    float maxParametricSegments360 = NumParametricSegments(fMaxTessellationSegments,
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
    int strokePreallocCount = fTotalCombinedVerbCnt * 5/4;
    int capPreallocCount = 8;
    this->allocPatchChunkAtLeast(strokePreallocCount + capPreallocCount);

    for (const SkPath& path : fPathList) {
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
}

void GrStrokeTessellateOp::moveTo(SkPoint pt) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokeTessellateOp::moveTo(SkPoint pt, SkPoint lastControlPoint) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fCurrContourFirstControlPoint = fLastControlPoint = lastControlPoint;
    fHasLastControlPoint = true;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokeTessellateOp::lineTo(SkPoint pt, JoinType prevJoinType) {
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

void GrStrokeTessellateOp::quadraticTo(const SkPoint p[3], JoinType prevJoinType, int maxDepth) {
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
    SkPoint asCubic[4];
    GrPathUtils::convertQuadToCubic(p, asCubic);

    // Ensure our hardware supports enough tessellation segments to render the curve. This early out
    // assumes a worst-case quadratic rotation of 180 degrees and a worst-case number of segments in
    // the join.
    //
    // An informal survey of skottie animations and gms revealed that even with a bare minimum of 64
    // tessellation segments, 99.9%+ of quadratics take this early out.
    float numParametricSegments_pow4 =
            GrWangsFormula::quadratic_pow4(fTolerances.fParametricIntolerance, p);
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
    float numRadialSegments = SkMeasureQuadRotation(p) * fTolerances.fNumRadialSegmentsPerRadian;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    float numParametricSegments = GrWangsFormula::root4(numParametricSegments_pow4);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    float numCombinedSegments = NumCombinedSegments(numParametricSegments, numRadialSegments);
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

void GrStrokeTessellateOp::cubicTo(const SkPoint p[4], JoinType prevJoinType,
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
    float numParametricSegments_pow4 =
            GrWangsFormula::cubic_pow4(fTolerances.fParametricIntolerance, p);
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
        if (int n = GrPathUtils::findCubicConvex180Chops(p, chopT)) {
            SkChopCubicAt(p, chops, chopT, n);
            this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth);
            for (int i = 1; i <= n; ++i) {
                // If we chopped at a cusp then rotation is not continuous between the two curves.
                // Insert a double cusp to make up for lost rotation.
                JoinType nextJoinType = (cubic_chop_is_cusp(chops + (i - 1)*3)) ?
                        JoinType::kCusp : JoinType::kFromStroke;
                this->cubicTo(chops + i*3, nextJoinType, Convex180Status::kYes, maxDepth);
            }
        } else {
            // The cubic was Convex180Status::kYes after all. Try again when we can use 180-degree
            // max segment limits instead of 360.
            this->cubicTo(p, prevJoinType, Convex180Status::kYes, maxDepth);
        }
        return;
    }

    // We still might have enough tessellation segments to render the curve. Check again with
    // its actual rotation.
    float numRadialSegments =
            SkMeasureNonInflectCubicRotation(p) * fTolerances.fNumRadialSegmentsPerRadian;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    float numParametricSegments = GrWangsFormula::root4(numParametricSegments_pow4);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    float numCombinedSegments = NumCombinedSegments(numParametricSegments, numRadialSegments);
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

void GrStrokeTessellateOp::joinTo(JoinType joinType, SkPoint nextControlPoint, int maxDepth) {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // The first stroke doesn't have a previous join.
        return;
    }

    if (!fSoloRoundJoinAlwaysFitsInPatch && maxDepth != 0 &&
        (fStroke.getJoin() == SkPaint::kRound_Join || joinType == JoinType::kCusp)) {
        SkVector tan0 = fCurrentPoint - fLastControlPoint;
        SkVector tan1 = nextControlPoint - fCurrentPoint;
        float rotation = SkMeasureAngleBetweenVectors(tan0, tan1);
        float numRadialSegments = rotation * fTolerances.fNumRadialSegmentsPerRadian;
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

void GrStrokeTessellateOp::close() {
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

void GrStrokeTessellateOp::cap() {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // We don't have any control points to orient the caps. In this case, square and round caps
        // are specified to be drawn as an axis-aligned square or circle respectively. Assign
        // default control points that achieve this.
        SkVector outset;
        if (!fStroke.isHairlineStyle()) {
            outset = {1, 0};
        } else {
            // If the stroke is hairline, orient the square on the post-transform x-axis instead.
            // We don't need to worry about the vector length since it will be normalized later.
            // Since the matrix cannot have perspective, the below is equivalent to:
            //
            //    outset = inverse(|a b|) * |1| * arbitrary_scale
            //                     |c d|    |0|
            //
            //    == 1/det * | d -b| * |1| * arbitrary_scale
            //               |-c  a|   |0|
            //
            //    == 1/det * | d| * arbitrary_scale
            //               |-c|
            //
            //    == | d|
            //       |-c|
            //
            SkASSERT(!fViewMatrix.hasPerspective());
            float c=fViewMatrix.getSkewY(), d=fViewMatrix.getScaleY();
            outset = {d, -c};
        }
        fCurrContourFirstControlPoint = fCurrContourStartPoint - outset;
        fLastControlPoint = fCurrContourStartPoint + outset;
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
            SkVector lastTangent = fCurrentPoint - fLastControlPoint;
            if (!fStroke.isHairlineStyle()) {
                // Extend the cap by 1/2 stroke width.
                lastTangent *= (.5f * fStroke.getWidth()) / lastTangent.length();
            } else {
                // Extend the cap by what will be 1/2 pixel after transformation.
                lastTangent *= .5f / fViewMatrix.mapVector(lastTangent.fX, lastTangent.fY).length();
            }
            this->lineTo(fCurrentPoint + lastTangent);
            this->moveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
            SkVector firstTangent = fCurrContourFirstControlPoint - fCurrContourStartPoint;
            if (!fStroke.isHairlineStyle()) {
                // Set the the cap back by 1/2 stroke width.
                firstTangent *= (-.5f * fStroke.getWidth()) / firstTangent.length();
            } else {
                // Set the cap back by what will be 1/2 pixel after transformation.
                firstTangent *=
                        -.5f / fViewMatrix.mapVector(firstTangent.fX, firstTangent.fY).length();
            }
            this->lineTo(fCurrContourStartPoint + firstTangent);
            break;
        }
    }

    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = false;)
}

void GrStrokeTessellateOp::cubicToRaw(JoinType prevJoinType, const SkPoint pts[4]) {
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

void GrStrokeTessellateOp::joinToRaw(JoinType joinType, SkPoint nextControlPoint) {
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

Patch* GrStrokeTessellateOp::reservePatch() {
    if (fPatchChunks.back().fPatchCount >= fCurrChunkPatchCapacity) {
        // The current chunk is full. Time to allocate a new one. (And no need to put back vertices;
        // the buffer is full.)
        this->allocPatchChunkAtLeast(fCurrChunkMinPatchAllocCount * 2);
    }
    if (!fCurrChunkPatchData) {
        SkDebugf("WARNING: Failed to allocate vertex buffer for tessellated stroke.");
        return nullptr;
    }
    SkASSERT(fPatchChunks.back().fPatchCount <= fCurrChunkPatchCapacity);
    Patch* patch = fCurrChunkPatchData + fPatchChunks.back().fPatchCount;
    ++fPatchChunks.back().fPatchCount;
    return patch;
}

void GrStrokeTessellateOp::allocPatchChunkAtLeast(int minPatchAllocCount) {
    PatchChunk* chunk = &fPatchChunks.push_back();
    fCurrChunkPatchData = (Patch*)fTarget->makeVertexSpaceAtLeast(sizeof(Patch), minPatchAllocCount,
                                                                  minPatchAllocCount,
                                                                  &chunk->fPatchBuffer,
                                                                  &chunk->fBasePatch,
                                                                  &fCurrChunkPatchCapacity);
    fCurrChunkMinPatchAllocCount = minPatchAllocCount;
}

void GrStrokeTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(chainBounds == this->bounds());
    if (fStencilProgram) {
        flushState->bindPipelineAndScissorClip(*fStencilProgram, this->bounds());
        flushState->bindTextures(fStencilProgram->primProc(), nullptr, fStencilProgram->pipeline());
        for (const auto& chunk : fPatchChunks) {
            if (chunk.fPatchBuffer) {
                flushState->bindBuffers(nullptr, nullptr, std::move(chunk.fPatchBuffer));
                flushState->draw(chunk.fPatchCount, chunk.fBasePatch);
            }
        }
    }
    if (fFillProgram) {
        flushState->bindPipelineAndScissorClip(*fFillProgram, this->bounds());
        flushState->bindTextures(fFillProgram->primProc(), nullptr, fFillProgram->pipeline());
        for (const auto& chunk : fPatchChunks) {
            if (chunk.fPatchBuffer) {
                flushState->bindBuffers(nullptr, nullptr, std::move(chunk.fPatchBuffer));
                flushState->draw(chunk.fPatchCount, chunk.fBasePatch);
            }
        }
    }
}
