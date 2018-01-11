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
#include "ccpr/GrCCPRAtlas.h"
#include "ccpr/GrCCPRGeometry.h"

using TriangleInstance = GrCCPRCoverageProcessor::TriangleInstance;
using CubicInstance = GrCCPRCoverageProcessor::CubicInstance;

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
    devBounds->setLTRB(topLeftPts[0].x(), topLeftPts[0].y(),
                       bottomRightPts[0].x(), bottomRightPts[0].y());
    devBounds45->setLTRB(topLeftPts[1].x(), topLeftPts[1].y(),
                         bottomRightPts[1].x(), bottomRightPts[1].y());

    this->parsePath(path, fLocalDevPtsBuffer.get());
}

void GrCCPathParser::parseDeviceSpacePath(const SkPath& deviceSpacePath) {
    this->parsePath(deviceSpacePath, SkPathPriv::PointData(deviceSpacePath));
}

void GrCCPathParser::parsePath(const SkPath& path, const SkPoint* deviceSpacePts) {
    SkASSERT(!fInstanceBuffer);
    SkASSERT(!fParsingPath);
    SkDEBUGCODE(fParsingPath = true);
    SkASSERT(path.isEmpty() || deviceSpacePts);

    fCurrPathPointsIdx = fGeometry.points().count();
    fCurrPathVerbsIdx = fGeometry.verbs().count();
    fCurrPathTallies = PrimitiveTallies();

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
        fCurrPathTallies += fGeometry.endContour();
    }
}

const GrCCPathParser::PrimitiveTallies& GrCCPathParser::saveParsedPath(ScissorMode scissorMode,
                                                                       const SkIRect&
                                                                       clippedDevIBounds,
                                                                       int16_t atlasOffsetX,
                                                                       int16_t atlasOffsetY) {
    SkASSERT(fParsingPath);

    fPathsInfo.push_back() = {
        scissorMode,
        atlasOffsetX, atlasOffsetY
    };

    fInstanceCounts[(int)scissorMode] += fCurrPathTallies;
    SkDEBUGCODE(fParsingPath = false);
    return fCurrPathTallies;
}

void GrCCPathParser::discardParsedPath() {
    SkASSERT(fParsingPath);
    fGeometry.resize_back(fCurrPathPointsIdx, fCurrPathVerbsIdx);
    SkDEBUGCODE(fParsingPath = false);
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

    const int32_t oneThirdCount = indexCount / 3;
    const int32_t twoThirdsCount = (2 * indexCount) / 3;
    out++->set(pts[indices[firstIndex]],
               pts[indices[firstIndex + oneThirdCount]],
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
    SkASSERT(!fParsingPath);

    // Here we build a single instance buffer to share with every draw call from every CoverageOP we
    // plan to produce.
    //
    // CoverageOps process 3 different types of primitives (triangles, quadratics, cubics), and each
    // primitive type is further divided into instances that require a scissor and those that don't.
    // This leaves us with 6 independent instance arrays to build for the GPU.
    //
    // Rather than placing each instance array in its own GPU buffer, we allocate a single
    // megabuffer and lay them all out side-by-side. We can offset the "baseInstance" parameter in
    // our draw calls to direct the GPU to the applicable elements within a given array.
    //
    // We already know how big to make each of the 6 arrays from fInstanceCounts[kNumScissorModes],
    // so layout is straightforward. Start with triangles and quadratics. They both view the
    // instance buffer as an array of TriangleInstance[], so we can just start at zero and lay them
    // out one after the other.
    fBaseInstances[0].fTriangles = 0;
    fBaseInstances[1].fTriangles = fBaseInstances[0].fTriangles + fInstanceCounts[0].fTriangles;
    fBaseInstances[0].fQuadratics = fBaseInstances[1].fTriangles + fInstanceCounts[1].fTriangles;
    fBaseInstances[1].fQuadratics = fBaseInstances[0].fQuadratics + fInstanceCounts[0].fQuadratics;
    int triEndIdx = fBaseInstances[1].fQuadratics + fInstanceCounts[1].fQuadratics;

    // Cubics (loops and serpentines) view the same instance buffer as an array of CubicInstance[].
    // So, reinterpreting the instance data as CubicInstance[], we start them on the first index
    // that will not overwrite previous TriangleInstance data.
    int cubicBaseIdx = GR_CT_DIV_ROUND_UP(triEndIdx * sizeof(TriangleInstance),
                                          sizeof(CubicInstance));
    fBaseInstances[0].fCubics = cubicBaseIdx;
    fBaseInstances[1].fCubics = fBaseInstances[0].fCubics + fInstanceCounts[0].fCubics;
    int cubicEndIdx = fBaseInstances[1].fCubics + fInstanceCounts[1].fCubics;

    fInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType,
                                            cubicEndIdx * sizeof(CubicInstance));
    if (!fInstanceBuffer) {
        return false;
    }

    TriangleInstance* triangleInstanceData = static_cast<TriangleInstance*>(fInstanceBuffer->map());
    CubicInstance* cubicInstanceData = reinterpret_cast<CubicInstance*>(triangleInstanceData);
    SkASSERT(cubicInstanceData);

    PathInfo* nextPathInfo = fPathsInfo.begin();
    float atlasOffsetX, atlasOffsetY;
    Sk2f atlasOffset;
    int ptsIdx = -1;
    PrimitiveTallies instanceIndices[2] = {fBaseInstances[0], fBaseInstances[1]};
    PrimitiveTallies* currIndices;
    SkSTArray<256, int32_t, true> currFan;

    const SkTArray<SkPoint, true>& pts = fGeometry.points();

    // Expand the ccpr verbs into GPU instance buffers.
    for (GrCCPRGeometry::Verb verb : fGeometry.verbs()) {
        switch (verb) {
            case GrCCPRGeometry::Verb::kBeginPath:
                SkASSERT(currFan.empty());
                currIndices = &instanceIndices[(int)nextPathInfo->fScissorMode];
                atlasOffsetX = static_cast<float>(nextPathInfo->fAtlasOffsetX);
                atlasOffsetY = static_cast<float>(nextPathInfo->fAtlasOffsetY);
                atlasOffset = {atlasOffsetX, atlasOffsetY};
                ++nextPathInfo;
                continue;

            case GrCCPRGeometry::Verb::kBeginContour:
                SkASSERT(currFan.empty());
                currFan.push_back(++ptsIdx);
                continue;

            case GrCCPRGeometry::Verb::kLineTo:
                SkASSERT(!currFan.empty());
                currFan.push_back(++ptsIdx);
                continue;

            case GrCCPRGeometry::Verb::kMonotonicQuadraticTo:
                SkASSERT(!currFan.empty());
                triangleInstanceData[currIndices->fQuadratics++].set(&pts[ptsIdx], atlasOffset);
                currFan.push_back(ptsIdx += 2);
                continue;

            case GrCCPRGeometry::Verb::kMonotonicCubicTo:
                SkASSERT(!currFan.empty());
                cubicInstanceData[currIndices->fCubics++].set(&pts[ptsIdx],
                                                              atlasOffsetX, atlasOffsetY);
                currFan.push_back(ptsIdx += 3);
                continue;

            case GrCCPRGeometry::Verb::kEndClosedContour: // endPt == startPt.
                SkASSERT(!currFan.empty());
                currFan.pop_back();
                // fallthru.
            case GrCCPRGeometry::Verb::kEndOpenContour: // endPt != startPt.
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

    SkASSERT(nextPathInfo == fPathsInfo.end());
    SkASSERT(ptsIdx == pts.count() - 1);
    SkASSERT(instanceIndices[0].fTriangles == fBaseInstances[1].fTriangles);
    SkASSERT(instanceIndices[1].fTriangles == fBaseInstances[0].fQuadratics);
    SkASSERT(instanceIndices[0].fQuadratics == fBaseInstances[1].fQuadratics);
    SkASSERT(instanceIndices[1].fQuadratics == triEndIdx);
    SkASSERT(instanceIndices[0].fCubics == fBaseInstances[1].fCubics);
    SkASSERT(instanceIndices[1].fCubics == cubicEndIdx);
    return true;
}

void GrCCPathParser::drawCoverageCount(GrOpFlushState* flushState, const SkIRect& drawBounds,
                                       const GrPipeline& pipeline,
                                       const PrimitiveTallies startIndices[kNumScissorModes],
                                       const PrimitiveTallies& unscissoredInstanceCounts,
                                       const SkTArray<ScissorBatch, true>& scissorBatches) const {
    using RenderPass = GrCCPRCoverageProcessor::RenderPass;

    SkASSERT(fInstanceBuffer);

    fMeshesScratchBuffer.reserve(1 + scissorBatches.count());
    fDynamicStatesScratchBuffer.reserve(1 + scissorBatches.count());

    // Triangles.
    this->drawRenderPass(RenderPass::kTriangleHulls, flushState, drawBounds, pipeline, startIndices,
                         unscissoredInstanceCounts, scissorBatches, &PrimitiveTallies::fTriangles);
    // this->drawRenderPass(RenderPass::kTriangleEdges, flushState, drawBounds, pipeline, startIndices,
    //                      unscissoredInstanceCounts, scissorBatches,
    //                      &PrimitiveTallies::fTriangles); // Might get skipped.
    // this->drawRenderPass(RenderPass::kTriangleCorners, flushState, drawBounds, pipeline,
    //                      startIndices, unscissoredInstanceCounts, scissorBatches,
    //                      &PrimitiveTallies::fTriangles);
    //
    // // Quadratics.
    // this->drawRenderPass(RenderPass::kQuadraticHulls, flushState, drawBounds, pipeline,
    //                      startIndices, unscissoredInstanceCounts, scissorBatches,
    //                      &PrimitiveTallies::fQuadratics);
    // this->drawRenderPass(RenderPass::kQuadraticCorners, flushState, drawBounds, pipeline,
    //                      startIndices, unscissoredInstanceCounts, scissorBatches,
    //                      &PrimitiveTallies::fQuadratics);
    //
    // // Cubics.
    // this->drawRenderPass(RenderPass::kCubicHulls, flushState, drawBounds, pipeline, startIndices,
    //                      unscissoredInstanceCounts, scissorBatches, &PrimitiveTallies::fCubics);
    // this->drawRenderPass(RenderPass::kCubicCorners, flushState, drawBounds, pipeline, startIndices,
    //                      unscissoredInstanceCounts, scissorBatches, &PrimitiveTallies::fCubics);
}

void GrCCPathParser::drawRenderPass(GrCCPRCoverageProcessor::RenderPass renderPass,
                                    GrOpFlushState* flushState, const SkIRect& drawBounds,
                                    const GrPipeline& pipeline,
                                    const PrimitiveTallies startIndices[kNumScissorModes],
                                    const PrimitiveTallies& unscissoredInstanceCounts,
                                    const SkTArray<ScissorBatch, true>& scissorBatches,
                                    int PrimitiveTallies::* instanceType) const {
    SkASSERT(pipeline.getScissorState().enabled());

    if (!GrCCPRCoverageProcessor::DoesRenderPass(renderPass, *flushState->caps().shaderCaps())) {
        return;
    }

    fMeshesScratchBuffer.reset();
    fDynamicStatesScratchBuffer.reset();

    GrCCPRCoverageProcessor proc(flushState->resourceProvider(), renderPass,
                                 GrDrawBuffer::kColor == pipeline.drawBuffer() ?
                                 GrCCPRCoverageProcessor::Output::kAlpha :
                                 GrCCPRCoverageProcessor::Output::kCoverageCount,
                                 *flushState->caps().shaderCaps());
    // proc.enableDebugVisualizations(.5f);

    if (int instanceCount = unscissoredInstanceCounts.*instanceType) {
        SkASSERT(instanceCount > 0);
        int baseInstance = fBaseInstances[(int)ScissorMode::kNonScissored].*instanceType +
                           startIndices[(int)ScissorMode::kNonScissored].*instanceType;
        proc.appendMesh(fInstanceBuffer.get(), instanceCount, baseInstance, &fMeshesScratchBuffer);
        fDynamicStatesScratchBuffer.push_back().fScissorRect = drawBounds;
    }

    int baseInstance = fBaseInstances[(int)ScissorMode::kScissored].*instanceType +
                       startIndices[(int)ScissorMode::kScissored].*instanceType;
    for (const ScissorBatch& batch : scissorBatches) {
        SkASSERT(drawBounds.contains(batch.fScissor));
        const int instanceCount = batch.fInstanceCounts.*instanceType;
        if (!instanceCount) {
            continue;
        }
        SkASSERT(instanceCount > 0);
        proc.appendMesh(fInstanceBuffer.get(), instanceCount, baseInstance, &fMeshesScratchBuffer);
        fDynamicStatesScratchBuffer.push_back().fScissorRect = batch.fScissor;
        baseInstance += instanceCount;
    }

    SkASSERT(fMeshesScratchBuffer.count() == fDynamicStatesScratchBuffer.count());

    if (!fMeshesScratchBuffer.empty()) {
        SkASSERT(flushState->rtCommandBuffer());
        flushState->rtCommandBuffer()->draw(pipeline, proc, fMeshesScratchBuffer.begin(),
                                            fDynamicStatesScratchBuffer.begin(),
                                            fMeshesScratchBuffer.count(), SkRect::Make(drawBounds));
    }
}
