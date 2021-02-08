/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeHardwareTessellator.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

static float num_combined_segments(float numParametricSegments, float numRadialSegments) {
    // The first and last edges are shared by both the parametric and radial sets of edges, so
    // the total number of edges is:
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

GrStrokeHardwareTessellator::GrStrokeHardwareTessellator(const GrShaderCaps& shaderCaps,
                                                         const SkMatrix& viewMatrix,
                                                         const SkStrokeRec& stroke)
        // Subtract 2 because the tessellation shader chops every cubic at two locations, and each
        // chop has the potential to introduce an extra segment.
        : fMaxTessellationSegments(shaderCaps.maxTessellationSegments() - 2)
        , fStroke(stroke)
        , fTolerances(GrStrokeTessellateShader::Tolerances::MakePreTransform(viewMatrix, stroke)) {
    // Calculate the worst-case numbers of parametric segments our hardware can support for the
    // current stroke radius, in the event that there are also enough radial segments to rotate
    // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to
    // send almost all curves directly to the hardware without having to chop.
    float numRadialSegments180 = std::max(std::ceil(
            SK_ScalarPI * fTolerances.fNumRadialSegmentsPerRadian), 1.f);
    float maxParametricSegments180 = num_parametric_segments(fMaxTessellationSegments,
                                                             numRadialSegments180);
    fMaxParametricSegments180_pow4 = pow4(maxParametricSegments180);

    float numRadialSegments360 = std::max(std::ceil(
            2*SK_ScalarPI * fTolerances.fNumRadialSegmentsPerRadian), 1.f);
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
}

static bool conic_has_cusp(const SkPoint p[3]) {
    SkVector a = p[1] - p[0];
    SkVector b = p[2] - p[1];
    // A conic of any class can only have a cusp if it is a degenerate flat line with a 180 degree
    // turnarund. To detect this, the beginning and ending tangents must be parallel
    // (a.cross(b) == 0) and pointing in opposite directions (a.dot(b) < 0).
    return a.cross(b) == 0 && a.dot(b) < 0;
}

void GrStrokeHardwareTessellator::prepare(GrMeshDrawOp::Target* target, const SkMatrix& viewMatrix,
                                          const GrSTArenaList<SkPath>& pathList, const SkStrokeRec&,
                                          int totalCombinedVerbCnt) {
    fTarget = target;
    fViewMatrix = &viewMatrix;

    // Pre-allocate at least enough vertex space for 1 in 4 strokes to chop, and for 8 caps.
    int strokePreallocCount = totalCombinedVerbCnt * 5/4;
    int capPreallocCount = 8;
    this->allocPatchChunkAtLeast(strokePreallocCount + capPreallocCount);

    for (const SkPath& path : pathList) {
        fHasLastControlPoint = false;
        SkDEBUGCODE(fHasCurrentPoint = false;)
        SkPathVerb previousVerb = SkPathVerb::kClose;
        for (auto [verb, p, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    // "A subpath ... consisting of a single moveto shall not be stroked."
                    // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                    if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
                        this->cap();
                    }
                    this->moveTo(p[0]);
                    break;
                case SkPathVerb::kLine:
                    SkASSERT(fHasCurrentPoint);
                    SkASSERT(p[0] == fCurrentPoint);
                    this->lineTo(p[1]);
                    break;
                case SkPathVerb::kQuad:
                    if (conic_has_cusp(p)) {
                        SkPoint cusp = SkEvalQuadAt(p, SkFindQuadMidTangent(p));
                        this->lineTo(cusp);
                        this->lineTo(p[2], JoinType::kBowtie);
                    } else {
                        this->conicTo(p, 1);
                    }
                    break;
                case SkPathVerb::kConic:
                    if (conic_has_cusp(p)) {
                        SkConic conic(p, *w);
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        this->lineTo(cusp);
                        this->lineTo(p[2], JoinType::kBowtie);
                    } else {
                        this->conicTo(p, *w);
                    }
                    break;
                case SkPathVerb::kCubic:
                    bool areCusps;
                    GrPathUtils::findCubicConvex180Chops(p, nullptr, &areCusps);
                    if (areCusps) {
                        this->cubicConvex180SegmentsTo(p);
                    } else {
                        this->cubicTo(p);
                    }
                    break;
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

    fTarget = nullptr;
    fViewMatrix = nullptr;
}

void GrStrokeHardwareTessellator::moveTo(SkPoint pt) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokeHardwareTessellator::moveTo(SkPoint pt, SkPoint lastControlPoint) {
    fCurrentPoint = fCurrContourStartPoint = pt;
    fCurrContourFirstControlPoint = fLastControlPoint = lastControlPoint;
    fHasLastControlPoint = true;
    SkDEBUGCODE(fHasCurrentPoint = true;)
}

void GrStrokeHardwareTessellator::lineTo(SkPoint pt, JoinType prevJoinType) {
    SkASSERT(fHasCurrentPoint);

    // Zero-length paths need special treatment because they are spec'd to behave differently.
    if (pt == fCurrentPoint) {
        return;
    }

    if (fMaxCombinedSegments_withJoin < 1) {
        // The stroke has extremely thick round joins and there aren't enough guaranteed segments to
        // always combine a join with a line patch. Emit the join in its own separate patch.
        this->joinTo(prevJoinType, pt);
        prevJoinType = JoinType::kNone;
    }

    SkPoint asPatch[4] = {fCurrentPoint, fCurrentPoint, pt, pt};
    this->emitPatch(prevJoinType, asPatch, pt);
}

void GrStrokeHardwareTessellator::conicTo(const SkPoint p[3], float w, JoinType prevJoinType,
                                          int maxDepth) {
    SkASSERT(fHasCurrentPoint);
    SkASSERT(p[0] == fCurrentPoint);

    // Zero-length paths need special treatment because they are spec'd to behave differently. If
    // the control point is colocated on an endpoint then this might end up being the case. Fall
    // back on a lineTo and let it make the final check.
    if (p[1] == p[0] || p[1] == p[2] || w == 0) {
        this->lineTo(p[2], prevJoinType);
        return;
    }

    // Convert to a patch.
    SkPoint asPatch[4];
    if (w == 1) {
        GrPathUtils::convertQuadToCubic(p, asPatch);
    } else {
        GrPathShader::WriteConicPatch(p, w, asPatch);
    }

    // Ensure our hardware supports enough tessellation segments to render the curve. This early out
    // assumes a worst-case quadratic rotation of 180 degrees and a worst-case number of segments in
    // the join.
    //
    // An informal survey of skottie animations and gms revealed that even with a bare minimum of 64
    // tessellation segments, 99.9%+ of quadratics take this early out.
    float numParametricSegments_pow4 =
            GrWangsFormula::quadratic_pow4(fTolerances.fParametricIntolerance, p);
    if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4_withJoin) {
        this->emitPatch(prevJoinType, asPatch, p[2]);
        return;
    }

    if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4 || maxDepth == 0) {
        if (numParametricSegments_pow4 > fMaxParametricSegments180_pow4_withJoin) {
            // There aren't enough guaranteed segments to include the join. Emit a standalone patch
            // for the join.
            this->joinTo(prevJoinType, asPatch);
            prevJoinType = JoinType::kNone;
        }
        this->emitPatch(prevJoinType, asPatch, p[2]);
        return;
    }

    // We still might have enough tessellation segments to render the curve. Check again with the
    // actual rotation.
    float numRadialSegments = SkMeasureQuadRotation(p) * fTolerances.fNumRadialSegmentsPerRadian;
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
        if (w == 1) {
            SkPoint chops[5];
            if (numParametricSegments >= numRadialSegments) {
                SkChopQuadAtHalf(p, chops);
            } else {
                SkChopQuadAtMidTangent(p, chops);
            }
            this->conicTo(chops, 1, prevJoinType, maxDepth - 1);
            this->conicTo(chops + 2, 1, JoinType::kBowtie, maxDepth - 1);
        } else {
            SkConic conic(p, w);
            float chopT = (numParametricSegments >= numRadialSegments) ? .5f
                                                                       : conic.findMidTangent();
            SkConic chops[2];
            if (conic.chopAt(chopT, chops)) {
                this->conicTo(chops[0].fPts, chops[0].fW, prevJoinType, maxDepth - 1);
                this->conicTo(chops[1].fPts, chops[1].fW, JoinType::kBowtie, maxDepth - 1);
            }
        }
        return;
    }

    if (numCombinedSegments > fMaxCombinedSegments_withJoin) {
        // There aren't enough guaranteed segments to include the join. Emit a standalone patch for
        // the join.
        this->joinTo(prevJoinType, asPatch);
        prevJoinType = JoinType::kNone;
    }
    this->emitPatch(prevJoinType, asPatch, p[2]);
}

void GrStrokeHardwareTessellator::cubicTo(const SkPoint p[4], JoinType prevJoinType,
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
    if (numParametricSegments_pow4 <= fMaxParametricSegments360_pow4_withJoin) {
        this->emitPatch(prevJoinType, p, p[3]);
        return;
    }

    float maxParametricSegments_pow4 = (convex180Status == Convex180Status::kYes) ?
            fMaxParametricSegments180_pow4 : fMaxParametricSegments360_pow4;
    if (numParametricSegments_pow4 <= maxParametricSegments_pow4 || maxDepth == 0) {
        float maxParametricSegments_pow4_withJoin = (convex180Status == Convex180Status::kYes) ?
                fMaxParametricSegments180_pow4_withJoin : fMaxParametricSegments360_pow4_withJoin;
        if (numParametricSegments_pow4 > maxParametricSegments_pow4_withJoin) {
            // There aren't enough guaranteed segments to include the join. Emit a standalone patch
            // for the join.
            this->joinTo(prevJoinType, p);
            prevJoinType = JoinType::kNone;
        }
        this->emitPatch(prevJoinType, p, p[3]);
        return;
    }

    // Ensure the curve does not inflect or rotate >180 degrees before we start subdividing and
    // measuring rotation.
    if (convex180Status == Convex180Status::kUnknown) {
        this->cubicConvex180SegmentsTo(p, prevJoinType, maxDepth);
        return;
    }

    // We still might have enough tessellation segments to render the curve. Check again with
    // its actual rotation.
    float numRadialSegments =
            SkMeasureNonInflectCubicRotation(p) * fTolerances.fNumRadialSegmentsPerRadian;
    numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
    float numParametricSegments = GrWangsFormula::root4(numParametricSegments_pow4);
    numParametricSegments = std::max(std::ceil(numParametricSegments), 1.f);
    float numCombinedSegments = num_combined_segments(numParametricSegments, numRadialSegments);
    if (numCombinedSegments > fMaxTessellationSegments) {
        // The hardware doesn't support enough segments for this curve. Chop and recurse.
        SkPoint chops[7];
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
        this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth - 1);
        this->cubicTo(chops + 3, JoinType::kBowtie, Convex180Status::kYes, maxDepth - 1);
        return;
    }

    if (numCombinedSegments > fMaxCombinedSegments_withJoin) {
        // There aren't enough guaranteed segments to include the join. Emit a standalone patch for
        // the join.
        this->joinTo(prevJoinType, p);
        prevJoinType = JoinType::kNone;
    }
    this->emitPatch(prevJoinType, p, p[3]);
}

void GrStrokeHardwareTessellator::cubicConvex180SegmentsTo(const SkPoint p[4],
                                                           JoinType prevJoinType, int maxDepth) {
    SkPoint chops[10];
    float chopT[2];
    bool areCusps = false;
    int numChops = GrPathUtils::findCubicConvex180Chops(p, chopT, &areCusps);
    if (numChops == 0) {
        // The curve is already convex and rotates no more than 180 degrees.
        this->cubicTo(p, prevJoinType, Convex180Status::kYes, maxDepth);
    } else if (numChops == 1) {
        SkChopCubicAt(p, chops, chopT[0]);
        if (areCusps) {
            // When chopping on a perfect cusp, these 3 points will be equal.
            chops[2] = chops[4] = chops[3];
        }
        this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth);
        this->cubicTo(chops + 3, JoinType::kBowtie, Convex180Status::kYes, maxDepth);
    } else {
        SkASSERT(numChops == 2);
        SkChopCubicAt(p, chops, chopT[0], chopT[1]);
        // Two cusps are only possible on a flat line with two 180-degree turnarounds.
        if (areCusps) {
            this->lineTo(chops[3], prevJoinType);
            this->lineTo(chops[6], JoinType::kBowtie);
            this->lineTo(chops[9], JoinType::kBowtie);
            return;
        }
        this->cubicTo(chops, prevJoinType, Convex180Status::kYes, maxDepth);
        this->cubicTo(chops + 3, JoinType::kBowtie, Convex180Status::kYes, maxDepth);
        this->cubicTo(chops + 6, JoinType::kBowtie, Convex180Status::kYes, maxDepth);
    }
}

void GrStrokeHardwareTessellator::joinTo(JoinType joinType, SkPoint nextControlPoint,
                                         int maxDepth) {
    SkASSERT(fHasCurrentPoint);

    if (!fHasLastControlPoint) {
        // The first stroke doesn't have a previous join.
        return;
    }

    if (!fSoloRoundJoinAlwaysFitsInPatch && maxDepth != 0 &&
        (fStroke.getJoin() == SkPaint::kRound_Join || joinType == JoinType::kBowtie)) {
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

    this->emitJoinPatch(joinType, nextControlPoint);
}

void GrStrokeHardwareTessellator::close() {
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

void GrStrokeHardwareTessellator::cap() {
    SkASSERT(fViewMatrix);
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
            SkASSERT(!fViewMatrix->hasPerspective());
            float c=fViewMatrix->getSkewY(), d=fViewMatrix->getScaleY();
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
            // If our join type isn't round we can alternatively use a bowtie.
            JoinType roundCapJoinType = (fStroke.getJoin() == SkPaint::kRound_Join)
                    ? JoinType::kFromStroke : JoinType::kBowtie;
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
                lastTangent *=
                        .5f / fViewMatrix->mapVector(lastTangent.fX, lastTangent.fY).length();
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
                        -.5f / fViewMatrix->mapVector(firstTangent.fX, firstTangent.fY).length();
            }
            this->lineTo(fCurrContourStartPoint + firstTangent);
            break;
        }
    }

    fHasLastControlPoint = false;
    SkDEBUGCODE(fHasCurrentPoint = false;)
}

void GrStrokeHardwareTessellator::emitPatch(JoinType prevJoinType, const SkPoint p[4],
                                            SkPoint endPt) {
    SkPoint c1 = (p[1] == p[0]) ? p[2] : p[1];
    SkPoint c2 = (p[2] == endPt) ? p[1] : p[2];

    if (prevJoinType == JoinType::kBowtie) {
        // Bowties need to go in their own patch if they will have >1 segment.
        // TODO: Investigate if an optimization like "x < fCosRadiansPerSegment" would be worth it.
        float rotation = SkMeasureAngleBetweenVectors(p[0] - fLastControlPoint, c1 - p[0]);
        if (rotation * fTolerances.fNumRadialSegmentsPerRadian > 1) {
            this->joinTo(prevJoinType, c1);
            prevJoinType = JoinType::kNone;
        }
    }

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

    if (this->reservePatch()) {
        // Disable the join section of this patch if prevJoinType is kNone by setting the previous
        // control point equal to p0.
        fPatchWriter.write((prevJoinType == JoinType::kNone) ? p[0] : fLastControlPoint);
        fPatchWriter.writeArray(p, 4);
    }

    fLastControlPoint = c2;
    fCurrentPoint = endPt;
}

void GrStrokeHardwareTessellator::emitJoinPatch(JoinType joinType, SkPoint nextControlPoint) {
    // We should never write out joins before the first curve.
    SkASSERT(fHasLastControlPoint);
    SkASSERT(fHasCurrentPoint);

    if (this->reservePatch()) {
        fPatchWriter.write(fLastControlPoint, fCurrentPoint);
        if (joinType == JoinType::kFromStroke) {
            // [p0, p3, p3, p3] is a reserved pattern that means this patch is a join only (no cubic
            // sections in the patch).
            fPatchWriter.write(nextControlPoint, nextControlPoint);
        } else {
            SkASSERT(joinType == JoinType::kBowtie);
            // [p0, p0, p0, p3] is a reserved pattern that means this patch is a bowtie.
            fPatchWriter.write(fCurrentPoint, fCurrentPoint);
        }
        fPatchWriter.write(nextControlPoint);
    }

    fLastControlPoint = nextControlPoint;
}

bool GrStrokeHardwareTessellator::reservePatch() {
    if (fPatchChunks.back().fPatchCount >= fCurrChunkPatchCapacity) {
        // The current chunk is full. Time to allocate a new one. (And no need to put back vertices;
        // the buffer is full.)
        this->allocPatchChunkAtLeast(fCurrChunkMinPatchAllocCount * 2);
    }
    if (!fPatchWriter.isValid()) {
        SkDebugf("WARNING: Failed to allocate vertex buffer for tessellated stroke.");
        return false;
    }
    SkASSERT(fPatchChunks.back().fPatchCount <= fCurrChunkPatchCapacity);
    ++fPatchChunks.back().fPatchCount;
    return true;
}

void GrStrokeHardwareTessellator::allocPatchChunkAtLeast(int minPatchAllocCount) {
    SkASSERT(fTarget);
    PatchChunk* chunk = &fPatchChunks.push_back();
    fPatchWriter = {fTarget->makeVertexSpaceAtLeast(
            GrStrokeTessellateShader::kTessellationPatchBaseStride, minPatchAllocCount,
            minPatchAllocCount, &chunk->fPatchBuffer, &chunk->fBasePatch,
            &fCurrChunkPatchCapacity)};
    fCurrChunkMinPatchAllocCount = minPatchAllocCount;
}

void GrStrokeHardwareTessellator::draw(GrOpFlushState* flushState) const {
    for (const auto& chunk : fPatchChunks) {
        if (chunk.fPatchBuffer) {
            flushState->bindBuffers(nullptr, nullptr, std::move(chunk.fPatchBuffer));
            flushState->draw(chunk.fPatchCount, chunk.fBasePatch);
        }
    }
}
