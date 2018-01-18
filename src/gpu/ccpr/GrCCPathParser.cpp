/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathParser.h"

#include "GrCaps.h"
#include "GrGpuCommandBuffer.h"
#include "GrOnFlushResourceProvider.h"
#include "GrOpFlushState.h"
#include "SkMathPriv.h"
#include "SkPath.h"
#include "SkPathPriv.h"
#include "SkPoint.h"
#include "ccpr/GrCCGeometry.h"

using TriangleInstance = GrCCCoverageProcessor::TriangleInstance;
using CubicInstance = GrCCCoverageProcessor::CubicInstance;

GrCCPathParser::GrCCPathParser(int maxTotalPaths, int maxPathPoints, int numSkPoints,
                               int numSkVerbs)
        : fLocalDevPtsBuffer(maxPathPoints + 1)  // Overallocate by one point to accomodate for
                                                 // overflow with Sk4f. (See parsePath.)
        , fGeometry(numSkPoints, numSkVerbs)
        , fPathsInfo(maxTotalPaths)
        , fScissorSubBatches(maxTotalPaths)
        , fTotalPrimitiveCounts{PrimitiveTallies(), PrimitiveTallies()} {
    // Batches decide what to draw by looking where the previous one ended. Define initial batches
    // that "end" at the beginning of the data. These will not be drawn, but will only be be read by
    // the first actual batch.
    fScissorSubBatches.push_back() = {PrimitiveTallies(), SkIRect::MakeEmpty()};
    fCoverageCountBatches.push_back() = {PrimitiveTallies(), fScissorSubBatches.count()};
}

void GrCCPathParser::parsePath(const SkMatrix& m, const SkPath& path, SkRect* devBounds,
                               SkRect* devBounds45) {
    const SkPoint* pts = SkPathPriv::PointData(path);
    int numPts = path.countPoints();
    SkASSERT(numPts + 1 <= fLocalDevPtsBuffer.count());

    if (!numPts) {
        devBounds->setEmpty();
        devBounds45->setEmpty();
        this->parsePath(path, nullptr);
        return;
    }

    // m45 transforms path points into "45 degree" device space. A bounding box in this space gives
    // the circumscribing octagon's diagonals. We could use SK_ScalarRoot2Over2, but an orthonormal
    // transform is not necessary as long as the shader uses the correct inverse.
    SkMatrix m45;
    m45.setSinCos(1, 1);
    m45.preConcat(m);

    // X,Y,T are two parallel view matrices that accumulate two bounding boxes as they map points:
    // device-space bounds and "45 degree" device-space bounds (| 1 -1 | * devCoords).
    //                                                          | 1  1 |
    Sk4f X = Sk4f(m.getScaleX(), m.getSkewY(), m45.getScaleX(), m45.getSkewY());
    Sk4f Y = Sk4f(m.getSkewX(), m.getScaleY(), m45.getSkewX(), m45.getScaleY());
    Sk4f T = Sk4f(m.getTranslateX(), m.getTranslateY(), m45.getTranslateX(), m45.getTranslateY());

    // Map the path's points to device space and accumulate bounding boxes.
    Sk4f devPt = SkNx_fma(Y, Sk4f(pts[0].y()), T);
    devPt = SkNx_fma(X, Sk4f(pts[0].x()), devPt);
    Sk4f topLeft = devPt;
    Sk4f bottomRight = devPt;

    // Store all 4 values [dev.x, dev.y, dev45.x, dev45.y]. We are only interested in the first two,
    // and will overwrite [dev45.x, dev45.y] with the next point. This is why the dst buffer must
    // be at least one larger than the number of points.
    devPt.store(&fLocalDevPtsBuffer[0]);

    for (int i = 1; i < numPts; ++i) {
        devPt = SkNx_fma(Y, Sk4f(pts[i].y()), T);
        devPt = SkNx_fma(X, Sk4f(pts[i].x()), devPt);
        topLeft = Sk4f::Min(topLeft, devPt);
        bottomRight = Sk4f::Max(bottomRight, devPt);
        devPt.store(&fLocalDevPtsBuffer[i]);
    }

    SkPoint topLeftPts[2], bottomRightPts[2];
    topLeft.store(topLeftPts);
    bottomRight.store(bottomRightPts);
    devBounds->setLTRB(topLeftPts[0].x(), topLeftPts[0].y(), bottomRightPts[0].x(),
                       bottomRightPts[0].y());
    devBounds45->setLTRB(topLeftPts[1].x(), topLeftPts[1].y(), bottomRightPts[1].x(),
                         bottomRightPts[1].y());

    this->parsePath(path, fLocalDevPtsBuffer.get());
}

void GrCCPathParser::parseDeviceSpacePath(const SkPath& deviceSpacePath) {
    this->parsePath(deviceSpacePath, SkPathPriv::PointData(deviceSpacePath));
}

void GrCCPathParser::parsePath(const SkPath& path, const SkPoint* deviceSpacePts) {
    SkASSERT(!fInstanceBuffer); // Can't call after finalize().
    SkASSERT(!fParsingPath); // Call saveParsedPath() or discardParsedPath() for the last one first.
    SkDEBUGCODE(fParsingPath = true);
    SkASSERT(path.isEmpty() || deviceSpacePts);

    fCurrPathPointsIdx = fGeometry.points().count();
    fCurrPathVerbsIdx = fGeometry.verbs().count();
    fCurrPathPrimitiveCounts = PrimitiveTallies();

    fGeometry.beginPath();

    if (path.isEmpty()) {
        return;
    }

    int ptsIdx = 0;
    bool insideContour = false;

    for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
        switch (verb) {
            case SkPath::kMove_Verb:
                this->endContourIfNeeded(insideContour);
                fGeometry.beginContour(deviceSpacePts[ptsIdx]);
                ++ptsIdx;
                insideContour = true;
                continue;
            case SkPath::kClose_Verb:
                this->endContourIfNeeded(insideContour);
                insideContour = false;
                continue;
            case SkPath::kLine_Verb:
                fGeometry.lineTo(deviceSpacePts[ptsIdx]);
                ++ptsIdx;
                continue;
            case SkPath::kQuad_Verb:
                fGeometry.quadraticTo(deviceSpacePts[ptsIdx], deviceSpacePts[ptsIdx + 1]);
                ptsIdx += 2;
                continue;
            case SkPath::kCubic_Verb:
                fGeometry.cubicTo(deviceSpacePts[ptsIdx], deviceSpacePts[ptsIdx + 1],
                                  deviceSpacePts[ptsIdx + 2]);
                ptsIdx += 3;
                continue;
            case SkPath::kConic_Verb:
                SK_ABORT("Conics are not supported.");
            default:
                SK_ABORT("Unexpected path verb.");
        }
    }

    this->endContourIfNeeded(insideContour);
}

void GrCCPathParser::endContourIfNeeded(bool insideContour) {
    if (insideContour) {
        fCurrPathPrimitiveCounts += fGeometry.endContour();
    }
}

void GrCCPathParser::saveParsedPath(ScissorMode scissorMode, const SkIRect& clippedDevIBounds,
                                    int16_t atlasOffsetX, int16_t atlasOffsetY) {
    SkASSERT(fParsingPath);

    fPathsInfo.push_back() = {scissorMode, atlasOffsetX, atlasOffsetY};
    fTotalPrimitiveCounts[(int)scissorMode] += fCurrPathPrimitiveCounts;

    if (ScissorMode::kScissored == scissorMode) {
        fScissorSubBatches.push_back() = {fTotalPrimitiveCounts[(int)ScissorMode::kScissored],
                                          clippedDevIBounds.makeOffset(atlasOffsetX, atlasOffsetY)};
    }

    SkDEBUGCODE(fParsingPath = false);
}

void GrCCPathParser::discardParsedPath() {
    SkASSERT(fParsingPath);
    fGeometry.resize_back(fCurrPathPointsIdx, fCurrPathVerbsIdx);
    SkDEBUGCODE(fParsingPath = false);
}

GrCCPathParser::CoverageCountBatchID GrCCPathParser::closeCurrentBatch() {
    SkASSERT(!fInstanceBuffer);
    SkASSERT(!fCoverageCountBatches.empty());

    int maxMeshes = 1 + fScissorSubBatches.count() -
                        fCoverageCountBatches.back().fEndScissorSubBatchIdx;
    fMaxMeshesPerDraw = SkTMax(fMaxMeshesPerDraw, maxMeshes);

    fCoverageCountBatches.push_back() = {
        fTotalPrimitiveCounts[(int)ScissorMode::kNonScissored],
        fScissorSubBatches.count()
    };
    return fCoverageCountBatches.count() - 1;
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
static TriangleInstance* emit_recursive_fan(const SkTArray<SkPoint, true>& pts,
                                            SkTArray<int32_t, true>& indices, int firstIndex,
                                            int indexCount, const Sk2f& atlasOffset,
                                            TriangleInstance out[]) {
    if (indexCount < 3) {
        return out;
    }

    int32_t oneThirdCount = indexCount / 3;
    int32_t twoThirdsCount = (2 * indexCount) / 3;
    out++->set(pts[indices[firstIndex]], pts[indices[firstIndex + oneThirdCount]],
               pts[indices[firstIndex + twoThirdsCount]], atlasOffset);

    out = emit_recursive_fan(pts, indices, firstIndex, oneThirdCount + 1, atlasOffset, out);
    out = emit_recursive_fan(pts, indices, firstIndex + oneThirdCount,
                             twoThirdsCount - oneThirdCount + 1, atlasOffset, out);

    int endIndex = firstIndex + indexCount;
    int32_t oldValue = indices[endIndex];
    indices[endIndex] = indices[firstIndex];
    out = emit_recursive_fan(pts, indices, firstIndex + twoThirdsCount,
                             indexCount - twoThirdsCount + 1, atlasOffset, out);
    indices[endIndex] = oldValue;

    return out;
}

bool GrCCPathParser::finalize(GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(!fParsingPath); // Call saveParsedPath() or discardParsedPath().
    SkASSERT(fCoverageCountBatches.back().fEndNonScissorIndices == // Call closeCurrentBatch().
             fTotalPrimitiveCounts[(int)ScissorMode::kNonScissored]);
    SkASSERT(fCoverageCountBatches.back().fEndScissorSubBatchIdx == fScissorSubBatches.count());

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
    // an array of TriangleInstance[], so we can begin at zero and lay them out one after the other.
    fBaseInstances[0].fTriangles = 0;
    fBaseInstances[1].fTriangles = fBaseInstances[0].fTriangles +
                                   fTotalPrimitiveCounts[0].fTriangles;
    fBaseInstances[0].fQuadratics = fBaseInstances[1].fTriangles +
                                    fTotalPrimitiveCounts[1].fTriangles;
    fBaseInstances[1].fQuadratics = fBaseInstances[0].fQuadratics +
                                    fTotalPrimitiveCounts[0].fQuadratics;
    int triEndIdx = fBaseInstances[1].fQuadratics + fTotalPrimitiveCounts[1].fQuadratics;

    // Cubics view the same instance buffer as an array of CubicInstance[]. So, reinterpreting the
    // instance data as CubicInstance[], we start them on the first index that will not overwrite
    // previous TriangleInstance data.
    int cubicBaseIdx =
            GR_CT_DIV_ROUND_UP(triEndIdx * sizeof(TriangleInstance), sizeof(CubicInstance));
    fBaseInstances[0].fCubics = cubicBaseIdx;
    fBaseInstances[1].fCubics = fBaseInstances[0].fCubics + fTotalPrimitiveCounts[0].fCubics;
    int cubicEndIdx = fBaseInstances[1].fCubics + fTotalPrimitiveCounts[1].fCubics;

    fInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType,
                                            cubicEndIdx * sizeof(CubicInstance));
    if (!fInstanceBuffer) {
        return false;
    }

    TriangleInstance* triangleInstanceData = static_cast<TriangleInstance*>(fInstanceBuffer->map());
    CubicInstance* cubicInstanceData = reinterpret_cast<CubicInstance*>(triangleInstanceData);
    SkASSERT(cubicInstanceData);

    PathInfo* currPathInfo = fPathsInfo.begin();
    float atlasOffsetX = 0.0, atlasOffsetY = 0.0;
    Sk2f atlasOffset;
    int ptsIdx = -1;
    PrimitiveTallies instanceIndices[2] = {fBaseInstances[0], fBaseInstances[1]};
    PrimitiveTallies* currIndices = nullptr;
    SkSTArray<256, int32_t, true> currFan;

    const SkTArray<SkPoint, true>& pts = fGeometry.points();

    // Expand the ccpr verbs into GPU instance buffers.
    for (GrCCGeometry::Verb verb : fGeometry.verbs()) {
        switch (verb) {
            case GrCCGeometry::Verb::kBeginPath:
                SkASSERT(currFan.empty());
                currIndices = &instanceIndices[(int)currPathInfo->fScissorMode];
                atlasOffsetX = static_cast<float>(currPathInfo->fAtlasOffsetX);
                atlasOffsetY = static_cast<float>(currPathInfo->fAtlasOffsetY);
                atlasOffset = {atlasOffsetX, atlasOffsetY};
                ++currPathInfo;
                continue;

            case GrCCGeometry::Verb::kBeginContour:
                SkASSERT(currFan.empty());
                currFan.push_back(++ptsIdx);
                continue;

            case GrCCGeometry::Verb::kLineTo:
                SkASSERT(!currFan.empty());
                currFan.push_back(++ptsIdx);
                continue;

            case GrCCGeometry::Verb::kMonotonicQuadraticTo:
                SkASSERT(!currFan.empty());
                triangleInstanceData[currIndices->fQuadratics++].set(&pts[ptsIdx], atlasOffset);
                currFan.push_back(ptsIdx += 2);
                continue;

            case GrCCGeometry::Verb::kMonotonicCubicTo:
                SkASSERT(!currFan.empty());
                cubicInstanceData[currIndices->fCubics++].set(&pts[ptsIdx], atlasOffsetX,
                                                              atlasOffsetY);
                currFan.push_back(ptsIdx += 3);
                continue;

            case GrCCGeometry::Verb::kEndClosedContour:  // endPt == startPt.
                SkASSERT(!currFan.empty());
                currFan.pop_back();
            // fallthru.
            case GrCCGeometry::Verb::kEndOpenContour:  // endPt != startPt.
                if (currFan.count() >= 3) {
                    int fanSize = currFan.count();
                    // Reserve space for emit_recursive_fan. Technically this can grow to
                    // fanSize + log3(fanSize), but we approximate with log2.
                    currFan.push_back_n(SkNextLog2(fanSize));
                    SkDEBUGCODE(TriangleInstance* end =)
                            emit_recursive_fan(pts, currFan, 0, fanSize, atlasOffset,
                                               triangleInstanceData + currIndices->fTriangles);
                    currIndices->fTriangles += fanSize - 2;
                    SkASSERT(triangleInstanceData + currIndices->fTriangles == end);
                }
                currFan.reset();
                continue;
        }
    }

    fInstanceBuffer->unmap();

    SkASSERT(currPathInfo == fPathsInfo.end());
    SkASSERT(ptsIdx == pts.count() - 1);
    SkASSERT(instanceIndices[0].fTriangles == fBaseInstances[1].fTriangles);
    SkASSERT(instanceIndices[1].fTriangles == fBaseInstances[0].fQuadratics);
    SkASSERT(instanceIndices[0].fQuadratics == fBaseInstances[1].fQuadratics);
    SkASSERT(instanceIndices[1].fQuadratics == triEndIdx);
    SkASSERT(instanceIndices[0].fCubics == fBaseInstances[1].fCubics);
    SkASSERT(instanceIndices[1].fCubics == cubicEndIdx);

    fMeshesScratchBuffer.reserve(fMaxMeshesPerDraw);
    fDynamicStatesScratchBuffer.reserve(fMaxMeshesPerDraw);

    return true;
}

void GrCCPathParser::drawCoverageCount(GrOpFlushState* flushState, CoverageCountBatchID batchID,
                                       const SkIRect& drawBounds) const {
    using RenderPass = GrCCCoverageProcessor::RenderPass;

    SkASSERT(fInstanceBuffer);

    GrPipeline pipeline(flushState->drawOpArgs().fProxy, GrPipeline::ScissorState::kEnabled,
                        SkBlendMode::kPlus);

    // Triangles.
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kTriangleHulls,
                         &PrimitiveTallies::fTriangles, drawBounds);
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kTriangleEdges,
                         &PrimitiveTallies::fTriangles, drawBounds);  // Might get skipped.
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kTriangleCorners,
                         &PrimitiveTallies::fTriangles, drawBounds);

    // Quadratics.
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kQuadraticHulls,
                         &PrimitiveTallies::fQuadratics, drawBounds);
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kQuadraticCorners,
                         &PrimitiveTallies::fQuadratics, drawBounds);

    // Cubics.
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kCubicHulls,
                         &PrimitiveTallies::fCubics, drawBounds);
    this->drawRenderPass(flushState, pipeline, batchID, RenderPass::kCubicCorners,
                         &PrimitiveTallies::fCubics, drawBounds);
}

void GrCCPathParser::drawRenderPass(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                    CoverageCountBatchID batchID,
                                    GrCCCoverageProcessor::RenderPass renderPass,
                                    int PrimitiveTallies::*instanceType,
                                    const SkIRect& drawBounds) const {
    SkASSERT(pipeline.getScissorState().enabled());

    if (!GrCCCoverageProcessor::DoesRenderPass(renderPass, *flushState->caps().shaderCaps())) {
        return;
    }

    // Don't call reset(), as that also resets the reserve count.
    fMeshesScratchBuffer.pop_back_n(fMeshesScratchBuffer.count());
    fDynamicStatesScratchBuffer.pop_back_n(fDynamicStatesScratchBuffer.count());

    GrCCCoverageProcessor proc(flushState->resourceProvider(), renderPass,
                               *flushState->caps().shaderCaps());

    SkASSERT(batchID > 0);
    SkASSERT(batchID < fCoverageCountBatches.count());
    const CoverageCountBatch& previousBatch = fCoverageCountBatches[batchID - 1];
    const CoverageCountBatch& batch = fCoverageCountBatches[batchID];

    if (int instanceCount = batch.fEndNonScissorIndices.*instanceType -
                            previousBatch.fEndNonScissorIndices.*instanceType) {
        SkASSERT(instanceCount > 0);
        int baseInstance = fBaseInstances[(int)ScissorMode::kNonScissored].*instanceType +
                           previousBatch.fEndNonScissorIndices.*instanceType;
        proc.appendMesh(fInstanceBuffer.get(), instanceCount, baseInstance, &fMeshesScratchBuffer);
        fDynamicStatesScratchBuffer.push_back().fScissorRect.setXYWH(0, 0, drawBounds.width(),
                                                                     drawBounds.height());
    }

    SkASSERT(previousBatch.fEndScissorSubBatchIdx > 0);
    SkASSERT(batch.fEndScissorSubBatchIdx <= fScissorSubBatches.count());
    int baseScissorInstance = fBaseInstances[(int)ScissorMode::kScissored].*instanceType;
    for (int i = previousBatch.fEndScissorSubBatchIdx; i < batch.fEndScissorSubBatchIdx; ++i) {
        const ScissorSubBatch& previousSubBatch = fScissorSubBatches[i - 1];
        const ScissorSubBatch& scissorSubBatch = fScissorSubBatches[i];
        int startIndex = previousSubBatch.fEndPrimitiveIndices.*instanceType;
        int instanceCount = scissorSubBatch.fEndPrimitiveIndices.*instanceType - startIndex;
        if (!instanceCount) {
            continue;
        }
        SkASSERT(instanceCount > 0);
        proc.appendMesh(fInstanceBuffer.get(), instanceCount,
                        baseScissorInstance + startIndex, &fMeshesScratchBuffer);
        fDynamicStatesScratchBuffer.push_back().fScissorRect = scissorSubBatch.fScissor;
    }

    SkASSERT(fMeshesScratchBuffer.count() == fDynamicStatesScratchBuffer.count());
    SkASSERT(fMeshesScratchBuffer.count() <= fMaxMeshesPerDraw);

    if (!fMeshesScratchBuffer.empty()) {
        SkASSERT(flushState->rtCommandBuffer());
        flushState->rtCommandBuffer()->draw(pipeline, proc, fMeshesScratchBuffer.begin(),
                                            fDynamicStatesScratchBuffer.begin(),
                                            fMeshesScratchBuffer.count(), SkRect::Make(drawBounds));
    }
}
