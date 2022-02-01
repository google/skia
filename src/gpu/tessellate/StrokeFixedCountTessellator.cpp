/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/StrokeFixedCountTessellator.h"

#include "src/core/SkGeometry.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/StrokeIterator.h"
#include "src/gpu/tessellate/WangsFormula.h"

#if SK_GPU_V1
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"
#endif

namespace skgpu {

namespace {

constexpr static float kMaxParametricSegments_pow4 =
        StrokeFixedCountTessellator::kMaxParametricSegments_pow4;

// Writes out strokes to the given instance chunk array, chopping if necessary so that all instances
// require 32 parametric segments or less. (We don't consider radial segments here. The tessellator
// will just add enough additional segments to handle a worst-case 180 degree stroke.)
class InstanceWriter {
public:
    InstanceWriter(PatchWriter& patchWriter, float matrixMaxScale)
            : fPatchWriter(patchWriter)
            , fParametricPrecision(StrokeTolerances::CalcParametricPrecision(matrixMaxScale)) {
    }

    float parametricPrecision() const { return fParametricPrecision; }

    // maxParametricSegments^4, or the number of parametric segments, raised to the 4th power,
    // that are required by the single instance we've written that requires the most segments.
    float maxParametricSegments_pow4() const { return fMaxParametricSegments_pow4; }

    SK_ALWAYS_INLINE void lineTo(SkPoint start, SkPoint end) {
        SkPoint cubic[] = {start, start, end, end};
        SkPoint endControlPoint = start;
        this->writeStroke(cubic, endControlPoint, kCubicCurveType);
    }

    SK_ALWAYS_INLINE void quadraticTo(const SkPoint p[3]) {
        float numParametricSegments_pow4 = wangs_formula::quadratic_pow4(fParametricPrecision, p);
        if (numParametricSegments_pow4 > kMaxParametricSegments_pow4) {
            this->chopQuadraticTo(p);
            return;
        }
        SkPoint cubic[4];
        VertexWriter(cubic, sizeof(cubic)) << QuadToCubic(p);
        SkPoint endControlPoint = cubic[2];
        this->writeStroke(cubic, endControlPoint, kCubicCurveType);
        fMaxParametricSegments_pow4 = std::max(numParametricSegments_pow4,
                                               fMaxParametricSegments_pow4);
    }

    SK_ALWAYS_INLINE void conicTo(const SkPoint p[3], float w) {
        float n = wangs_formula::conic_pow2(fParametricPrecision, p, w);
        float numParametricSegments_pow4 = n*n;
        if (numParametricSegments_pow4 > kMaxParametricSegments_pow4) {
            this->chopConicTo({p, w});
            return;
        }
        SkPoint conic[4] = {p[0], p[1], p[2], {w, std::numeric_limits<float>::infinity()}};
        SkPoint endControlPoint = conic[1];
        this->writeStroke(conic, endControlPoint, kConicCurveType);
        fMaxParametricSegments_pow4 = std::max(numParametricSegments_pow4,
                                               fMaxParametricSegments_pow4);
    }

    SK_ALWAYS_INLINE void cubicConvex180To(const SkPoint p[4]) {
        float numParametricSegments_pow4 = wangs_formula::cubic_pow4(fParametricPrecision, p);
        if (numParametricSegments_pow4 > kMaxParametricSegments_pow4) {
            this->chopCubicConvex180To(p);
            return;
        }
        SkPoint endControlPoint = (p[3] != p[2]) ? p[2] : (p[2] != p[1]) ? p[1] : p[0];
        this->writeStroke(p, endControlPoint, kCubicCurveType);
        fMaxParametricSegments_pow4 = std::max(numParametricSegments_pow4,
                                               fMaxParametricSegments_pow4);
    }

    // Called when we encounter the verb "kMoveWithinContour". Moves invalidate the previous control
    // point. The stroke iterator tells us the new value to use for the previous control point.
    void setLastControlPoint(SkPoint newLastControlPoint) {
        fLastControlPoint = newLastControlPoint;
        fHasLastControlPoint = true;
    }

    // Draws a circle whose diameter is equal to the stroke width. We emit circles at cusp points
    // round caps, and empty strokes that are specified to be drawn as circles.
    void writeCircle(SkPoint location) {
        // The shader interprets an empty stroke + empty join as a special case that denotes a
        // circle, or 180-degree point stroke.
        PatchWriter::CubicPatch(fPatchWriter) << VertexWriter::Repeat<5>(location);
    }

    void finishContour() {
        if (fHasDeferredFirstStroke) {
            // We deferred the first stroke because we didn't know the previous control point to use
            // for its join. We write it out now.
            SkASSERT(fHasLastControlPoint);
            this->writeStroke(fDeferredFirstStroke, SkPoint(),
                              fDeferredCurveTypeIfUnsupportedInfinity);
            fHasDeferredFirstStroke = false;
        }
        fHasLastControlPoint = false;
    }

private:
    void chopQuadraticTo(const SkPoint p[3]) {
        SkPoint chops[5];
        SkChopQuadAtHalf(p, chops);
        this->quadraticTo(chops);
        this->quadraticTo(chops + 2);
    }

    void chopConicTo(const SkConic& conic) {
        SkConic chops[2];
        if (!conic.chopAt(.5f, chops)) {
            return;
        }
        this->conicTo(chops[0].fPts, chops[0].fW);
        this->conicTo(chops[1].fPts, chops[1].fW);
    }

    void chopCubicConvex180To(const SkPoint p[4]) {
        SkPoint chops[7];
        SkChopCubicAtHalf(p, chops);
        this->cubicConvex180To(chops);
        this->cubicConvex180To(chops + 3);
    }

    SK_ALWAYS_INLINE void writeStroke(const SkPoint p[4], SkPoint endControlPoint,
                                      float curveTypeIfUnsupportedInfinity) {
        if (fHasLastControlPoint) {
            PatchWriter::Patch(fPatchWriter, curveTypeIfUnsupportedInfinity)
                    << VertexWriter::Array(p, 4) << fLastControlPoint;
        } else {
            // We don't know the previous control point yet to use for the join. Defer writing out
            // this stroke until the end.
            memcpy(fDeferredFirstStroke, p, sizeof(fDeferredFirstStroke));
            fDeferredCurveTypeIfUnsupportedInfinity = curveTypeIfUnsupportedInfinity;
            fHasDeferredFirstStroke = true;
            fHasLastControlPoint = true;
        }
        fLastControlPoint = endControlPoint;
    }

    void discardStroke(const SkPoint p[], int numPts) {
        // Set fLastControlPoint to the next stroke's p0 (which will be equal to the final point of
        // this stroke). This has the effect of disabling the next stroke's join.
        fLastControlPoint = p[numPts - 1];
        fHasLastControlPoint = true;
    }

    PatchWriter& fPatchWriter;
    const float fParametricPrecision;
    float fMaxParametricSegments_pow4 = 1;

    // We can't write out the first stroke until we know the previous control point for its join.
    SkPoint fDeferredFirstStroke[4];
    float fDeferredCurveTypeIfUnsupportedInfinity;
    SkPoint fLastControlPoint;  // Used to configure the joins in the instance data.
    bool fHasDeferredFirstStroke = false;
    bool fHasLastControlPoint = false;
};

// Returns the worst-case number of edges we will need in order to draw a join of the given type.
int worst_case_edges_in_join(SkPaint::Join joinType, float numRadialSegmentsPerRadian) {
    int numEdges = StrokeFixedCountTessellator::NumFixedEdgesInJoin(joinType);
    if (joinType == SkPaint::kRound_Join) {
        // For round joins we need to count the radial edges on our own. Account for a worst-case
        // join of 180 degrees (SK_ScalarPI radians).
        numEdges += std::max(SkScalarCeilToInt(numRadialSegmentsPerRadian * SK_ScalarPI) - 1, 0);
    }
    return numEdges;
}

}  // namespace


int StrokeFixedCountTessellator::patchPreallocCount(int totalCombinedStrokeVerbCnt) const {
    // Over-allocate enough patches for each stroke to chop once, and for 8 extra caps. Since we
    // have to chop at inflections, points of 180 degree rotation, and anywhere a stroke requires
    // too many parametric segments, many strokes will end up getting choppped.
    int strokePreallocCount = totalCombinedStrokeVerbCnt * 2;
    int capPreallocCount = 8;
    return strokePreallocCount + capPreallocCount;
}

int StrokeFixedCountTessellator::writePatches(PatchWriter& patchWriter,
                                              const SkMatrix& shaderMatrix,
                                              std::array<float,2> matrixMinMaxScales,
                                              PathStrokeList* pathStrokeList) {
    int maxEdgesInJoin = 0;
    float maxRadialSegmentsPerRadian = 0;

    InstanceWriter instanceWriter(patchWriter, matrixMinMaxScales[1]);

    if (!(fAttribs & PatchAttribs::kStrokeParams)) {
        // Strokes are static. Calculate tolerances once.
        const SkStrokeRec& stroke = pathStrokeList->fStroke;
        float localStrokeWidth = StrokeTolerances::GetLocalStrokeWidth(matrixMinMaxScales.data(),
                                                                       stroke.getWidth());
        float numRadialSegmentsPerRadian = StrokeTolerances::CalcNumRadialSegmentsPerRadian(
                instanceWriter.parametricPrecision(), localStrokeWidth);
        maxEdgesInJoin = worst_case_edges_in_join(stroke.getJoin(), numRadialSegmentsPerRadian);
        maxRadialSegmentsPerRadian = numRadialSegmentsPerRadian;
    }

    // Fast SIMD queue that buffers up values for "numRadialSegmentsPerRadian". Only used when we
    // have dynamic stroke.
    StrokeToleranceBuffer toleranceBuffer(instanceWriter.parametricPrecision());

    for (PathStrokeList* pathStroke = pathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        if (fAttribs & PatchAttribs::kStrokeParams) {
            // Strokes are dynamic. Calculate tolerances every time.
            float numRadialSegmentsPerRadian =
                    toleranceBuffer.fetchRadialSegmentsPerRadian(pathStroke);
            maxEdgesInJoin = std::max(
                    worst_case_edges_in_join(stroke.getJoin(), numRadialSegmentsPerRadian),
                    maxEdgesInJoin);
            maxRadialSegmentsPerRadian = std::max(numRadialSegmentsPerRadian,
                                                  maxRadialSegmentsPerRadian);
            patchWriter.updateStrokeParamsAttrib(stroke);
        }
        if (fAttribs & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(pathStroke->fColor);
        }
        StrokeIterator strokeIter(pathStroke->fPath, &pathStroke->fStroke, &shaderMatrix);
        while (strokeIter.next()) {
            const SkPoint* p = strokeIter.pts();
            switch (strokeIter.verb()) {
                using Verb = StrokeIterator::Verb;
                int numChops;
                case Verb::kContourFinished:
                    instanceWriter.finishContour();
                    break;
                case Verb::kCircle:
                    // Round cap or else an empty stroke that is specified to be drawn as a circle.
                    instanceWriter.writeCircle(p[0]);
                    [[fallthrough]];
                case Verb::kMoveWithinContour:
                    instanceWriter.setLastControlPoint(p[0]);
                    break;
                case Verb::kLine:
                    instanceWriter.lineTo(p[0], p[1]);
                    break;
                case Verb::kQuad:
                    if (ConicHasCusp(p)) {
                        // The cusp is always at the midtandent.
                        SkPoint cusp = SkEvalQuadAt(p, SkFindQuadMidTangent(p));
                        instanceWriter.writeCircle(cusp);
                        // A quad can only have a cusp if it's flat with a 180-degree turnaround.
                        instanceWriter.lineTo(p[0], cusp);
                        instanceWriter.lineTo(cusp, p[2]);
                    } else {
                        instanceWriter.quadraticTo(p);
                    }
                    break;
                case Verb::kConic:
                    if (ConicHasCusp(p)) {
                        // The cusp is always at the midtandent.
                        SkConic conic(p, strokeIter.w());
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        instanceWriter.writeCircle(cusp);
                        // A conic can only have a cusp if it's flat with a 180-degree turnaround.
                        instanceWriter.lineTo(p[0], cusp);
                        instanceWriter.lineTo(cusp, p[2]);
                    } else {
                        instanceWriter.conicTo(p, strokeIter.w());
                    }
                    break;
                case Verb::kCubic:
                    SkPoint chops[10];
                    float T[2];
                    bool areCusps;
                    numChops = FindCubicConvex180Chops(p, T, &areCusps);
                    if (numChops == 0) {
                        instanceWriter.cubicConvex180To(p);
                    } else if (numChops == 1) {
                        SkChopCubicAt(p, chops, T[0]);
                        if (areCusps) {
                            instanceWriter.writeCircle(chops[3]);
                            // In a perfect world, these 3 points would be be equal after chopping
                            // on a cusp.
                            chops[2] = chops[4] = chops[3];
                        }
                        instanceWriter.cubicConvex180To(chops);
                        instanceWriter.cubicConvex180To(chops + 3);
                    } else {
                        SkASSERT(numChops == 2);
                        SkChopCubicAt(p, chops, T[0], T[1]);
                        if (areCusps) {
                            instanceWriter.writeCircle(chops[3]);
                            instanceWriter.writeCircle(chops[6]);
                            // Two cusps are only possible if it's a flat line with two 180-degree
                            // turnarounds.
                            instanceWriter.lineTo(chops[0], chops[3]);
                            instanceWriter.lineTo(chops[3], chops[6]);
                            instanceWriter.lineTo(chops[6], chops[9]);
                        } else {
                            instanceWriter.cubicConvex180To(chops);
                            instanceWriter.cubicConvex180To(chops + 3);
                            instanceWriter.cubicConvex180To(chops + 6);
                        }
                    }
                    break;
            }
        }
    }

    // The maximum rotation we can have in a stroke is 180 degrees (SK_ScalarPI radians).
    int maxRadialSegmentsInStroke =
            std::max(SkScalarCeilToInt(maxRadialSegmentsPerRadian * SK_ScalarPI), 1);

    int maxParametricSegmentsInStroke = SkScalarCeilToInt(sqrtf(sqrtf(
            instanceWriter.maxParametricSegments_pow4())));
    SkASSERT(maxParametricSegmentsInStroke >= 1);  // maxParametricSegments_pow4 is always >= 1.

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

void StrokeFixedCountTessellator::InitializeVertexIDFallbackBuffer(VertexWriter vertexWriter,
                                                                   size_t bufferSize) {
    SkASSERT(bufferSize % (sizeof(float) * 2) == 0);
    int edgeCount = bufferSize / (sizeof(float) * 2);
    for (int i = 0; i < edgeCount; ++i) {
        vertexWriter << (float)i << (float)-i;
    }
}

#if SK_GPU_V1

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

int StrokeFixedCountTessellator::prepare(GrMeshDrawTarget* target,
                                         const SkMatrix& shaderMatrix,
                                         std::array<float,2> matrixMinMaxScales,
                                         PathStrokeList* pathStrokeList,
                                         int totalCombinedStrokeVerbCnt) {
    PatchWriter patchWriter(target, this, this->patchPreallocCount(totalCombinedStrokeVerbCnt));

    fFixedEdgeCount = this->writePatches(patchWriter,
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
        constexpr static int kMaxVerticesInFallbackBuffer = 2048;
        fFixedEdgeCount = std::min(fFixedEdgeCount, kMaxVerticesInFallbackBuffer/2);

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

        fVertexBufferIfNoIDSupport = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex,
                kMaxVerticesInFallbackBuffer * sizeof(float),
                gVertexIDFallbackBufferKey,
                InitializeVertexIDFallbackBuffer);
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

#endif

}  // namespace skgpu
