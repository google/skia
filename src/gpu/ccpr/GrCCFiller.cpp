/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCFiller.h"

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include <stdlib.h>

using TriPointInstance = GrCCCoverageProcessor::TriPointInstance;
using QuadPointInstance = GrCCCoverageProcessor::QuadPointInstance;

GrCCFiller::GrCCFiller(Algorithm algorithm, int numPaths, int numSkPoints, int numSkVerbs,
                       int numConicWeights)
        : fAlgorithm(algorithm)
        , fGeometry(numSkPoints, numSkVerbs, numConicWeights)
        , fPathInfos(numPaths)
        , fScissorSubBatches(numPaths)
        , fTotalPrimitiveCounts{PrimitiveTallies(), PrimitiveTallies()} {
    // Batches decide what to draw by looking where the previous one ended. Define initial batches
    // that "end" at the beginning of the data. These will not be drawn, but will only be be read by
    // the first actual batch.
    fScissorSubBatches.push_back() = {PrimitiveTallies(), SkIRect::MakeEmpty()};
    fBatches.push_back() = {PrimitiveTallies(), fScissorSubBatches.count(), PrimitiveTallies()};
}

void GrCCFiller::parseDeviceSpaceFill(const SkPath& path, const SkPoint* deviceSpacePts,
                                      GrScissorTest scissorTest, const SkIRect& clippedDevIBounds,
                                      const SkIVector& devToAtlasOffset) {
    SkASSERT(!fInstanceBuffer);  // Can't call after prepareToDraw().
    SkASSERT(!path.isEmpty());

    int currPathPointsIdx = fGeometry.points().count();
    int currPathVerbsIdx = fGeometry.verbs().count();
    PrimitiveTallies currPathPrimitiveCounts = PrimitiveTallies();

    fGeometry.beginPath();

    const float* conicWeights = SkPathPriv::ConicWeightData(path);
    int ptsIdx = 0;
    int conicWeightsIdx = 0;
    bool insideContour = false;

    for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (insideContour) {
                    currPathPrimitiveCounts += fGeometry.endContour();
                }
                fGeometry.beginContour(deviceSpacePts[ptsIdx]);
                ++ptsIdx;
                insideContour = true;
                continue;
            case SkPath::kClose_Verb:
                if (insideContour) {
                    currPathPrimitiveCounts += fGeometry.endContour();
                }
                insideContour = false;
                continue;
            case SkPath::kLine_Verb:
                fGeometry.lineTo(&deviceSpacePts[ptsIdx - 1]);
                ++ptsIdx;
                continue;
            case SkPath::kQuad_Verb:
                fGeometry.quadraticTo(&deviceSpacePts[ptsIdx - 1]);
                ptsIdx += 2;
                continue;
            case SkPath::kCubic_Verb:
                fGeometry.cubicTo(&deviceSpacePts[ptsIdx - 1]);
                ptsIdx += 3;
                continue;
            case SkPath::kConic_Verb:
                fGeometry.conicTo(&deviceSpacePts[ptsIdx - 1], conicWeights[conicWeightsIdx]);
                ptsIdx += 2;
                ++conicWeightsIdx;
                continue;
            default:
                SK_ABORT("Unexpected path verb.");
        }
    }
    SkASSERT(ptsIdx == path.countPoints());
    SkASSERT(conicWeightsIdx == SkPathPriv::ConicWeightCnt(path));

    if (insideContour) {
        currPathPrimitiveCounts += fGeometry.endContour();
    }

    fPathInfos.emplace_back(scissorTest, devToAtlasOffset);

    // Tessellate fans from very large and/or simple paths, in order to reduce overdraw.
    int numVerbs = fGeometry.verbs().count() - currPathVerbsIdx - 1;
    int64_t tessellationWork = (int64_t)numVerbs * (32 - SkCLZ(numVerbs)); // N log N.
    int64_t fanningWork = (int64_t)clippedDevIBounds.height() * clippedDevIBounds.width();
    if (tessellationWork * (50*50) + (100*100) < fanningWork) { // Don't tessellate under 100x100.
        fPathInfos.back().tessellateFan(
                fAlgorithm, path, fGeometry, currPathVerbsIdx, currPathPointsIdx, clippedDevIBounds,
                &currPathPrimitiveCounts);
    }

    fTotalPrimitiveCounts[(int)scissorTest] += currPathPrimitiveCounts;

    if (GrScissorTest::kEnabled == scissorTest) {
        fScissorSubBatches.push_back() = {fTotalPrimitiveCounts[(int)GrScissorTest::kEnabled],
                                          clippedDevIBounds.makeOffset(devToAtlasOffset.fX,
                                                                       devToAtlasOffset.fY)};
    }
}

void GrCCFiller::PathInfo::tessellateFan(
        Algorithm algorithm, const SkPath& originalPath, const GrCCFillGeometry& geometry,
        int verbsIdx, int ptsIdx, const SkIRect& clippedDevIBounds,
        PrimitiveTallies* newTriangleCounts) {
    using Verb = GrCCFillGeometry::Verb;
    SkASSERT(-1 == fFanTessellationCount);
    SkASSERT(!fFanTessellation);

    const SkTArray<Verb, true>& verbs = geometry.verbs();
    const SkTArray<SkPoint, true>& pts = geometry.points();

    newTriangleCounts->fTriangles =
            newTriangleCounts->fWeightedTriangles = 0;

    // Build an SkPath of the Redbook fan.
    SkPath fan;
    if (Algorithm::kCoverageCount == algorithm) {
        // We use "winding" fill type right now because we are producing a coverage count, and must
        // fill in every region that has non-zero wind. The path processor will convert coverage
        // count to the appropriate fill type later.
        fan.setFillType(SkPath::kWinding_FillType);
    } else {
        // When counting winding numbers in the stencil buffer, it works to use even/odd for the fan
        // tessellation (where applicable). But we need to strip out inverse fill info because
        // inverse-ness gets accounted for later on.
        fan.setFillType(SkPath::ConvertToNonInverseFillType(originalPath.getFillType()));
    }
    SkASSERT(Verb::kBeginPath == verbs[verbsIdx]);
    for (int i = verbsIdx + 1; i < verbs.count(); ++i) {
        switch (verbs[i]) {
            case Verb::kBeginPath:
                SK_ABORT("Invalid GrCCFillGeometry");
                continue;

            case Verb::kBeginContour:
                fan.moveTo(pts[ptsIdx++]);
                continue;

            case Verb::kLineTo:
                fan.lineTo(pts[ptsIdx++]);
                continue;

            case Verb::kMonotonicQuadraticTo:
            case Verb::kMonotonicConicTo:
                fan.lineTo(pts[ptsIdx + 1]);
                ptsIdx += 2;
                continue;

            case Verb::kMonotonicCubicTo:
                fan.lineTo(pts[ptsIdx + 2]);
                ptsIdx += 3;
                continue;

            case Verb::kEndClosedContour:
            case Verb::kEndOpenContour:
                fan.close();
                continue;
        }
    }

    GrTessellator::WindingVertex* vertices = nullptr;
    SkASSERT(!fan.isInverseFillType());
    fFanTessellationCount = GrTessellator::PathToVertices(
            fan, std::numeric_limits<float>::infinity(), SkRect::Make(clippedDevIBounds),
            &vertices);
    if (fFanTessellationCount <= 0) {
        SkASSERT(0 == fFanTessellationCount);
        SkASSERT(nullptr == vertices);
        return;
    }

    SkASSERT(0 == fFanTessellationCount % 3);
    for (int i = 0; i < fFanTessellationCount; i += 3) {
        int tessWinding = vertices[i].fWinding;
        SkASSERT(tessWinding == vertices[i + 1].fWinding);
        SkASSERT(tessWinding == vertices[i + 2].fWinding);

        // Ensure this triangle's points actually wind in the same direction as tessWinding.
        // CCPR shaders use the sign of wind to determine which direction to bloat, so even for
        // "wound" triangles the winding sign and point ordering need to agree.
        float ax = vertices[i].fPos.fX - vertices[i + 1].fPos.fX;
        float ay = vertices[i].fPos.fY - vertices[i + 1].fPos.fY;
        float bx = vertices[i].fPos.fX - vertices[i + 2].fPos.fX;
        float by = vertices[i].fPos.fY - vertices[i + 2].fPos.fY;
        float wind = ax*by - ay*bx;
        if ((wind > 0) != (-tessWinding > 0)) { // Tessellator has opposite winding sense.
            std::swap(vertices[i + 1].fPos, vertices[i + 2].fPos);
        }

        int weight = abs(tessWinding);
        SkASSERT(SkPath::kEvenOdd_FillType != fan.getFillType() || weight == 1);
        if (weight > 1 && Algorithm::kCoverageCount == algorithm) {
            ++newTriangleCounts->fWeightedTriangles;
        } else {
            newTriangleCounts->fTriangles += weight;
        }
    }

    fFanTessellation.reset(vertices);
}

GrCCFiller::BatchID GrCCFiller::closeCurrentBatch() {
    SkASSERT(!fInstanceBuffer);
    SkASSERT(!fBatches.empty());

    const auto& lastBatch = fBatches.back();
    int maxMeshes = 1 + fScissorSubBatches.count() - lastBatch.fEndScissorSubBatchIdx;
    fMaxMeshesPerDraw = SkTMax(fMaxMeshesPerDraw, maxMeshes);

    const auto& lastScissorSubBatch = fScissorSubBatches[lastBatch.fEndScissorSubBatchIdx - 1];
    PrimitiveTallies batchTotalCounts = fTotalPrimitiveCounts[(int)GrScissorTest::kDisabled] -
                                        lastBatch.fEndNonScissorIndices;
    batchTotalCounts += fTotalPrimitiveCounts[(int)GrScissorTest::kEnabled] -
                        lastScissorSubBatch.fEndPrimitiveIndices;

    // This will invalidate lastBatch.
    fBatches.push_back() = {
        fTotalPrimitiveCounts[(int)GrScissorTest::kDisabled],
        fScissorSubBatches.count(),
        batchTotalCounts
    };
    return fBatches.count() - 1;
}

// Emits a contour's triangle fan.
//
// Classic Redbook fanning would be the triangles: [0  1  2], [0  2  3], ..., [0  n-2  n-1].
//
// This function emits the triangle: [0  n/3  n*2/3], and then recurses on all three sides. The
// advantage to this approach is that for a convex-ish contour, it generates larger triangles.
// Classic fanning tends to generate long, skinny triangles, which are expensive to draw since they
// have a longer perimeter to rasterize and antialias.
//
// The indices array indexes the fan's points (think: glDrawElements), and must have at least log3
// elements past the end for this method to use as scratch space.
//
// Returns the next triangle instance after the final one emitted.
static TriPointInstance* emit_recursive_fan(
        const SkTArray<SkPoint, true>& pts, SkTArray<int32_t, true>& indices, int firstIndex,
        int indexCount, const Sk2f& devToAtlasOffset, TriPointInstance::Ordering ordering,
        TriPointInstance out[]) {
    if (indexCount < 3) {
        return out;
    }

    int32_t oneThirdCount = indexCount / 3;
    int32_t twoThirdsCount = (2 * indexCount) / 3;
    out++->set(pts[indices[firstIndex]], pts[indices[firstIndex + oneThirdCount]],
               pts[indices[firstIndex + twoThirdsCount]], devToAtlasOffset, ordering);

    out = emit_recursive_fan(
            pts, indices, firstIndex, oneThirdCount + 1, devToAtlasOffset, ordering, out);
    out = emit_recursive_fan(
            pts, indices, firstIndex + oneThirdCount, twoThirdsCount - oneThirdCount + 1,
            devToAtlasOffset, ordering, out);

    int endIndex = firstIndex + indexCount;
    int32_t oldValue = indices[endIndex];
    indices[endIndex] = indices[firstIndex];
    out = emit_recursive_fan(
            pts, indices, firstIndex + twoThirdsCount, indexCount - twoThirdsCount + 1,
            devToAtlasOffset, ordering, out);
    indices[endIndex] = oldValue;

    return out;
}

void GrCCFiller::emitTessellatedFan(
        const GrTessellator::WindingVertex* vertices, int numVertices, const Sk2f& devToAtlasOffset,
        TriPointInstance::Ordering ordering, TriPointInstance* triPointInstanceData,
        QuadPointInstance* quadPointInstanceData, GrCCFillGeometry::PrimitiveTallies* indices) {
    for (int i = 0; i < numVertices; i += 3) {
        int weight = abs(vertices[i].fWinding);
        SkASSERT(weight >= 1);
        if (weight > 1 && Algorithm::kStencilWindingCount != fAlgorithm) {
            quadPointInstanceData[indices->fWeightedTriangles++].setW(
                    vertices[i].fPos, vertices[i+1].fPos, vertices[i + 2].fPos, devToAtlasOffset,
                    static_cast<float>(abs(vertices[i].fWinding)));
        } else for (int j = 0; j < weight; ++j) {
            // Unfortunately, there is not a way to increment stencil values by an amount larger
            // than 1. Instead we draw the triangle 'weight' times.
            triPointInstanceData[indices->fTriangles++].set(
                    vertices[i].fPos, vertices[i + 1].fPos, vertices[i + 2].fPos, devToAtlasOffset,
                    ordering);
        }
    }
}

bool GrCCFiller::prepareToDraw(GrOnFlushResourceProvider* onFlushRP) {
    using Verb = GrCCFillGeometry::Verb;
    SkASSERT(!fInstanceBuffer);
    SkASSERT(fBatches.back().fEndNonScissorIndices == // Call closeCurrentBatch().
             fTotalPrimitiveCounts[(int)GrScissorTest::kDisabled]);
    SkASSERT(fBatches.back().fEndScissorSubBatchIdx == fScissorSubBatches.count());

    auto triangleOrdering = (Algorithm::kCoverageCount == fAlgorithm)
            ? TriPointInstance::Ordering::kXYTransposed
            : TriPointInstance::Ordering::kXYInterleaved;

    // Here we build a single instance buffer to share with every internal batch.
    //
    // CCPR processs 3 different types of primitives: triangles, quadratics, cubics. Each primitive
    // type is further divided into instances that require a scissor and those that don't. This
    // leaves us with 3*2 = 6 independent instance arrays to build for the GPU.
    //
    // Rather than place each instance array in its own GPU buffer, we allocate a single
    // megabuffer and lay them all out side-by-side. We can offset the "baseInstance" parameter in
    // our draw calls to direct the GPU to the applicable elements within a given array.
    //
    // We already know how big to make each of the 6 arrays from fTotalPrimitiveCounts, so layout is
    // straightforward. Start with triangles and quadratics. They both view the instance buffer as
    // an array of TriPointInstance[], so we can begin at zero and lay them out one after the other.
    fBaseInstances[0].fTriangles = 0;
    fBaseInstances[1].fTriangles = fBaseInstances[0].fTriangles +
                                   fTotalPrimitiveCounts[0].fTriangles;
    fBaseInstances[0].fQuadratics = fBaseInstances[1].fTriangles +
                                    fTotalPrimitiveCounts[1].fTriangles;
    fBaseInstances[1].fQuadratics = fBaseInstances[0].fQuadratics +
                                    fTotalPrimitiveCounts[0].fQuadratics;
    int triEndIdx = fBaseInstances[1].fQuadratics + fTotalPrimitiveCounts[1].fQuadratics;

    // Wound triangles and cubics both view the same instance buffer as an array of
    // QuadPointInstance[]. So, reinterpreting the instance data as QuadPointInstance[], we start
    // them on the first index that will not overwrite previous TriPointInstance data.
    int quadBaseIdx =
            GrSizeDivRoundUp(triEndIdx * sizeof(TriPointInstance), sizeof(QuadPointInstance));
    fBaseInstances[0].fWeightedTriangles = quadBaseIdx;
    fBaseInstances[1].fWeightedTriangles = fBaseInstances[0].fWeightedTriangles +
                                        fTotalPrimitiveCounts[0].fWeightedTriangles;
    fBaseInstances[0].fCubics = fBaseInstances[1].fWeightedTriangles +
                                fTotalPrimitiveCounts[1].fWeightedTriangles;
    fBaseInstances[1].fCubics = fBaseInstances[0].fCubics + fTotalPrimitiveCounts[0].fCubics;
    fBaseInstances[0].fConics = fBaseInstances[1].fCubics + fTotalPrimitiveCounts[1].fCubics;
    fBaseInstances[1].fConics = fBaseInstances[0].fConics + fTotalPrimitiveCounts[0].fConics;
    int quadEndIdx = fBaseInstances[1].fConics + fTotalPrimitiveCounts[1].fConics;

    fInstanceBuffer =
            onFlushRP->makeBuffer(GrGpuBufferType::kVertex, quadEndIdx * sizeof(QuadPointInstance));
    if (!fInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR fill instance buffer.\n");
        return false;
    }

    TriPointInstance* triPointInstanceData = static_cast<TriPointInstance*>(fInstanceBuffer->map());
    QuadPointInstance* quadPointInstanceData =
            reinterpret_cast<QuadPointInstance*>(triPointInstanceData);
    SkASSERT(quadPointInstanceData);

    PathInfo* nextPathInfo = fPathInfos.begin();
    Sk2f devToAtlasOffset;
    PrimitiveTallies instanceIndices[2] = {fBaseInstances[0], fBaseInstances[1]};
    PrimitiveTallies* currIndices = nullptr;
    SkSTArray<256, int32_t, true> currFan;
    bool currFanIsTessellated = false;

    const SkTArray<SkPoint, true>& pts = fGeometry.points();
    int ptsIdx = -1;
    int nextConicWeightIdx = 0;

    // Expand the ccpr verbs into GPU instance buffers.
    for (Verb verb : fGeometry.verbs()) {
        switch (verb) {
            case Verb::kBeginPath:
                SkASSERT(currFan.empty());
                currIndices = &instanceIndices[(int)nextPathInfo->scissorTest()];
                devToAtlasOffset = Sk2f(static_cast<float>(nextPathInfo->devToAtlasOffset().fX),
                                        static_cast<float>(nextPathInfo->devToAtlasOffset().fY));
                currFanIsTessellated = nextPathInfo->hasFanTessellation();
                if (currFanIsTessellated) {
                    this->emitTessellatedFan(
                            nextPathInfo->fanTessellation(), nextPathInfo->fanTessellationCount(),
                            devToAtlasOffset, triangleOrdering, triPointInstanceData,
                            quadPointInstanceData, currIndices);
                }
                ++nextPathInfo;
                continue;

            case Verb::kBeginContour:
                SkASSERT(currFan.empty());
                ++ptsIdx;
                if (!currFanIsTessellated) {
                    currFan.push_back(ptsIdx);
                }
                continue;

            case Verb::kLineTo:
                ++ptsIdx;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case Verb::kMonotonicQuadraticTo:
                triPointInstanceData[currIndices->fQuadratics++].set(
                        &pts[ptsIdx], devToAtlasOffset, TriPointInstance::Ordering::kXYTransposed);
                ptsIdx += 2;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case Verb::kMonotonicCubicTo:
                quadPointInstanceData[currIndices->fCubics++].set(
                        &pts[ptsIdx], devToAtlasOffset[0], devToAtlasOffset[1]);
                ptsIdx += 3;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case Verb::kMonotonicConicTo:
                quadPointInstanceData[currIndices->fConics++].setW(
                        &pts[ptsIdx], devToAtlasOffset,
                        fGeometry.getConicWeight(nextConicWeightIdx));
                ptsIdx += 2;
                ++nextConicWeightIdx;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case Verb::kEndClosedContour:  // endPt == startPt.
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.pop_back();
                }
            // fallthru.
            case Verb::kEndOpenContour:  // endPt != startPt.
                SkASSERT(!currFanIsTessellated || currFan.empty());
                if (!currFanIsTessellated && currFan.count() >= 3) {
                    int fanSize = currFan.count();
                    // Reserve space for emit_recursive_fan. Technically this can grow to
                    // fanSize + log3(fanSize), but we approximate with log2.
                    currFan.push_back_n(SkNextLog2(fanSize));
                    SkDEBUGCODE(TriPointInstance* end =) emit_recursive_fan(
                            pts, currFan, 0, fanSize, devToAtlasOffset, triangleOrdering,
                            triPointInstanceData + currIndices->fTriangles);
                    currIndices->fTriangles += fanSize - 2;
                    SkASSERT(triPointInstanceData + currIndices->fTriangles == end);
                }
                currFan.reset();
                continue;
        }
    }

    fInstanceBuffer->unmap();

    SkASSERT(nextPathInfo == fPathInfos.end());
    SkASSERT(ptsIdx == pts.count() - 1);
    SkASSERT(instanceIndices[0].fTriangles == fBaseInstances[1].fTriangles);
    SkASSERT(instanceIndices[1].fTriangles == fBaseInstances[0].fQuadratics);
    SkASSERT(instanceIndices[0].fQuadratics == fBaseInstances[1].fQuadratics);
    SkASSERT(instanceIndices[1].fQuadratics == triEndIdx);
    SkASSERT(instanceIndices[0].fWeightedTriangles == fBaseInstances[1].fWeightedTriangles);
    SkASSERT(instanceIndices[1].fWeightedTriangles == fBaseInstances[0].fCubics);
    SkASSERT(instanceIndices[0].fCubics == fBaseInstances[1].fCubics);
    SkASSERT(instanceIndices[1].fCubics == fBaseInstances[0].fConics);
    SkASSERT(instanceIndices[0].fConics == fBaseInstances[1].fConics);
    SkASSERT(instanceIndices[1].fConics == quadEndIdx);

    fMeshesScratchBuffer.reserve(fMaxMeshesPerDraw);
    fScissorRectScratchBuffer.reserve(fMaxMeshesPerDraw);

    return true;
}

void GrCCFiller::drawFills(
        GrOpFlushState* flushState, GrCCCoverageProcessor* proc, const GrPipeline& pipeline,
        BatchID batchID, const SkIRect& drawBounds) const {
    using PrimitiveType = GrCCCoverageProcessor::PrimitiveType;

    SkASSERT(fInstanceBuffer);

    GrResourceProvider* rp = flushState->resourceProvider();
    const PrimitiveTallies& batchTotalCounts = fBatches[batchID].fTotalPrimitiveCounts;

    if (batchTotalCounts.fTriangles) {
        proc->reset(PrimitiveType::kTriangles, rp);
        this->drawPrimitives(
                flushState, *proc, pipeline, batchID, &PrimitiveTallies::fTriangles, drawBounds);
    }

    if (batchTotalCounts.fWeightedTriangles) {
        SkASSERT(Algorithm::kStencilWindingCount != fAlgorithm);
        proc->reset(PrimitiveType::kWeightedTriangles, rp);
        this->drawPrimitives(
                flushState, *proc, pipeline, batchID, &PrimitiveTallies::fWeightedTriangles,
                drawBounds);
    }

    if (batchTotalCounts.fQuadratics) {
        proc->reset(PrimitiveType::kQuadratics, rp);
        this->drawPrimitives(
                flushState, *proc, pipeline, batchID, &PrimitiveTallies::fQuadratics, drawBounds);
    }

    if (batchTotalCounts.fCubics) {
        proc->reset(PrimitiveType::kCubics, rp);
        this->drawPrimitives(
                flushState, *proc, pipeline, batchID, &PrimitiveTallies::fCubics, drawBounds);
    }

    if (batchTotalCounts.fConics) {
        proc->reset(PrimitiveType::kConics, rp);
        this->drawPrimitives(
                flushState, *proc, pipeline, batchID, &PrimitiveTallies::fConics, drawBounds);
    }
}

void GrCCFiller::drawPrimitives(
        GrOpFlushState* flushState, const GrCCCoverageProcessor& proc, const GrPipeline& pipeline,
        BatchID batchID, int PrimitiveTallies::*instanceType, const SkIRect& drawBounds) const {
    SkASSERT(pipeline.isScissorEnabled());

    // Don't call reset(), as that also resets the reserve count.
    fMeshesScratchBuffer.pop_back_n(fMeshesScratchBuffer.count());
    fScissorRectScratchBuffer.pop_back_n(fScissorRectScratchBuffer.count());

    SkASSERT(batchID > 0);
    SkASSERT(batchID < fBatches.count());
    const Batch& previousBatch = fBatches[batchID - 1];
    const Batch& batch = fBatches[batchID];
    SkDEBUGCODE(int totalInstanceCount = 0);

    if (int instanceCount = batch.fEndNonScissorIndices.*instanceType -
                            previousBatch.fEndNonScissorIndices.*instanceType) {
        SkASSERT(instanceCount > 0);
        int baseInstance = fBaseInstances[(int)GrScissorTest::kDisabled].*instanceType +
                           previousBatch.fEndNonScissorIndices.*instanceType;
        proc.appendMesh(fInstanceBuffer, instanceCount, baseInstance, &fMeshesScratchBuffer);
        fScissorRectScratchBuffer.push_back().setXYWH(0, 0, drawBounds.width(),
                                                      drawBounds.height());
        SkDEBUGCODE(totalInstanceCount += instanceCount);
    }

    SkASSERT(previousBatch.fEndScissorSubBatchIdx > 0);
    SkASSERT(batch.fEndScissorSubBatchIdx <= fScissorSubBatches.count());
    int baseScissorInstance = fBaseInstances[(int)GrScissorTest::kEnabled].*instanceType;
    for (int i = previousBatch.fEndScissorSubBatchIdx; i < batch.fEndScissorSubBatchIdx; ++i) {
        const ScissorSubBatch& previousSubBatch = fScissorSubBatches[i - 1];
        const ScissorSubBatch& scissorSubBatch = fScissorSubBatches[i];
        int startIndex = previousSubBatch.fEndPrimitiveIndices.*instanceType;
        int instanceCount = scissorSubBatch.fEndPrimitiveIndices.*instanceType - startIndex;
        if (!instanceCount) {
            continue;
        }
        SkASSERT(instanceCount > 0);
        proc.appendMesh(fInstanceBuffer, instanceCount, baseScissorInstance + startIndex,
                        &fMeshesScratchBuffer);
        fScissorRectScratchBuffer.push_back() = scissorSubBatch.fScissor;
        SkDEBUGCODE(totalInstanceCount += instanceCount);
    }

    SkASSERT(fMeshesScratchBuffer.count() == fScissorRectScratchBuffer.count());
    SkASSERT(fMeshesScratchBuffer.count() <= fMaxMeshesPerDraw);
    SkASSERT(totalInstanceCount == batch.fTotalPrimitiveCounts.*instanceType);

    if (!fMeshesScratchBuffer.empty()) {
        proc.draw(flushState, pipeline, fScissorRectScratchBuffer.begin(),
                  fMeshesScratchBuffer.begin(), fMeshesScratchBuffer.count(),
                  SkRect::Make(drawBounds));
    }
}
