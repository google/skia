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

using Tolerances = GrStrokeTessellateShader::Tolerances;

namespace {

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

class PatchWriter {
public:
    using ShaderFlags = GrStrokeTessellator::ShaderFlags;
    using PatchChunk = GrStrokeHardwareTessellator::PatchChunk;

    enum class JoinType {
        kMiter = SkPaint::kMiter_Join,
        kRound = SkPaint::kRound_Join,
        kBevel = SkPaint::kBevel_Join,
        kBowtie = SkPaint::kLast_Join + 1  // Double sided round join.
    };

    PatchWriter(ShaderFlags shaderFlags, GrMeshDrawOp::Target* target,
                SkTArray<PatchChunk>* patchChunks, int totalCombinedVerbCnt)
            : fShaderFlags(shaderFlags)
            , fTarget(target)
            , fPatchChunks(patchChunks)
            , fPatchStride(GrStrokeTessellateShader::PatchStride(fShaderFlags))
            // Subtract 2 because the tessellation shader chops every cubic at two locations, and
            // each chop has the potential to introduce an extra segment.
            , fMaxTessellationSegments(target->caps().shaderCaps()->maxTessellationSegments() - 2) {
        // Pre-allocate at least enough vertex space for 1 in 4 strokes to chop, and for 8 caps.
        int strokePreallocCount = totalCombinedVerbCnt * 5/4;
        int capPreallocCount = 8;
        this->allocPatchChunkAtLeast(strokePreallocCount + capPreallocCount);
    }

    ~PatchWriter() {
        fTarget->putBackVertices(fCurrChunkPatchCapacity - fPatchChunks->back().fPatchCount,
                                 fPatchStride);
    }

    void updateTolerances(Tolerances tolerances, SkPaint::Join joinType) {
        // Calculate the worst-case numbers of parametric segments our hardware can support for the
        // current stroke radius, in the event that there are also enough radial segments to rotate
        // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to
        // send almost all curves directly to the hardware without having to chop.
        float numRadialSegments180 = std::max(std::ceil(
                SK_ScalarPI * tolerances.fNumRadialSegmentsPerRadian), 1.f);
        float maxParametricSegments180 = num_parametric_segments(fMaxTessellationSegments,
                                                                 numRadialSegments180);
        fMaxParametricSegments180_pow4 = pow4(maxParametricSegments180);

        float numRadialSegments360 = std::max(std::ceil(
                2*SK_ScalarPI * tolerances.fNumRadialSegmentsPerRadian), 1.f);
        float maxParametricSegments360 = num_parametric_segments(fMaxTessellationSegments,
                                                                 numRadialSegments360);
        fMaxParametricSegments360_pow4 = pow4(maxParametricSegments360);

        // Now calculate the worst-case numbers of parametric segments if we are to integrate a join
        // into the same patch as the curve.
        float maxNumSegmentsInJoin;
        switch (joinType) {
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
        // Subtract an extra 1 off the end because when we integrate a join, the tessellator has to
        // add a redundant edge between the join and curve.
        fMaxParametricSegments180_pow4_withJoin = pow4(std::max(
                maxParametricSegments180 - maxNumSegmentsInJoin - 1, 0.f));
        fMaxParametricSegments360_pow4_withJoin = pow4(std::max(
                maxParametricSegments360 - maxNumSegmentsInJoin - 1, 0.f));
        fMaxCombinedSegments_withJoin = fMaxTessellationSegments - maxNumSegmentsInJoin - 1;
        fSoloRoundJoinAlwaysFitsInPatch = (numRadialSegments180 <= fMaxTessellationSegments);
        fTolerances = tolerances;
    }

    void updateDynamicStroke(const SkStrokeRec& stroke) {
        SkASSERT(fShaderFlags & ShaderFlags::kDynamicStroke);
        fDynamicStroke.set(stroke);
    }

    void updateDynamicColor(const SkPMColor4f& color) {
        SkASSERT(fShaderFlags & ShaderFlags::kDynamicColor);
        bool wideColor = fShaderFlags & ShaderFlags::kWideColor;
        SkASSERT(wideColor || color.fitsInBytes());
        fDynamicColor.set(color, wideColor);
    }

    void moveTo(SkPoint pt) {
        fCurrContourStartPoint = pt;
        fHasLastControlPoint = false;
    }

    void lineTo(JoinType prevJoinType, SkPoint p0, SkPoint p1) {
        // Zero-length paths need special treatment because they are spec'd to behave differently.
        if (p0 == p1) {
            return;
        }

        SkPoint asPatch[4] = {p0, p0, p1, p1};
        this->rawStrokeTo(prevJoinType, (fMaxCombinedSegments_withJoin >= 1), asPatch, p1);
    }

    void conicTo(JoinType prevJoinType, const SkPoint p[3], float w, int maxDepth = -1) {
        // Zero-length paths need special treatment because they are spec'd to behave differently.
        // If the control point is colocated on an endpoint then this might end up being the case.
        // Fall back on a lineTo and let it make the final check.
        if (p[1] == p[0] || p[1] == p[2] || w == 0) {
            this->lineTo(prevJoinType, p[0], p[2]);
            return;
        }

        // Convert to a patch.
        SkPoint asPatch[4];
        if (w == 1) {
            GrPathUtils::convertQuadToCubic(p, asPatch);
        } else {
            GrPathShader::WriteConicPatch(p, w, asPatch);
        }

        // Ensure our hardware supports enough tessellation segments to render the curve. This early
        // out assumes a worst-case quadratic rotation of 180 degrees and a worst-case number of
        // segments in the join.
        //
        // An informal survey of skottie animations and gms revealed that even with a bare minimum
        // of 64 tessellation segments, 99.9%+ of quadratics take this early out.
        float numParametricSegments_pow4 =
                GrWangsFormula::quadratic_pow4(fTolerances.fParametricIntolerance, p);
        if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4_withJoin) {
            this->rawStrokeTo(prevJoinType, /*prevJoinFitsInPatch=*/true, asPatch, p[2]);
            return;
        }

        if (numParametricSegments_pow4 <= fMaxParametricSegments180_pow4 || maxDepth == 0) {
            this->rawStrokeTo(prevJoinType,
                              (numParametricSegments_pow4 <=
                               fMaxParametricSegments180_pow4_withJoin), asPatch, p[2]);
            return;
        }

        // We still might have enough tessellation segments to render the curve. Check again with
        // the actual rotation.
        float numRadialSegments =
                SkMeasureQuadRotation(p) * fTolerances.fNumRadialSegmentsPerRadian;
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
                this->conicTo(prevJoinType, chops, 1, maxDepth - 1);
                this->conicTo(JoinType::kBowtie, chops + 2, 1, maxDepth - 1);
            } else {
                SkConic conic(p, w);
                float chopT = (numParametricSegments >= numRadialSegments) ? .5f
                                                                           : conic.findMidTangent();
                SkConic chops[2];
                if (conic.chopAt(chopT, chops)) {
                    this->conicTo(prevJoinType, chops[0].fPts, chops[0].fW, maxDepth - 1);
                    this->conicTo(JoinType::kBowtie, chops[1].fPts, chops[1].fW, maxDepth - 1);
                }
            }
            return;
        }

        this->rawStrokeTo(prevJoinType, (numCombinedSegments <= fMaxCombinedSegments_withJoin),
                          asPatch, p[2]);
    }

    // Is a cubic curve convex, and does it rotate no more than 180 degrees?
    enum class Convex180Status : bool {
        kUnknown,
        kYes
    };

    void cubicTo(JoinType prevJoinType, const SkPoint p[4],
                 Convex180Status convex180Status = Convex180Status::kUnknown, int maxDepth = -1) {
        // The stroke tessellation shader assigns special meaning to p0==p1==p2 and p1==p2==p3. If
        // this is the case then we need to rewrite the cubic.
        if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
            this->lineTo(prevJoinType, p[0], p[3]);
            return;
        }

        // Ensure our hardware supports enough tessellation segments to render the curve. This early
        // out assumes a worst-case cubic rotation of 360 degrees and a worst-case number of
        // segments in the join.
        //
        // An informal survey of skottie animations revealed that with a bare minimum of 64
        // tessellation segments, 95% of cubics take this early out.
        float numParametricSegments_pow4 =
                GrWangsFormula::cubic_pow4(fTolerances.fParametricIntolerance, p);
        if (numParametricSegments_pow4 <= fMaxParametricSegments360_pow4_withJoin) {
            this->rawStrokeTo(prevJoinType, /*prevJoinFitsInPatch=*/true, p, p[3]);
            return;
        }

        float maxParametricSegments_pow4 = (convex180Status == Convex180Status::kYes) ?
                fMaxParametricSegments180_pow4 : fMaxParametricSegments360_pow4;
        if (numParametricSegments_pow4 <= maxParametricSegments_pow4 || maxDepth == 0) {
            float maxParametricSegments_pow4_withJoin = (convex180Status == Convex180Status::kYes)
                    ? fMaxParametricSegments180_pow4_withJoin
                    : fMaxParametricSegments360_pow4_withJoin;
            this->rawStrokeTo(prevJoinType,
                              (numParametricSegments_pow4 <= maxParametricSegments_pow4_withJoin),
                              p, p[3]);
            return;
        }

        // Ensure the curve does not inflect or rotate >180 degrees before we start subdividing and
        // measuring rotation.
        if (convex180Status == Convex180Status::kUnknown) {
            this->cubicConvex180SegmentsTo(prevJoinType, p);
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
            this->cubicTo(prevJoinType, chops, Convex180Status::kYes, maxDepth - 1);
            this->cubicTo(JoinType::kBowtie, chops + 3, Convex180Status::kYes, maxDepth - 1);
            return;
        }

        this->rawStrokeTo(prevJoinType, (numCombinedSegments <= fMaxCombinedSegments_withJoin), p,
                          p[3]);
    }

    void cubicConvex180SegmentsTo(JoinType prevJoinType, const SkPoint p[4]) {
        SkPoint chops[10];
        float chopT[2];
        bool areCusps = false;
        int numChops = GrPathUtils::findCubicConvex180Chops(p, chopT, &areCusps);
        if (numChops == 0) {
            // The curve is already convex and rotates no more than 180 degrees.
            this->cubicTo(prevJoinType, p, Convex180Status::kYes);
        } else if (numChops == 1) {
            SkChopCubicAt(p, chops, chopT[0]);
            if (areCusps) {
                // When chopping on a perfect cusp, these 3 points will be equal.
                chops[2] = chops[4] = chops[3];
            }
            this->cubicTo(prevJoinType, chops, Convex180Status::kYes);
            this->cubicTo(JoinType::kBowtie, chops + 3, Convex180Status::kYes);
        } else {
            SkASSERT(numChops == 2);
            SkChopCubicAt(p, chops, chopT[0], chopT[1]);
            // Two cusps are only possible on a flat line with two 180-degree turnarounds.
            if (areCusps) {
                this->lineTo(prevJoinType, chops[0], chops[3]);
                this->lineTo(JoinType::kBowtie, chops[3], chops[6]);
                this->lineTo(JoinType::kBowtie, chops[6], chops[9]);
                return;
            }
            this->cubicTo(prevJoinType, chops, Convex180Status::kYes);
            this->cubicTo(JoinType::kBowtie, chops + 3, Convex180Status::kYes);
            this->cubicTo(JoinType::kBowtie, chops + 6, Convex180Status::kYes);
        }
    }

    void close(SkPoint contourEndpoint, const SkMatrix& viewMatrix, const SkStrokeRec& stroke) {
        if (!fHasLastControlPoint) {
            // Draw caps instead of closing if the subpath is zero length:
            //
            //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has
            //   a value of round or square producing respectively a circle or a square."
            //
            //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
            //
            this->cap(contourEndpoint, viewMatrix, stroke);
            return;
        }

        // Draw a line back to the beginning. (This will be discarded if
        // contourEndpoint == fCurrContourStartPoint.)
        auto strokeJoinType = JoinType(stroke.getJoin());
        this->lineTo(strokeJoinType, contourEndpoint, fCurrContourStartPoint);
        this->joinTo(strokeJoinType, fCurrContourStartPoint, fCurrContourFirstControlPoint);

        fHasLastControlPoint = false;
    }

    void cap(SkPoint contourEndpoint, const SkMatrix& viewMatrix, const SkStrokeRec& stroke) {
        if (!fHasLastControlPoint) {
            // We don't have any control points to orient the caps. In this case, square and round
            // caps are specified to be drawn as an axis-aligned square or circle respectively.
            // Assign default control points that achieve this.
            SkVector outset;
            if (!stroke.isHairlineStyle()) {
                outset = {1, 0};
            } else {
                // If the stroke is hairline, orient the square on the post-transform x-axis
                // instead. We don't need to worry about the vector length since it will be
                // normalized later. Since the matrix cannot have perspective, the below is
                // equivalent to:
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
                SkASSERT(!viewMatrix.hasPerspective());
                float c=viewMatrix.getSkewY(), d=viewMatrix.getScaleY();
                outset = {d, -c};
            }
            fCurrContourFirstControlPoint = fCurrContourStartPoint - outset;
            fLastControlPoint = fCurrContourStartPoint + outset;
            fHasLastControlPoint = true;
            contourEndpoint = fCurrContourStartPoint;
        }

        switch (stroke.getCap()) {
            case SkPaint::kButt_Cap:
                break;
            case SkPaint::kRound_Cap: {
                // A round cap is the same thing as a 180-degree round join.
                // If our join type isn't round we can alternatively use a bowtie.
                JoinType roundCapJoinType = (stroke.getJoin() == SkPaint::kRound_Join)
                        ? JoinType::kRound : JoinType::kBowtie;
                this->joinTo(roundCapJoinType, contourEndpoint, fLastControlPoint);
                this->moveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
                this->joinTo(roundCapJoinType, fCurrContourStartPoint,
                             fCurrContourFirstControlPoint);
                break;
            }
            case SkPaint::kSquare_Cap: {
                // A square cap is the same as appending lineTos.
                auto strokeJoinType = JoinType(stroke.getJoin());
                SkVector lastTangent = contourEndpoint - fLastControlPoint;
                if (!stroke.isHairlineStyle()) {
                    // Extend the cap by 1/2 stroke width.
                    lastTangent *= (.5f * stroke.getWidth()) / lastTangent.length();
                } else {
                    // Extend the cap by what will be 1/2 pixel after transformation.
                    lastTangent *=
                            .5f / viewMatrix.mapVector(lastTangent.fX, lastTangent.fY).length();
                }
                this->lineTo(strokeJoinType, contourEndpoint, contourEndpoint + lastTangent);
                this->moveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
                SkVector firstTangent = fCurrContourFirstControlPoint - fCurrContourStartPoint;
                if (!stroke.isHairlineStyle()) {
                    // Set the the cap back by 1/2 stroke width.
                    firstTangent *= (-.5f * stroke.getWidth()) / firstTangent.length();
                } else {
                    // Set the cap back by what will be 1/2 pixel after transformation.
                    firstTangent *=
                            -.5f / viewMatrix.mapVector(firstTangent.fX, firstTangent.fY).length();
                }
                this->lineTo(strokeJoinType, fCurrContourStartPoint,
                             fCurrContourStartPoint + firstTangent);
                break;
            }
        }

        fHasLastControlPoint = false;
    }

private:
    void moveTo(SkPoint pt, SkPoint lastControlPoint) {
        fCurrContourStartPoint = pt;
        fCurrContourFirstControlPoint = fLastControlPoint = lastControlPoint;
        fHasLastControlPoint = true;
    }

    void rawStrokeTo(JoinType prevJoinType, bool prevJoinFitsInPatch, const SkPoint p[4],
                    SkPoint endPt) {
        SkPoint c1 = (p[1] == p[0]) ? p[2] : p[1];
        SkPoint c2 = (p[2] == endPt) ? p[1] : p[2];

        if (prevJoinType == JoinType::kBowtie) {
            // Bowties need to go in their own patch if they will have >1 segment. TODO: Investigate
            // if an optimization like "x < fCosRadiansPerSegment" would be worth it.
            float rotation = SkMeasureAngleBetweenVectors(p[0] - fLastControlPoint, c1 - p[0]);
            if (rotation * fTolerances.fNumRadialSegmentsPerRadian > 1) {
                this->joinTo(prevJoinType, p[0], c1);
                fLastControlPoint = p[0];  // Disables the join section of this patch.
            }
        } else if (!fHasLastControlPoint) {
            // The first stroke doesn't have a previous join (yet). If the current contour ends up
            // closing itself, we will add that join as its own patch. TODO: Consider deferring the
            // first stroke until we know whether the contour will close. This will allow us to use
            // the closing join as the first patch's previous join.
            fHasLastControlPoint = true;
            fCurrContourFirstControlPoint = c1;
            fLastControlPoint = p[0];  // Disables the join section of this patch.
        } else if (!prevJoinFitsInPatch) {
            // The stroke has extremely thick round joins and there aren't enough guaranteed
            // segments to always combine a join with a line patch. Emit the join in its own
            // separate patch.
            this->joinTo(prevJoinType, p[0], c1);
            fLastControlPoint = p[0];  // Disables the join section of this patch.
        }

        if (this->reservePatch()) {
            fPatchWriter.write(fLastControlPoint);
            fPatchWriter.writeArray(p, 4);
            this->emitDynamicAttribs();
        }

        fLastControlPoint = c2;
    }

    void joinTo(JoinType joinType, SkPoint junctionPoint, SkPoint nextControlPoint,
                int maxDepth = -1) {
        if (!fHasLastControlPoint) {
            // The first stroke doesn't have a previous join.
            return;
        }

        if (!fSoloRoundJoinAlwaysFitsInPatch && maxDepth != 0 &&
            (joinType == JoinType::kRound || joinType == JoinType::kBowtie)) {
            SkVector tan0 = junctionPoint - fLastControlPoint;
            SkVector tan1 = nextControlPoint - junctionPoint;
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
                // FIXME(skia:11347): This hack ensures "c0 - junctionPoint" gives the exact same
                // ieee fp32 vector as "-(c1 - junctionPoint)". Tessellated stroking is becoming
                // less experimental, so t's time to think of a cleaner method to avoid T-junctions
                // when we chop joins.
                int maxAttempts = 10;
                do {
                    bisector = (junctionPoint + bisector) - (junctionPoint - bisector);
                    c0 = junctionPoint + bisector;
                    c1 = junctionPoint - bisector;
                } while (c0 - junctionPoint != -(c1 - junctionPoint) && --maxAttempts);
                this->joinTo(joinType, junctionPoint, c0, maxDepth - 1);  // First join half.
                fLastControlPoint = c1;
                // Second join half.
                this->joinTo(joinType, junctionPoint, nextControlPoint, maxDepth - 1);
                return;
            }
        }

        // We should never write out joins before the first curve.
        SkASSERT(fHasLastControlPoint);

        if (this->reservePatch()) {
            fPatchWriter.write(fLastControlPoint, junctionPoint);
            if (joinType == JoinType::kBowtie) {
                // {prevControlPoint, [p0, p0, p0, p3]} is a reserved patch pattern that means this
                // patch is a bowtie. The bowtie is anchored on p0 and its tangent angles go from
                // (p0 - prevControlPoint) to (p3 - p0).
                fPatchWriter.write(junctionPoint, junctionPoint);
            } else {
                // {prevControlPoint, [p0, p3, p3, p3]} is a reserved patch pattern that means this
                // patch is a join only (no curve sections in the patch). The join is anchored on p0 and
                // its tangent angles go from (p0 - prevControlPoint) to (p3 - p0).
                fPatchWriter.write(nextControlPoint, nextControlPoint);
            }
            fPatchWriter.write(nextControlPoint);
            this->emitDynamicAttribs();
        }

        fLastControlPoint = nextControlPoint;
    }

    void emitDynamicAttribs() {
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            fPatchWriter.write(fDynamicStroke);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            fPatchWriter.write(fDynamicColor);
        }
    }

    bool reservePatch() {
        if (fPatchChunks->back().fPatchCount >= fCurrChunkPatchCapacity) {
            // The current chunk is full. Time to allocate a new one. (And no need to put back
            // vertices; the buffer is full.)
            this->allocPatchChunkAtLeast(fCurrChunkMinPatchAllocCount * 2);
        }
        if (!fPatchWriter.isValid()) {
            SkDebugf("WARNING: Failed to allocate vertex buffer for tessellated stroke.");
            return false;
        }
        SkASSERT(fPatchChunks->back().fPatchCount <= fCurrChunkPatchCapacity);
        ++fPatchChunks->back().fPatchCount;
        return true;
    }

    void allocPatchChunkAtLeast(int minPatchAllocCount) {
        SkASSERT(fTarget);
        PatchChunk* chunk = &fPatchChunks->push_back();
        fPatchWriter = {fTarget->makeVertexSpaceAtLeast(fPatchStride, minPatchAllocCount,
                                                        minPatchAllocCount, &chunk->fPatchBuffer,
                                                        &chunk->fBasePatch,
                                                        &fCurrChunkPatchCapacity)};
        fCurrChunkMinPatchAllocCount = minPatchAllocCount;
    }

    const ShaderFlags fShaderFlags;
    GrMeshDrawOp::Target* const fTarget;
    SkTArray<PatchChunk>* const fPatchChunks;

    // Size in bytes of a tessellation patch with our shader flags.
    const size_t fPatchStride;

    // The maximum number of tessellation segments the hardware can emit for a single patch.
    const int fMaxTessellationSegments;

    // These values contain worst-case numbers of parametric segments, raised to the 4th power, that
    // our hardware can support for the current stroke radius. They assume curve rotations of 180
    // and 360 degrees respectively. These are used for "quick accepts" that allow us to send almost
    // all curves directly to the hardware without having to chop. We raise to the 4th power because
    // the "pow4" variants of Wang's formula are the quickest to evaluate.
    GrStrokeTessellateShader::Tolerances fTolerances;
    float fMaxParametricSegments180_pow4;
    float fMaxParametricSegments360_pow4;
    float fMaxParametricSegments180_pow4_withJoin;
    float fMaxParametricSegments360_pow4_withJoin;
    float fMaxCombinedSegments_withJoin;
    bool fSoloRoundJoinAlwaysFitsInPatch;

    // Variables related to the patch chunk that we are currently writing out during prepareBuffers.
    int fCurrChunkPatchCapacity;
    int fCurrChunkMinPatchAllocCount;
    GrVertexWriter fPatchWriter;

    // Variables related to the specific contour that we are currently iterating during
    // prepareBuffers().
    bool fHasLastControlPoint = false;
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;
    SkPoint fLastControlPoint;

    // Values for the current dynamic state (if any) that will get written out with each patch.
    GrStrokeTessellateShader::DynamicStroke fDynamicStroke;
    GrVertexColor fDynamicColor;
};

}  // namespace

static bool conic_has_cusp(const SkPoint p[3]) {
    SkVector a = p[1] - p[0];
    SkVector b = p[2] - p[1];
    // A conic of any class can only have a cusp if it is a degenerate flat line with a 180 degree
    // turnarund. To detect this, the beginning and ending tangents must be parallel
    // (a.cross(b) == 0) and pointing in opposite directions (a.dot(b) < 0).
    return a.cross(b) == 0 && a.dot(b) < 0;
}

void GrStrokeHardwareTessellator::prepare(GrMeshDrawOp::Target* target,
                                          const SkMatrix& viewMatrix) {
    using JoinType = PatchWriter::JoinType;

    std::array<float, 2> matrixScales;
    if (!viewMatrix.getMinMaxScales(matrixScales.data())) {
        matrixScales.fill(1);
    }

    PatchWriter patchWriter(fShaderFlags, target, &fPatchChunks, fTotalCombinedVerbCnt);
    const SkStrokeRec* strokeForTolerances = nullptr;

    for (PathStrokeList* pathStroke = fPathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        if (!strokeForTolerances || strokeForTolerances->getWidth() != stroke.getWidth() ||
            strokeForTolerances->getCap() != stroke.getCap()) {
            auto tolerances = Tolerances::MakePreTransform(matrixScales.data(), stroke.getWidth());
            patchWriter.updateTolerances(tolerances, stroke.getJoin());
            strokeForTolerances = &stroke;
        }
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            patchWriter.updateDynamicStroke(stroke);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            patchWriter.updateDynamicColor(pathStroke->fColor);
        }

        const SkPath& path = pathStroke->fPath;
        auto strokeJoinType = JoinType(stroke.getJoin());
        SkPathVerb previousVerb = SkPathVerb::kClose;
        for (auto [verb, p, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    // "A subpath ... consisting of a single moveto shall not be stroked."
                    // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                    if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
                        patchWriter.cap(p[-1], viewMatrix, stroke);
                    }
                    patchWriter.moveTo(p[0]);
                    break;
                case SkPathVerb::kLine:
                    patchWriter.lineTo(strokeJoinType, p[0], p[1]);
                    break;
                case SkPathVerb::kQuad:
                    if (conic_has_cusp(p)) {
                        SkPoint cusp = SkEvalQuadAt(p, SkFindQuadMidTangent(p));
                        patchWriter.lineTo(strokeJoinType, p[0], cusp);
                        patchWriter.lineTo(JoinType::kBowtie, cusp, p[2]);
                    } else {
                        patchWriter.conicTo(strokeJoinType, p, 1);
                    }
                    break;
                case SkPathVerb::kConic:
                    if (conic_has_cusp(p)) {
                        SkConic conic(p, *w);
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        patchWriter.lineTo(strokeJoinType, p[0], cusp);
                        patchWriter.lineTo(JoinType::kBowtie, cusp, p[2]);
                    } else {
                        patchWriter.conicTo(strokeJoinType, p, *w);
                    }
                    break;
                case SkPathVerb::kCubic:
                    bool areCusps;
                    GrPathUtils::findCubicConvex180Chops(p, nullptr, &areCusps);
                    if (areCusps) {
                        patchWriter.cubicConvex180SegmentsTo(strokeJoinType, p);
                    } else {
                        patchWriter.cubicTo(strokeJoinType, p);
                    }
                    break;
                case SkPathVerb::kClose:
                    patchWriter.close(p[0], viewMatrix, stroke);
                    break;
            }
            previousVerb = verb;
        }
        if (previousVerb != SkPathVerb::kMove && previousVerb != SkPathVerb::kClose) {
            const SkPoint* p = SkPathPriv::PointData(path);
            patchWriter.cap(p[path.countPoints() - 1], viewMatrix, stroke);
        }
    }
}

void GrStrokeHardwareTessellator::draw(GrOpFlushState* flushState) const {
    for (const auto& chunk : fPatchChunks) {
        if (chunk.fPatchBuffer) {
            flushState->bindBuffers(nullptr, nullptr, std::move(chunk.fPatchBuffer));
            flushState->draw(chunk.fPatchCount, chunk.fBasePatch);
        }
    }
}
