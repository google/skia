/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/tessellate/StrokeTessellator.h"

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

using namespace skgpu::tess;

using FixedCountStrokeWriter = PatchWriter<GrVertexChunkBuilder,
                                           Required<PatchAttribs::kJoinControlPoint>,
                                           Optional<PatchAttribs::kStrokeParams>,
                                           Optional<PatchAttribs::kColor>,
                                           Optional<PatchAttribs::kWideColorIfEnabled>,
                                           Optional<PatchAttribs::kExplicitCurveType>,
                                           ReplicateLineEndPoints,
                                           TrackJoinControlPoints>;

int write_fixed_count_patches(FixedCountStrokeWriter&& patchWriter,
                              const SkMatrix& shaderMatrix,
                              StrokeTessellator::PathStrokeList* pathStrokeList) {
    int maxEdgesInJoin = 0;
    float maxRadialSegmentsPerRadian = 0;
    // getMaxScale() returns -1 if it can't compute a scale factor (e.g. perspective), taking the
    // absolute value automatically converts that to an identity scale factor for our purposes.
    float maxScale = std::abs(shaderMatrix.getMaxScale());

    if (!(patchWriter.attribs() & PatchAttribs::kStrokeParams)) {
        // Strokes are static. Calculate tolerances once.
        const SkStrokeRec& stroke = pathStrokeList->fStroke;
        float approxDevStrokeWidth = stroke.isHairlineStyle() ? 1.f : maxScale * stroke.getWidth();
        float numRadialSegmentsPerRadian =
                CalcNumRadialSegmentsPerRadian(0.5f * approxDevStrokeWidth);
        maxEdgesInJoin = WorstCaseEdgesInJoin(stroke.getJoin(), numRadialSegmentsPerRadian);
        maxRadialSegmentsPerRadian = numRadialSegmentsPerRadian;
    }

    // The vector xform approximates how the control points are transformed by the shader to
    // more accurately compute how many *parametric* segments are needed.
    wangs_formula::VectorXform shaderXform{shaderMatrix};
    for (auto* pathStroke = pathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        if (patchWriter.attribs() & PatchAttribs::kStrokeParams) {
            // Strokes are dynamic. Calculate tolerances every time.
            float approxDevStrokeWidth =
                    stroke.isHairlineStyle() ? 1.f : maxScale * stroke.getWidth();
            float numRadialSegmentsPerRadian =
                    CalcNumRadialSegmentsPerRadian(0.5f * approxDevStrokeWidth);
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

}  // namespace


SKGPU_DECLARE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

void StrokeTessellator::prepare(GrMeshDrawTarget* target,
                                const SkMatrix& shaderMatrix,
                                PathStrokeList* pathStrokeList,
                                int totalCombinedStrokeVerbCnt) {
    int preallocCount = FixedCountStrokes::PreallocCount(totalCombinedStrokeVerbCnt);
    FixedCountStrokeWriter patchWriter{fAttribs, target, &fVertexChunkArray, preallocCount};

    fFixedEdgeCount = write_fixed_count_patches(std::move(patchWriter),
                                                shaderMatrix,
                                                pathStrokeList);

    fFixedEdgeCount = std::min(fFixedEdgeCount, FixedCountStrokes::kMaxEdges);

    if (!target->caps().shaderCaps()->vertexIDSupport()) {
        // Our shader won't be able to use sk_VertexID. Bind a fallback vertex buffer with the IDs
        // in it instead.
        fFixedEdgeCount = std::min(fFixedEdgeCount, FixedCountStrokes::kMaxEdgesNoVertexIDs);

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gVertexIDFallbackBufferKey);

        fVertexBufferIfNoIDSupport = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex,
                FixedCountStrokes::VertexBufferSize(),
                gVertexIDFallbackBufferKey,
                FixedCountStrokes::WriteVertexBuffer);
    }
}

void StrokeTessellator::draw(GrOpFlushState* flushState) const {
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

}  // namespace skgpu::v1
