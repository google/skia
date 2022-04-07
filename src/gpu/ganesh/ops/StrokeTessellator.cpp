/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/StrokeTessellator.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/StrokeIterator.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu::v1 {

namespace {

// Calculates and buffers up future values for "numRadialSegmentsPerRadian" using SIMD.
class alignas(sizeof(float) * 4) StrokeToleranceBuffer {
public:
    using PathStrokeList = StrokeTessellator::PathStrokeList;

    StrokeToleranceBuffer(float matrixMaxScale) : fMatrixMaxScale(matrixMaxScale) {}

    float fetchRadialSegmentsPerRadian(PathStrokeList* head) {
        // StrokeTessellateOp::onCombineIfPossible does not allow hairlines to become dynamic. If
        // this changes, we will need to call StrokeTolerances::GetLocalStrokeWidth() for each
        // stroke.
        SkASSERT(!head->fStroke.isHairlineStyle());
        if (fBufferIdx == 4) {
            // We ran out of values. Peek ahead and buffer up 4 more.
            PathStrokeList* peekAhead = head;
            int i = 0;
            do {
                fStrokeWidths[i++] = peekAhead->fStroke.getWidth();
            } while ((peekAhead = peekAhead->fNext) && i < 4);
            auto tol = StrokeTolerances::ApproxNumRadialSegmentsPerRadian(fMatrixMaxScale,
                                                                          fStrokeWidths);
            tol.store(fNumRadialSegmentsPerRadian);
            fBufferIdx = 0;
        }
        SkASSERT(0 <= fBufferIdx && fBufferIdx < 4);
        SkASSERT(fStrokeWidths[fBufferIdx] == head->fStroke.getWidth());
        return fNumRadialSegmentsPerRadian[fBufferIdx++];
    }

private:
    float4 fStrokeWidths{};  // Must be first for alignment purposes.
    float fNumRadialSegmentsPerRadian[4];
    const float fMatrixMaxScale;
    int fBufferIdx = 4;  // Initialize the buffer as "empty";
};

// *** Fixed-count tessellation stroking

using FixedCountStrokeWriter = PatchWriter<GrVertexChunkBuilder,
                                           Required<PatchAttribs::kJoinControlPoint>,
                                           Optional<PatchAttribs::kStrokeParams>,
                                           Optional<PatchAttribs::kColor>,
                                           Optional<PatchAttribs::kWideColorIfEnabled>,
                                           Optional<PatchAttribs::kExplicitCurveType>,
                                           TrackJoinControlPoints>;

int write_fixed_count_patches(FixedCountStrokeWriter&& patchWriter,
                              const SkMatrix& shaderMatrix,
                              std::array<float,2> matrixMinMaxScales,
                              StrokeTessellator::PathStrokeList* pathStrokeList) {
    int maxEdgesInJoin = 0;
    float maxRadialSegmentsPerRadian = 0;
    const float matrixMaxScale = matrixMinMaxScales[1];
    if (!(patchWriter.attribs() & PatchAttribs::kStrokeParams)) {
        // Strokes are static. Calculate tolerances once.
        const SkStrokeRec& stroke = pathStrokeList->fStroke;
        float localStrokeWidth = StrokeTolerances::GetLocalStrokeWidth(matrixMinMaxScales.data(),
                                                                       stroke.getWidth());
        float numRadialSegmentsPerRadian = StrokeTolerances::CalcNumRadialSegmentsPerRadian(
                matrixMaxScale, localStrokeWidth);
        maxEdgesInJoin = WorstCaseEdgesInJoin(stroke.getJoin(), numRadialSegmentsPerRadian);
        maxRadialSegmentsPerRadian = numRadialSegmentsPerRadian;
    }

    // Fast SIMD queue that buffers up values for "numRadialSegmentsPerRadian". Only used when we
    // have dynamic stroke.
    StrokeToleranceBuffer toleranceBuffer(matrixMaxScale);

    // The vector xform approximates how the control points are transformed by the shader to
    // more accurately compute how many *parametric* segments are needed.
    wangs_formula::VectorXform shaderXform{shaderMatrix};
    for (auto* pathStroke = pathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        if (patchWriter.attribs() & PatchAttribs::kStrokeParams) {
            // Strokes are dynamic. Calculate tolerances every time.
            float numRadialSegmentsPerRadian =
                    toleranceBuffer.fetchRadialSegmentsPerRadian(pathStroke);
            maxEdgesInJoin = std::max(
                    WorstCaseEdgesInJoin(stroke.getJoin(), numRadialSegmentsPerRadian),
                    maxEdgesInJoin);
            maxRadialSegmentsPerRadian = std::max(numRadialSegmentsPerRadian,
                                                  maxRadialSegmentsPerRadian);
            patchWriter.updateStrokeParamsAttrib(stroke);
        }
        if (patchWriter.attribs() & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(pathStroke->fColor);
        }

        StrokeIterator strokeIter(pathStroke->fPath, &pathStroke->fStroke, &shaderMatrix);
        while (strokeIter.next()) {
            using Verb = StrokeIterator::Verb;
            const SkPoint* p = strokeIter.pts();
            int numChops;
            switch (strokeIter.verb()) {
                case Verb::kContourFinished:
                    patchWriter.writeDeferredStrokePatch();
                    break;
                case Verb::kCircle:
                    // Round cap or else an empty stroke that is specified to be drawn as a circle.
                    patchWriter.writeCircle(p[0]);
                    [[fallthrough]];
                case Verb::kMoveWithinContour:
                    // A regular kMove invalidates the previous control point; the stroke iterator
                    // tells us a new value to use.
                    patchWriter.updateJoinControlPointAttrib(p[0]);
                    break;
                case Verb::kLine:
                    patchWriter.writeLine(p[0], p[1]);
                    break;
                case Verb::kQuad:
                    if (ConicHasCusp(p)) {
                        // The cusp is always at the midtandent.
                        SkPoint cusp = SkEvalQuadAt(p, SkFindQuadMidTangent(p));
                        patchWriter.writeCircle(cusp);
                        // A quad can only have a cusp if it's flat with a 180-degree turnaround.
                        patchWriter.writeLine(p[0], cusp);
                        patchWriter.writeLine(cusp, p[2]);
                    } else {
                        patchWriter.writeQuadratic(p, shaderXform);
                    }
                    break;
                case Verb::kConic:
                    if (ConicHasCusp(p)) {
                        // The cusp is always at the midtandent.
                        SkConic conic(p, strokeIter.w());
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        patchWriter.writeCircle(cusp);
                        // A conic can only have a cusp if it's flat with a 180-degree turnaround.
                        patchWriter.writeLine(p[0], cusp);
                        patchWriter.writeLine(cusp, p[2]);
                    } else {
                        patchWriter.writeConic(p, strokeIter.w(), shaderXform);
                    }
                    break;
                case Verb::kCubic:
                    SkPoint chops[10];
                    float T[2];
                    bool areCusps;
                    numChops = FindCubicConvex180Chops(p, T, &areCusps);
                    if (numChops == 0) {
                        patchWriter.writeCubic(p, shaderXform);
                    } else if (numChops == 1) {
                        SkChopCubicAt(p, chops, T[0]);
                        if (areCusps) {
                            patchWriter.writeCircle(chops[3]);
                            // In a perfect world, these 3 points would be be equal after chopping
                            // on a cusp.
                            chops[2] = chops[4] = chops[3];
                        }
                        patchWriter.writeCubic(chops, shaderXform);
                        patchWriter.writeCubic(chops + 3, shaderXform);
                    } else {
                        SkASSERT(numChops == 2);
                        SkChopCubicAt(p, chops, T[0], T[1]);
                        if (areCusps) {
                            patchWriter.writeCircle(chops[3]);
                            patchWriter.writeCircle(chops[6]);
                            // Two cusps are only possible if it's a flat line with two 180-degree
                            // turnarounds.
                            patchWriter.writeLine(chops[0], chops[3]);
                            patchWriter.writeLine(chops[3], chops[6]);
                            patchWriter.writeLine(chops[6], chops[9]);
                        } else {
                            patchWriter.writeCubic(chops, shaderXform);
                            patchWriter.writeCubic(chops + 3, shaderXform);
                            patchWriter.writeCubic(chops + 6, shaderXform);
                        }
                    }
                    break;
            }
        }
    }

    // The maximum rotation we can have in a stroke is 180 degrees (SK_ScalarPI radians).
    int maxRadialSegmentsInStroke =
            std::max(SkScalarCeilToInt(maxRadialSegmentsPerRadian * SK_ScalarPI), 1);

    int maxParametricSegmentsInStroke = patchWriter.requiredFixedSegments();
    SkASSERT(maxParametricSegmentsInStroke >= 1);

    // Now calculate the maximum number of edges we will need in the stroke portion of the instance.
    // The first and last edges in a stroke are shared by both the parametric and radial sets of
    // edges, so the total number of edges is:
    //
    //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
    //
    // It's also important to differentiate between the number of edges and segments in a strip:
    //
    //   numSegments = numEdges - 1
    //
    // So the total number of combined edges in the stroke is:
    //
    //   numEdgesInStroke = numParametricSegments + 1 + numRadialSegments + 1 - 2
    //                    = numParametricSegments + numRadialSegments
    //
    int maxEdgesInStroke = maxRadialSegmentsInStroke + maxParametricSegmentsInStroke;

    // Each triangle strip has two sections: It starts with a join then transitions to a stroke. The
    // number of edges in an instance is the sum of edges from the join and stroke sections both.
    // NOTE: The final join edge and the first stroke edge are co-located, however we still need to
    // emit both because the join's edge is half-width and the stroke's is full-width.
    return maxEdgesInJoin + maxEdgesInStroke;
}

// *** HW tessellation stroking ***

float num_combined_segments(float numParametricSegments, float numRadialSegments) {
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

float2 pow4(float2 x) {
    auto xx = x*x;
    return xx*xx;
}

void quad_to_cubic(const SkPoint p[3], SkPoint patch[4]) {
    // Matches arithmetic in PatchWriter::writeQuadratic's conversion to a cubic.
    float4 p0p2{skvx::bit_pun<float2>(p[0]), skvx::bit_pun<float2>(p[2])};
    float4 mid = mix(p0p2, skvx::bit_pun<float2>(p[1]).xyxy(), 2/3.f);

    patch[0] = p[0];
    mid.store(patch + 1); // also writes patch[2]
    patch[3] = p[2];
}

SK_ALWAYS_INLINE bool cubic_has_cusp(const SkPoint p[4]) {
    float2 p0 = skvx::bit_pun<float2>(p[0]);
    float2 p1 = skvx::bit_pun<float2>(p[1]);
    float2 p2 = skvx::bit_pun<float2>(p[2]);
    float2 p3 = skvx::bit_pun<float2>(p[3]);

    // See FindCubicConvex180Chops() for the math.
    float2 C = p1 - p0;
    float2 D = p2 - p1;
    float2 E = p3 - p0;
    float2 B = D - C;
    float2 A = -3*D + E;

    float a = cross(A, B);
    float b = cross(A, C);
    float c = cross(B, C);
    float discr = b*b - 4*a*c;

    // If -cuspThreshold <= discr <= cuspThreshold, it means the two roots are within a distance of
    // 2^-11 from one another in parametric space. This is close enough for our purposes to take the
    // slow codepath that knows how to handle cusps.
    constexpr static float kEpsilon = 1.f / (1 << 11);
    float cuspThreshold = (2*kEpsilon) * a;
    cuspThreshold *= cuspThreshold;

    return fabsf(discr) <= cuspThreshold &&
           // The most common type of cusp we encounter is when p0==p1 or p2==p3. Unless the curve
           // is a flat line (a==b==c==0), these don't actually need special treatment because the
           // cusp occurs at t=0 or t=1.
           (!(skvx::all(p0 == p1) || skvx::all(p2 == p3)) || (a == 0 && b == 0 && c == 0));
}

using HwPatchWriterBase = PatchWriter<GrVertexChunkBuilder,
                                      Required<PatchAttribs::kJoinControlPoint>,
                                      Optional<PatchAttribs::kStrokeParams>,
                                      Optional<PatchAttribs::kColor>,
                                      Optional<PatchAttribs::kWideColorIfEnabled>>;

class HwPatchWriter : HwPatchWriterBase {
public:
    enum class JoinType {
        kMiter = SkPaint::kMiter_Join,
        kRound = SkPaint::kRound_Join,
        kBevel = SkPaint::kBevel_Join,
        kBowtie = SkPaint::kLast_Join + 1  // Double sided round join.
    };

    HwPatchWriter(GrMeshDrawTarget* target,
                  GrVertexChunkArray* vertexChunkArray,
                  PatchAttribs attribs,
                  int maxTessellationSegments,
                  int preallocCount,
                  float matrixMaxScale)
            : HwPatchWriterBase(attribs, 0, target, vertexChunkArray, preallocCount)
            // Subtract 2 because the tessellation shader chops every cubic at two locations, and
            // each chop has the potential to introduce an extra segment.
            , fMaxTessellationSegments(std::max(maxTessellationSegments - 2, 1))
            , fParametricPrecision(matrixMaxScale * kTessellationPrecision)
            , fHasJoinControlPoint(false) {}

    using PatchWriter::attribs;
    using PatchWriter::updateColorAttrib;

    // This is the precision value, adjusted for the view matrix, to use with Wang's formulas when
    // determining how many parametric segments a curve will require.
    float parametricPrecision() const {
        return fParametricPrecision;
    }
    // Will a line and worst-case previous join both fit in a single patch together?
    bool lineFitsInPatch_withJoin() {
        return fMaxCombinedSegments_withJoin >= 1;
    }
    // Will a stroke with the given number of parametric segments and a worst-case rotation of 180
    // degrees fit in a single patch?
    bool stroke180FitsInPatch(float numParametricSegments_pow4) {
        return numParametricSegments_pow4 <= fMaxParametricSegments_pow4[0];
    }
    // Will a worst-case 180-degree stroke with the given number of parametric segments, and a
    // worst-case join fit in a single patch together?
    bool stroke180FitsInPatch_withJoin(float numParametricSegments_pow4) {
        return numParametricSegments_pow4 <= fMaxParametricSegments_pow4_withJoin[0];
    }
    // Will a stroke with the given number of parametric segments and a worst-case rotation of 360
    // degrees fit in a single patch?
    bool stroke360FitsInPatch(float numParametricSegments_pow4) {
        return numParametricSegments_pow4 <= fMaxParametricSegments_pow4[1];
    }
    // Will a worst-case 360-degree stroke with the given number of parametric segments, and a
    // worst-case join fit in a single patch together?
    bool stroke360FitsInPatch_withJoin(float numParametricSegments_pow4) {
        return numParametricSegments_pow4 <= fMaxParametricSegments_pow4_withJoin[1];
    }

    void updateTolerances(float numRadialSegmentsPerRadian, const SkStrokeRec& stroke) {
        SkPaint::Join joinType = stroke.getJoin();
        fNumRadialSegmentsPerRadian = numRadialSegmentsPerRadian;

        // Calculate the worst-case numbers of parametric segments our hardware can support for the
        // current stroke radius, in the event that there are also enough radial segments to rotate
        // 180 and 360 degrees respectively. These are used for "quick accepts" that allow us to
        // send almost all curves directly to the hardware without having to chop.
        float2 numRadialSegments_180_360 = skvx::max(skvx::ceil(
                float2{SK_ScalarPI, 2*SK_ScalarPI} * fNumRadialSegmentsPerRadian), 1);
        // numEdges = numSegments + 1. See num_combined_segments().
        float maxTotalEdges = fMaxTessellationSegments + 1;
        // numParametricSegments = numTotalEdges - numRadialSegments. See num_combined_segments().
        float2 maxParametricSegments = skvx::max(maxTotalEdges - numRadialSegments_180_360, 0);
        float2 maxParametricSegments_pow4 = pow4(maxParametricSegments);
        maxParametricSegments_pow4.store(fMaxParametricSegments_pow4);

        // Find the worst-case numbers of parametric segments if we are to integrate a join into the
        // same patch as the curve.
        float numRadialSegments180 = numRadialSegments_180_360[0];
        float worstCaseNumSegmentsInJoin;
        switch (joinType) {
            case SkPaint::kBevel_Join: worstCaseNumSegmentsInJoin = 1; break;
            case SkPaint::kMiter_Join: worstCaseNumSegmentsInJoin = 2; break;
            case SkPaint::kRound_Join: worstCaseNumSegmentsInJoin = numRadialSegments180; break;
        }

        // Now calculate the worst-case numbers of parametric segments if we also want to combine a
        // join with the patch. Subtract an extra 1 off the end because when we integrate a join,
        // the tessellator has to add a redundant edge between the join and curve.
        float2 maxParametricSegments_pow4_withJoin = pow4(skvx::max(
                maxParametricSegments - worstCaseNumSegmentsInJoin - 1, 0));
        maxParametricSegments_pow4_withJoin.store(fMaxParametricSegments_pow4_withJoin);

        fMaxCombinedSegments_withJoin = fMaxTessellationSegments - worstCaseNumSegmentsInJoin - 1;
        fSoloRoundJoinAlwaysFitsInPatch = (numRadialSegments180 <= fMaxTessellationSegments);
        fStrokeJoinType = JoinType(joinType);

        if (this->attribs() & PatchAttribs::kStrokeParams) {
            this->updateStrokeParamsAttrib(stroke);
        }
    }

    void updateJoinControlPoint(SkPoint p) {
        this->updateJoinControlPointAttrib(p);
        fJoinControlPoint = p;
        fHasJoinControlPoint = true;
    }

    void moveTo(SkPoint pt) {
        fCurrContourStartPoint = pt;
        fHasJoinControlPoint = false;
    }

    // Writes out the given line, possibly chopping its previous join until the segments fit in
    // tessellation patches.
    void writeLineTo(SkPoint p0, SkPoint p1) {
        this->writeLineTo(fStrokeJoinType, p0, p1);
    }
    void writeLineTo(JoinType prevJoinType, SkPoint p0, SkPoint p1) {
        // Zero-length paths need special treatment because they are spec'd to behave differently.
        if (p0 == p1) {
            return;
        }
        SkPoint asPatch[4] = {p0, p0, p1, p1};
        this->internalPatchTo(prevJoinType, this->lineFitsInPatch_withJoin(), asPatch, p1);
    }

    // Recursively chops the given conic and its previous join until the segments fit in
    // tessellation patches.
    void writeConicPatchesTo(const SkPoint p[3], float w) {
        this->internalConicPatchesTo(fStrokeJoinType, p, w);
    }

    // Chops the given cubic at points of inflection and 180-degree rotation, and then recursively
    // chops the previous join and cubic sections as necessary until the segments fit in
    // tessellation patches.
    void writeCubicConvex180PatchesTo(const SkPoint p[4]) {
        SkPoint chops[10];
        float chopT[2];
        bool areCusps;
        int numChops = FindCubicConvex180Chops(p, chopT, &areCusps);
        if (numChops == 0) {
            // The curve is already convex and rotates no more than 180 degrees.
            this->internalCubicConvex180PatchesTo(fStrokeJoinType, p);
        } else if (numChops == 1) {
            SkChopCubicAt(p, chops, chopT[0]);
            if (areCusps) {
                // When chopping on a perfect cusp, these 3 points will be equal.
                chops[2] = chops[4] = chops[3];
            }
            this->internalCubicConvex180PatchesTo(fStrokeJoinType, chops);
            this->internalCubicConvex180PatchesTo(JoinType::kBowtie, chops + 3);
        } else {
            SkASSERT(numChops == 2);
            SkChopCubicAt(p, chops, chopT[0], chopT[1]);
            // Two cusps are only possible on a flat line with two 180-degree turnarounds.
            if (areCusps) {
                this->writeLineTo(chops[0], chops[3]);
                this->writeLineTo(JoinType::kBowtie, chops[3], chops[6]);
                this->writeLineTo(JoinType::kBowtie, chops[6], chops[9]);
                return;
            }
            this->internalCubicConvex180PatchesTo(fStrokeJoinType, chops);
            this->internalCubicConvex180PatchesTo(JoinType::kBowtie, chops + 3);
            this->internalCubicConvex180PatchesTo(JoinType::kBowtie, chops + 6);
        }
    }

    // Writes out the given stroke patch exactly as provided, without chopping or checking the
    // number of segments. Possibly chops its previous join until the segments fit in tessellation
    // patches.
    SK_ALWAYS_INLINE void writePatchTo(bool prevJoinFitsInPatch, const SkPoint p[4],
                                       SkPoint endControlPoint) {
        SkASSERT(fStrokeJoinType != JoinType::kBowtie);

        if (!fHasJoinControlPoint) {
            // The first stroke doesn't have a previous join (yet). If the current contour ends up
            // closing itself, we will add that join as its own patch. TODO: Consider deferring the
            // first stroke until we know whether the contour will close. This will allow us to use
            // the closing join as the first patch's previous join.
            fCurrContourFirstControlPoint = (p[1] != p[0]) ? p[1] : p[2];
            // Disables the join section of this patch.
            this->updateJoinControlPoint(p[0]);
        } else if (!prevJoinFitsInPatch) {
            // There aren't enough guaranteed segments to fold the previous join into this patch.
            // Emit the join in its own separate patch.
            this->internalJoinTo(fStrokeJoinType, p[0], (p[1] != p[0]) ? p[1] : p[2]);
            // Disables the join section of this patch.
            this->updateJoinControlPoint(p[0]);
        }

        this->writePatch(skvx::bit_pun<float2>(p[0]), skvx::bit_pun<float2>(p[1]),
                         skvx::bit_pun<float2>(p[2]), skvx::bit_pun<float2>(p[3]),
                         /*unused*/0.f);
        this->updateJoinControlPoint(endControlPoint);
    }

    void writeClose(SkPoint contourEndpoint, const SkMatrix& viewMatrix,
                    const SkStrokeRec& stroke) {
        if (!fHasJoinControlPoint) {
            // Draw caps instead of closing if the subpath is zero length:
            //
            //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has
            //   a value of round or square producing respectively a circle or a square."
            //
            //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
            //
            this->writeCaps(contourEndpoint, viewMatrix, stroke);
            return;
        }

        // Draw a line back to the beginning. (This will be discarded if
        // contourEndpoint == fCurrContourStartPoint.)
        this->writeLineTo(contourEndpoint, fCurrContourStartPoint);
        this->internalJoinTo(fStrokeJoinType,
                             fCurrContourStartPoint,
                             fCurrContourFirstControlPoint);
        fHasJoinControlPoint = false;
    }

    void writeCaps(SkPoint contourEndpoint, const SkMatrix& viewMatrix, const SkStrokeRec& stroke) {
        if (!fHasJoinControlPoint) {
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
            contourEndpoint = fCurrContourStartPoint;
            this->updateJoinControlPoint(fCurrContourStartPoint + outset);
        }

        switch (stroke.getCap()) {
            case SkPaint::kButt_Cap:
                break;
            case SkPaint::kRound_Cap: {
                // A round cap is the same thing as a 180-degree round join.
                // If our join type isn't round we can alternatively use a bowtie.
                JoinType roundCapJoinType = (stroke.getJoin() == SkPaint::kRound_Join)
                        ? JoinType::kRound : JoinType::kBowtie;
                this->internalJoinTo(roundCapJoinType, contourEndpoint, fJoinControlPoint);
                this->internalMoveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
                this->internalJoinTo(roundCapJoinType, fCurrContourStartPoint,
                                     fCurrContourFirstControlPoint);
                break;
            }
            case SkPaint::kSquare_Cap: {
                // A square cap is the same as appending lineTos.
                auto strokeJoinType = JoinType(stroke.getJoin());
                SkVector lastTangent = contourEndpoint - fJoinControlPoint;
                if (!stroke.isHairlineStyle()) {
                    // Extend the cap by 1/2 stroke width.
                    lastTangent *= (.5f * stroke.getWidth()) / lastTangent.length();
                } else {
                    // Extend the cap by what will be 1/2 pixel after transformation.
                    lastTangent *=
                            .5f / viewMatrix.mapVector(lastTangent.fX, lastTangent.fY).length();
                }
                this->writeLineTo(strokeJoinType, contourEndpoint, contourEndpoint + lastTangent);
                this->internalMoveTo(fCurrContourStartPoint, fCurrContourFirstControlPoint);
                SkVector firstTangent = fCurrContourFirstControlPoint - fCurrContourStartPoint;
                if (!stroke.isHairlineStyle()) {
                    // Set the the cap back by 1/2 stroke width.
                    firstTangent *= (-.5f * stroke.getWidth()) / firstTangent.length();
                } else {
                    // Set the cap back by what will be 1/2 pixel after transformation.
                    firstTangent *=
                            -.5f / viewMatrix.mapVector(firstTangent.fX, firstTangent.fY).length();
                }
                this->writeLineTo(strokeJoinType, fCurrContourStartPoint,
                                  fCurrContourStartPoint + firstTangent);
                break;
            }
        }

        fHasJoinControlPoint = false;
    }

private:
    void internalMoveTo(SkPoint pt, SkPoint lastControlPoint) {
        fCurrContourStartPoint = pt;
        fCurrContourFirstControlPoint = lastControlPoint;
        this->updateJoinControlPoint(lastControlPoint);
    }

    // Recursively chops the given conic and its previous join until the segments fit in
    // tessellation patches.
    void internalConicPatchesTo(JoinType prevJoinType, const SkPoint p[3], float w,
                                int maxDepth = -1) {
        // Zero-length paths need special treatment because they are spec'd to behave differently.
        // If the control point is colocated on an endpoint then this might end up being the case.
        // Fall back on a lineTo and let it make the final check.
        if (p[1] == p[0] || p[1] == p[2] || w == 0) {
            this->writeLineTo(prevJoinType, p[0], p[2]);
            return;
        }

        // Convert to a patch.
        SkPoint asPatch[4];
        if (w == 1) {
            quad_to_cubic(p, asPatch);
        } else {
            memcpy(asPatch, p, sizeof(SkPoint) * 3);
            asPatch[3] = {w, std::numeric_limits<float>::infinity()};
        }

        float numParametricSegments_pow4;
        if (w == 1) {
            numParametricSegments_pow4 = wangs_formula::quadratic_pow4(fParametricPrecision, p);
        } else {
            float n = wangs_formula::conic_pow2(fParametricPrecision, p, w);
            numParametricSegments_pow4 = n*n;
        }
        if (this->stroke180FitsInPatch(numParametricSegments_pow4) || maxDepth == 0) {
            this->internalPatchTo(prevJoinType,
                                  this->stroke180FitsInPatch_withJoin(numParametricSegments_pow4),
                                  asPatch, p[2]);
            return;
        }

        // We still might have enough tessellation segments to render the curve. Check again with
        // the actual rotation.
        float numRadialSegments = SkMeasureQuadRotation(p) * fNumRadialSegmentsPerRadian;
        numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
        float numParametricSegments = wangs_formula::root4(numParametricSegments_pow4);
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
                this->internalConicPatchesTo(prevJoinType, chops, 1, maxDepth - 1);
                this->internalConicPatchesTo(JoinType::kBowtie, chops + 2, 1, maxDepth - 1);
            } else {
                SkConic conic(p, w);
                float chopT = (numParametricSegments >= numRadialSegments) ? .5f
                                                                           : conic.findMidTangent();
                SkConic chops[2];
                if (conic.chopAt(chopT, chops)) {
                    this->internalConicPatchesTo(prevJoinType, chops[0].fPts, chops[0].fW,
                                                  maxDepth - 1);
                    this->internalConicPatchesTo(JoinType::kBowtie, chops[1].fPts, chops[1].fW,
                                                  maxDepth - 1);
                }
            }
            return;
        }

        this->internalPatchTo(prevJoinType, (numCombinedSegments <= fMaxCombinedSegments_withJoin),
                              asPatch, p[2]);
    }

    // Recursively chops the given cubic and its previous join until the segments fit in
    // tessellation patches. The cubic must be convex and must not rotate more than 180 degrees.
    void internalCubicConvex180PatchesTo(JoinType prevJoinType, const SkPoint p[4],
                                         int maxDepth = -1) {
        // The stroke tessellation shader assigns special meaning to p0==p1==p2 and p1==p2==p3. If
        // this is the case then we need to rewrite the cubic.
        if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
            this->writeLineTo(prevJoinType, p[0], p[3]);
            return;
        }

        float numParametricSegments_pow4 = wangs_formula::cubic_pow4(fParametricPrecision, p);
        if (this->stroke180FitsInPatch(numParametricSegments_pow4) || maxDepth == 0) {
            this->internalPatchTo(prevJoinType,
                                  this->stroke180FitsInPatch_withJoin(numParametricSegments_pow4),
                                  p, p[3]);
            return;
        }

        // We still might have enough tessellation segments to render the curve. Check again with
        // its actual rotation.
        float numRadialSegments = SkMeasureNonInflectCubicRotation(p) * fNumRadialSegmentsPerRadian;
        numRadialSegments = std::max(std::ceil(numRadialSegments), 1.f);
        float numParametricSegments = wangs_formula::root4(numParametricSegments_pow4);
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
            this->internalCubicConvex180PatchesTo(prevJoinType, chops, maxDepth - 1);
            this->internalCubicConvex180PatchesTo(JoinType::kBowtie, chops + 3, maxDepth - 1);
            return;
        }

        this->internalPatchTo(prevJoinType, (numCombinedSegments <= fMaxCombinedSegments_withJoin),
                              p, p[3]);
    }

    // Writes out the given stroke patch exactly as provided, without chopping or checking the
    // number of segments. Possibly chops its previous join until the segments fit in tessellation
    // patches. It is valid for prevJoinType to be kBowtie.
    void internalPatchTo(JoinType prevJoinType, bool prevJoinFitsInPatch, const SkPoint p[4],
                         SkPoint endPt) {
        if (prevJoinType == JoinType::kBowtie) {
            SkASSERT(fHasJoinControlPoint);
            // Bowtie joins are only used on internal chops, and internal chops almost always have
            // continuous tangent angles (i.e., the ending tangent of the first chop and the
            // beginning tangent of the second both point in the same direction). The tangents will
            // only ever not point in the same direction if we chopped at a cusp point, so that's
            // the only time we actually need a bowtie.
            SkPoint nextControlPoint = (p[1] == p[0]) ? p[2] : p[1];
            SkVector a = p[0] - fJoinControlPoint;
            SkVector b = nextControlPoint - p[0];
            float ab_cosTheta = a.dot(b);
            float ab_pow2 = a.dot(a) * b.dot(b);
            // To check if tangents 'a' and 'b' do not point in the same direction, any of the
            // following formulas work:
            //
            //          0 != theta
            //          1 != cosTheta
            //          1 != cosTheta * abs(cosTheta)  [Still false when cosTheta == -1]
            //
            // Introducing a slop term for fuzzy equality gives:
            //
            //          1 !~= cosTheta * abs(cosTheta)                [tolerance = epsilon]
            //     (ab)^2 !~= (ab)^2 * cosTheta * abs(cosTheta)       [tolerance = (ab)^2 * epsilon]
            //     (ab)^2 !~= (ab * cosTheta) * (ab * abs(cosTheta))  [tolerance = (ab)^2 * epsilon]
            //     (ab)^2 !~= (ab * cosTheta) * abs(ab * cosTheta)    [tolerance = (ab)^2 * epsilon]
            //
            // Since we also scale the tolerance, the formula is unaffected by the magnitude of the
            // tangent vectors. (And we can fold "ab" in to the abs() because it's always positive.)
            if (!SkScalarNearlyEqual(ab_pow2, ab_cosTheta * fabsf(ab_cosTheta),
                                     ab_pow2 * SK_ScalarNearlyZero)) {
                this->internalJoinTo(JoinType::kBowtie, p[0], nextControlPoint);
                // Disables the join section of this patch.
                this->updateJoinControlPoint(p[0]);
                prevJoinFitsInPatch = true;
            }
        }

        this->writePatchTo(prevJoinFitsInPatch, p, (p[2] != endPt) ? p[2] : p[1]);
    }

    // Recursively chops the given join until the segments fit in tessellation patches.
    void internalJoinTo(JoinType joinType, SkPoint junctionPoint, SkPoint nextControlPoint,
                        int maxDepth = -1) {
        if (!fHasJoinControlPoint) {
            // The first stroke doesn't have a previous join.
            return;
        }

        if (!fSoloRoundJoinAlwaysFitsInPatch && maxDepth != 0 &&
            (joinType == JoinType::kRound || joinType == JoinType::kBowtie)) {
            SkVector tan0 = junctionPoint - fJoinControlPoint;
            SkVector tan1 = nextControlPoint - junctionPoint;
            float rotation = SkMeasureAngleBetweenVectors(tan0, tan1);
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
                // First join half.
                this->internalJoinTo(joinType, junctionPoint, c0, maxDepth - 1);
                this->updateJoinControlPoint(c1);
                // Second join half.
                this->internalJoinTo(joinType, junctionPoint, nextControlPoint, maxDepth - 1);
                return;
            }
        }

        // We should never write out joins before the first curve.
        SkASSERT(fHasJoinControlPoint);

        {
            SkPoint midpoint;
            if (joinType == JoinType::kBowtie) {
                // {prevControlPoint, [p0, p0, p0, p3]} is a reserved patch pattern that means this
                // patch is a bowtie. The bowtie is anchored on p0 and its tangent angles go from
                // (p0 - prevControlPoint) to (p3 - p0).
                midpoint = junctionPoint;
            } else {
                // {prevControlPoint, [p0, p3, p3, p3]} is a reserved patch pattern that means this
                // patch is a join only (no curve sections in the patch). The join is anchored on p0
                // and its tangent angles go from (p0 - prevControlPoint) to (p3 - p0).
                midpoint = nextControlPoint;
            }

            this->writePatch(skvx::bit_pun<float2>(junctionPoint),
                             skvx::bit_pun<float2>(midpoint),
                             skvx::bit_pun<float2>(midpoint),
                             skvx::bit_pun<float2>(nextControlPoint),
                             /*unused*/0.f);
        }

        this->updateJoinControlPoint(nextControlPoint);
    }

    // The maximum number of tessellation segments the hardware can emit for a single patch.
    const int fMaxTessellationSegments;

    // This is the precision value, adjusted for the view matrix, to use with Wang's formulas when
    // determining how many parametric segments a curve will require.
    const float fParametricPrecision;

    // Number of radial segments required for each radian of rotation in order to look smooth with
    // the current stroke radius.
    float fNumRadialSegmentsPerRadian;

    // These arrays contain worst-case numbers of parametric segments, raised to the 4th power, that
    // our hardware can support for the current stroke radius. They assume curve rotations of 180
    // and 360 degrees respectively. These are used for "quick accepts" that allow us to send almost
    // all curves directly to the hardware without having to chop. We raise to the 4th power because
    // the "pow4" variants of Wang's formula are the quickest to evaluate.
    float fMaxParametricSegments_pow4[2];  // Values for strokes that rotate 180 and 360 degrees.
    float fMaxParametricSegments_pow4_withJoin[2];  // For strokes that rotate 180 and 360 degrees.

    // Maximum number of segments we can allocate for a stroke if we are stuffing it in a patch
    // together with a worst-case join.
    float fMaxCombinedSegments_withJoin;

    // Additional info on the current stroke radius/join type.
    bool fSoloRoundJoinAlwaysFitsInPatch;
    JoinType fStrokeJoinType;

    // Variables related to the specific contour that we are currently iterating during
    // prepareBuffers().
    SkPoint fCurrContourStartPoint;
    SkPoint fCurrContourFirstControlPoint;

    bool fHasJoinControlPoint;
    // Stored in super class as well, but value determines what's written into the instance data.
    // This value is available to us, but not exposed to use, we use it for calculations
    SkPoint fJoinControlPoint;
};

void write_hw_patches(HwPatchWriter&& hwPatchWriter,
                      const SkMatrix& shaderMatrix,
                      std::array<float,2> matrixMinMaxScales,
                      StrokeTessellator::PathStrokeList* pathStrokeList) {
    using JoinType = HwPatchWriter::JoinType;

    float matrixMaxScale = matrixMinMaxScales[1];
    if (!(hwPatchWriter.attribs() & PatchAttribs::kStrokeParams)) {
        // Strokes are static. Calculate tolerances once.
        const SkStrokeRec& stroke = pathStrokeList->fStroke;
        float localStrokeWidth = StrokeTolerances::GetLocalStrokeWidth(matrixMinMaxScales.data(),
                                                                       stroke.getWidth());
        float numRadialSegmentsPerRadian = StrokeTolerances::CalcNumRadialSegmentsPerRadian(
                matrixMaxScale, localStrokeWidth);
        hwPatchWriter.updateTolerances(numRadialSegmentsPerRadian, stroke);
    }

    // Fast SIMD queue that buffers up values for "numRadialSegmentsPerRadian". Only used when we
    // have dynamic strokes.
    StrokeToleranceBuffer toleranceBuffer(matrixMaxScale);

    for (auto* pathStroke = pathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        if (hwPatchWriter.attribs() & PatchAttribs::kStrokeParams) {
            // Strokes are dynamic. Update tolerances with every new stroke.
            hwPatchWriter.updateTolerances(toleranceBuffer.fetchRadialSegmentsPerRadian(pathStroke),
                                           stroke);
        }
        if (hwPatchWriter.attribs() & PatchAttribs::kColor) {
            hwPatchWriter.updateColorAttrib(pathStroke->fColor);
        }

        const SkPath& path = pathStroke->fPath;
        bool contourIsEmpty = true;
        for (auto [verb, p, w] : SkPathPriv::Iterate(path)) {
            bool prevJoinFitsInPatch;
            SkPoint scratchPts[4];
            const SkPoint* patchPts;
            SkPoint endControlPoint;
            switch (verb) {
                case SkPathVerb::kMove:
                    // "A subpath ... consisting of a single moveto shall not be stroked."
                    // https://www.w3.org/TR/SVG11/painting.html#StrokeProperties
                    if (!contourIsEmpty) {
                        hwPatchWriter.writeCaps(p[-1], shaderMatrix, stroke);
                    }
                    hwPatchWriter.moveTo(p[0]);
                    contourIsEmpty = true;
                    continue;
                case SkPathVerb::kClose:
                    hwPatchWriter.writeClose(p[0], shaderMatrix, stroke);
                    contourIsEmpty = true;
                    continue;
                case SkPathVerb::kLine:
                    // Set this to false first, before the upcoming continue might disrupt our flow.
                    contourIsEmpty = false;
                    if (p[0] == p[1]) {
                        continue;
                    }
                    prevJoinFitsInPatch = hwPatchWriter.lineFitsInPatch_withJoin();
                    scratchPts[0] = scratchPts[1] = p[0];
                    scratchPts[2] = scratchPts[3] = p[1];
                    patchPts = scratchPts;
                    endControlPoint = p[0];
                    break;
                case SkPathVerb::kQuad: {
                    contourIsEmpty = false;
                    if (p[1] == p[0] || p[1] == p[2]) {
                        // Zero-length paths need special treatment because they are spec'd to
                        // behave differently. If the control point is colocated on an endpoint then
                        // this might end up being the case. Fall back on a lineTo and let it make
                        // the final check.
                        hwPatchWriter.writeLineTo(p[0], p[2]);
                        continue;
                    }
                    if (ConicHasCusp(p)) {
                        // Cusps are rare, but the tessellation shader can't handle them. Chop the
                        // curve into segments that the shader can handle.
                        SkPoint cusp = SkEvalQuadAt(p, SkFindQuadMidTangent(p));
                        hwPatchWriter.writeLineTo(p[0], cusp);
                        hwPatchWriter.writeLineTo(JoinType::kBowtie, cusp, p[2]);
                        continue;
                    }
                    float numParametricSegments_pow4 =
                            wangs_formula::quadratic_pow4(hwPatchWriter.parametricPrecision(), p);
                    if (!hwPatchWriter.stroke180FitsInPatch(numParametricSegments_pow4)) {
                        // The curve requires more tessellation segments than the hardware can
                        // support. This is rare. Recursively chop until each sub-curve fits.
                        hwPatchWriter.writeConicPatchesTo(p, 1);
                        continue;
                    }
                    // The curve fits in a single tessellation patch. This is the most common case.
                    // Write it out directly.
                    prevJoinFitsInPatch = hwPatchWriter.stroke180FitsInPatch_withJoin(
                            numParametricSegments_pow4);
                    quad_to_cubic(p, scratchPts);
                    patchPts = scratchPts;
                    endControlPoint = patchPts[2];
                    break;
                }
                case SkPathVerb::kConic: {
                    contourIsEmpty = false;
                    if (p[1] == p[0] || p[1] == p[2]) {
                        // Zero-length paths need special treatment because they are spec'd to
                        // behave differently. If the control point is colocated on an endpoint then
                        // this might end up being the case. Fall back on a lineTo and let it make
                        // the final check.
                        hwPatchWriter.writeLineTo(p[0], p[2]);
                        continue;
                    }
                    if (ConicHasCusp(p)) {
                        // Cusps are rare, but the tessellation shader can't handle them. Chop the
                        // curve into segments that the shader can handle.
                        SkConic conic(p, *w);
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        hwPatchWriter.writeLineTo(p[0], cusp);
                        hwPatchWriter.writeLineTo(JoinType::kBowtie, cusp, p[2]);
                        continue;
                    }
                    // For now, the tessellation shader still uses Wang's quadratic formula when it
                    // draws conics.
                    // TODO: Update here when the shader starts using the real conic formula.
                    float n = wangs_formula::conic_pow2(hwPatchWriter.parametricPrecision(), p, *w);
                    float numParametricSegments_pow4 = n*n;
                    if (!hwPatchWriter.stroke180FitsInPatch(numParametricSegments_pow4)) {
                        // The curve requires more tessellation segments than the hardware can
                        // support. This is rare. Recursively chop until each sub-curve fits.
                        hwPatchWriter.writeConicPatchesTo(p, *w);
                        continue;
                    }
                    // The curve fits in a single tessellation patch. This is the most common
                    // case. Write it out directly.
                    prevJoinFitsInPatch = hwPatchWriter.stroke180FitsInPatch_withJoin(
                            numParametricSegments_pow4);
                    memcpy(scratchPts, p, sizeof(SkPoint) * 3);
                    scratchPts[3] = {*w, std::numeric_limits<float>::infinity()};
                    patchPts = scratchPts;
                    endControlPoint = p[1];
                    break;
                }
                case SkPathVerb::kCubic: {
                    contourIsEmpty = false;
                    if (p[1] == p[2] && (p[1] == p[0] || p[1] == p[3])) {
                        // The stroke tessellation shader assigns special meaning to p0==p1==p2 and
                        // p1==p2==p3. If this is the case then we need to rewrite the cubic.
                        hwPatchWriter.writeLineTo(p[0], p[3]);
                        continue;
                    }
                    float numParametricSegments_pow4 =
                            wangs_formula::cubic_pow4(hwPatchWriter.parametricPrecision(), p);
                    if (!hwPatchWriter.stroke360FitsInPatch(numParametricSegments_pow4) ||
                        cubic_has_cusp(p)) {
                        // Either the curve requires more tessellation segments than the hardware
                        // can support, or it has cusp(s). Either case is rare. Chop it into
                        // sections that rotate 180 degrees or less (which will naturally be the
                        // cusp points if there are any), and then recursively chop each section
                        // until it fits.
                        hwPatchWriter.writeCubicConvex180PatchesTo(p);
                        continue;
                    }
                    // The curve fits in a single tessellation patch. This is the most common case.
                    // Write it out directly.
                    prevJoinFitsInPatch = hwPatchWriter.stroke360FitsInPatch_withJoin(
                            numParametricSegments_pow4);
                    patchPts = p;
                    endControlPoint = (p[2] != p[3]) ? p[2] : p[1];
                    break;
                }
            }
            hwPatchWriter.writePatchTo(prevJoinFitsInPatch, patchPts, endControlPoint);
        }
        if (!contourIsEmpty) {
            const SkPoint* p = SkPathPriv::PointData(path);
            hwPatchWriter.writeCaps(p[path.countPoints() - 1], shaderMatrix, stroke);
        }
    }
}

}  // namespace


SKGPU_DECLARE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

int StrokeFixedCountTessellator::prepare(GrMeshDrawTarget* target,
                                         const SkMatrix& shaderMatrix,
                                         std::array<float,2> matrixMinMaxScales,
                                         PathStrokeList* pathStrokeList,
                                         int totalCombinedStrokeVerbCnt) {
    int preallocCount = FixedCountStrokes::PreallocCount(totalCombinedStrokeVerbCnt);
    FixedCountStrokeWriter patchWriter{fAttribs, kMaxParametricSegments,
                                       target, &fVertexChunkArray, preallocCount};

    fFixedEdgeCount = write_fixed_count_patches(std::move(patchWriter),
                                                shaderMatrix,
                                                matrixMinMaxScales,
                                                pathStrokeList);

    // Don't draw more vertices than can be indexed by a signed short. We just have to draw the line
    // somewhere and this seems reasonable enough. (There are two vertices per edge, so 2^14 edges
    // make 2^15 vertices.)
    fFixedEdgeCount = std::min(fFixedEdgeCount, (1 << 14) - 1);

    if (!target->caps().shaderCaps()->vertexIDSupport()) {
        // Our shader won't be able to use sk_VertexID. Bind a fallback vertex buffer with the IDs
        // in it instead.
        constexpr static int kMaxEdgesInFallbackBuffer = 1024;
        fFixedEdgeCount = std::min(fFixedEdgeCount, kMaxEdgesInFallbackBuffer);

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

        fVertexBufferIfNoIDSupport = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex,
                FixedCountStrokes::VertexBufferSize(kMaxEdgesInFallbackBuffer),
                gVertexIDFallbackBufferKey,
                FixedCountStrokes::WriteVertexBuffer);
    }

    return fFixedEdgeCount;
}

void StrokeFixedCountTessellator::draw(GrOpFlushState* flushState) const {
    if (fVertexChunkArray.empty() || fFixedEdgeCount <= 0) {
        return;
    }
    if (!flushState->caps().shaderCaps()->vertexIDSupport() &&
        !fVertexBufferIfNoIDSupport) {
        return;
    }
    for (const auto& instanceChunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, instanceChunk.fBuffer, fVertexBufferIfNoIDSupport);
        flushState->drawInstanced(instanceChunk.fCount,
                                  instanceChunk.fBase,
                                  fFixedEdgeCount * 2,
                                  0);
    }
}

int StrokeHardwareTessellator::prepare(GrMeshDrawTarget* target,
                                       const SkMatrix& shaderMatrix,
                                       std::array<float,2> matrixMinMaxScales,
                                       PathStrokeList* pathStrokeList,
                                       int totalCombinedStrokeVerbCnt) {
    // Over-allocate enough patches for 1 in 4 strokes to chop and for 8 extra caps.
    int strokePreallocCount = (totalCombinedStrokeVerbCnt * 5) / 4;
    int capPreallocCount = 8;
    int preallocCount = strokePreallocCount + capPreallocCount;

    HwPatchWriter writer{target, &fVertexChunkArray, fAttribs, fMaxTessellationSegments,
                         preallocCount, matrixMinMaxScales[1]};
    write_hw_patches(std::move(writer), shaderMatrix, matrixMinMaxScales, pathStrokeList);
    return 0; // 0 is returned for non-fixed-count stroke tessellation
}

void StrokeHardwareTessellator::draw(GrOpFlushState* flushState) const {
    for (const auto& vertexChunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, nullptr, vertexChunk.fBuffer);
        flushState->draw(vertexChunk.fCount, vertexChunk.fBase);
    }
}

}  // namespace skgpu::v1
