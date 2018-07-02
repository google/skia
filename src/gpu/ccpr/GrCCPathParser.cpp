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
#include <stdlib.h>

using TriPointInstance = GrCCCoverageProcessor::TriPointInstance;
using QuadPointInstance = GrCCCoverageProcessor::QuadPointInstance;

GrCCPathParser::GrCCPathParser(int numPaths, const PathStats& pathStats)
          // Overallocate by one point to accomodate for overflow with Sk4f. (See parsePath.)
        : fLocalDevPtsBuffer(pathStats.fMaxPointsPerPath + 1)
        , fGeometry(pathStats.fNumTotalSkPoints, pathStats.fNumTotalSkVerbs,
                    pathStats.fNumTotalConicWeights)
        , fPathsInfo(numPaths)
        , fScissorSubBatches(numPaths)
        , fTotalPrimitiveCounts{PrimitiveTallies(), PrimitiveTallies()} {
    // Batches decide what to draw by looking where the previous one ended. Define initial batches
    // that "end" at the beginning of the data. These will not be drawn, but will only be be read by
    // the first actual batch.
    fScissorSubBatches.push_back() = {PrimitiveTallies(), SkIRect::MakeEmpty()};
    fCoverageCountBatches.push_back() = {PrimitiveTallies(), fScissorSubBatches.count(),
                                         PrimitiveTallies()};
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

    const float* conicWeights = SkPathPriv::ConicWeightData(path);
    int ptsIdx = 0;
    int conicWeightsIdx = 0;
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

    this->endContourIfNeeded(insideContour);
}

void GrCCPathParser::endContourIfNeeded(bool insideContour) {
    if (insideContour) {
        fCurrPathPrimitiveCounts += fGeometry.endContour();
    }
}

void GrCCPathParser::saveParsedPath(ScissorMode scissorMode, const SkIRect& clippedDevIBounds,
                                    const SkIVector& devToAtlasOffset) {
    SkASSERT(fParsingPath);

    fPathsInfo.emplace_back(scissorMode, devToAtlasOffset);

    // Tessellate fans from very large and/or simple paths, in order to reduce overdraw.
    int numVerbs = fGeometry.verbs().count() - fCurrPathVerbsIdx - 1;
    int64_t tessellationWork = (int64_t)numVerbs * (32 - SkCLZ(numVerbs)); // N log N.
    int64_t fanningWork = (int64_t)clippedDevIBounds.height() * clippedDevIBounds.width();
    if (tessellationWork * (50*50) + (100*100) < fanningWork) { // Don't tessellate under 100x100.
        fCurrPathPrimitiveCounts.fTriangles =
                fCurrPathPrimitiveCounts.fWeightedTriangles = 0;

        const SkTArray<GrCCGeometry::Verb, true>& verbs = fGeometry.verbs();
        const SkTArray<SkPoint, true>& pts = fGeometry.points();
        int ptsIdx = fCurrPathPointsIdx;

        // Build an SkPath of the Redbook fan. We use "winding" fill type right now because we are
        // producing a coverage count, and must fill in every region that has non-zero wind. The
        // path processor will convert coverage count to the appropriate fill type later.
        SkPath fan;
        fan.setFillType(SkPath::kWinding_FillType);
        SkASSERT(GrCCGeometry::Verb::kBeginPath == verbs[fCurrPathVerbsIdx]);
        for (int i = fCurrPathVerbsIdx + 1; i < fGeometry.verbs().count(); ++i) {
            switch (verbs[i]) {
                case GrCCGeometry::Verb::kBeginPath:
                    SK_ABORT("Invalid GrCCGeometry");
                    continue;

                case GrCCGeometry::Verb::kBeginContour:
                    fan.moveTo(pts[ptsIdx++]);
                    continue;

                case GrCCGeometry::Verb::kLineTo:
                    fan.lineTo(pts[ptsIdx++]);
                    continue;

                case GrCCGeometry::Verb::kMonotonicQuadraticTo:
                case GrCCGeometry::Verb::kMonotonicConicTo:
                    fan.lineTo(pts[ptsIdx + 1]);
                    ptsIdx += 2;
                    continue;

                case GrCCGeometry::Verb::kMonotonicCubicTo:
                    fan.lineTo(pts[ptsIdx + 2]);
                    ptsIdx += 3;
                    continue;

                case GrCCGeometry::Verb::kEndClosedContour:
                case GrCCGeometry::Verb::kEndOpenContour:
                    fan.close();
                    continue;
            }
        }
        GrTessellator::WindingVertex* vertices = nullptr;
        int count = GrTessellator::PathToVertices(fan, std::numeric_limits<float>::infinity(),
                                                  SkRect::Make(clippedDevIBounds), &vertices);
        SkASSERT(0 == count % 3);
        for (int i = 0; i < count; i += 3) {
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

            if (1 == abs(tessWinding)) {
                ++fCurrPathPrimitiveCounts.fTriangles;
            } else {
                ++fCurrPathPrimitiveCounts.fWeightedTriangles;
            }
        }

        fPathsInfo.back().adoptFanTessellation(vertices, count);
    }

    fTotalPrimitiveCounts[(int)scissorMode] += fCurrPathPrimitiveCounts;

    if (ScissorMode::kScissored == scissorMode) {
        fScissorSubBatches.push_back() = {fTotalPrimitiveCounts[(int)ScissorMode::kScissored],
                                          clippedDevIBounds.makeOffset(devToAtlasOffset.fX,
                                                                       devToAtlasOffset.fY)};
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

    const auto& lastBatch = fCoverageCountBatches.back();
    int maxMeshes = 1 + fScissorSubBatches.count() - lastBatch.fEndScissorSubBatchIdx;
    fMaxMeshesPerDraw = SkTMax(fMaxMeshesPerDraw, maxMeshes);

    const auto& lastScissorSubBatch = fScissorSubBatches[lastBatch.fEndScissorSubBatchIdx - 1];
    PrimitiveTallies batchTotalCounts = fTotalPrimitiveCounts[(int)ScissorMode::kNonScissored] -
                                        lastBatch.fEndNonScissorIndices;
    batchTotalCounts += fTotalPrimitiveCounts[(int)ScissorMode::kScissored] -
                        lastScissorSubBatch.fEndPrimitiveIndices;

    // This will invalidate lastBatch.
    fCoverageCountBatches.push_back() = {
        fTotalPrimitiveCounts[(int)ScissorMode::kNonScissored],
        fScissorSubBatches.count(),
        batchTotalCounts
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
static TriPointInstance* emit_recursive_fan(const SkTArray<SkPoint, true>& pts,
                                            SkTArray<int32_t, true>& indices, int firstIndex,
                                            int indexCount, const Sk2f& devToAtlasOffset,
                                            TriPointInstance out[]) {
    if (indexCount < 3) {
        return out;
    }

    int32_t oneThirdCount = indexCount / 3;
    int32_t twoThirdsCount = (2 * indexCount) / 3;
    out++->set(pts[indices[firstIndex]], pts[indices[firstIndex + oneThirdCount]],
               pts[indices[firstIndex + twoThirdsCount]], devToAtlasOffset);

    out = emit_recursive_fan(pts, indices, firstIndex, oneThirdCount + 1, devToAtlasOffset, out);
    out = emit_recursive_fan(pts, indices, firstIndex + oneThirdCount,
                             twoThirdsCount - oneThirdCount + 1, devToAtlasOffset, out);

    int endIndex = firstIndex + indexCount;
    int32_t oldValue = indices[endIndex];
    indices[endIndex] = indices[firstIndex];
    out = emit_recursive_fan(pts, indices, firstIndex + twoThirdsCount,
                             indexCount - twoThirdsCount + 1, devToAtlasOffset, out);
    indices[endIndex] = oldValue;

    return out;
}

static void emit_tessellated_fan(const GrTessellator::WindingVertex* vertices, int numVertices,
                                 const Sk2f& devToAtlasOffset,
                                 TriPointInstance* triPointInstanceData,
                                 QuadPointInstance* quadPointInstanceData,
                                 GrCCGeometry::PrimitiveTallies* indices) {
    for (int i = 0; i < numVertices; i += 3) {
        if (1 == abs(vertices[i].fWinding)) {
            triPointInstanceData[indices->fTriangles++].set(vertices[i].fPos, vertices[i + 1].fPos,
                                                            vertices[i + 2].fPos, devToAtlasOffset);
        } else {
            quadPointInstanceData[indices->fWeightedTriangles++].setW(
                    vertices[i].fPos, vertices[i+1].fPos, vertices[i + 2].fPos, devToAtlasOffset,
                    static_cast<float>(abs(vertices[i].fWinding)));
        }
    }
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
            GR_CT_DIV_ROUND_UP(triEndIdx * sizeof(TriPointInstance), sizeof(QuadPointInstance));
    fBaseInstances[0].fWeightedTriangles = quadBaseIdx;
    fBaseInstances[1].fWeightedTriangles = fBaseInstances[0].fWeightedTriangles +
                                        fTotalPrimitiveCounts[0].fWeightedTriangles;
    fBaseInstances[0].fCubics = fBaseInstances[1].fWeightedTriangles +
                                fTotalPrimitiveCounts[1].fWeightedTriangles;
    fBaseInstances[1].fCubics = fBaseInstances[0].fCubics + fTotalPrimitiveCounts[0].fCubics;
    fBaseInstances[0].fConics = fBaseInstances[1].fCubics + fTotalPrimitiveCounts[1].fCubics;
    fBaseInstances[1].fConics = fBaseInstances[0].fConics + fTotalPrimitiveCounts[0].fConics;
    int quadEndIdx = fBaseInstances[1].fConics + fTotalPrimitiveCounts[1].fConics;

    fInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType,
                                            quadEndIdx * sizeof(QuadPointInstance));
    if (!fInstanceBuffer) {
        return false;
    }

    TriPointInstance* triPointInstanceData = static_cast<TriPointInstance*>(fInstanceBuffer->map());
    QuadPointInstance* quadPointInstanceData =
            reinterpret_cast<QuadPointInstance*>(triPointInstanceData);
    SkASSERT(quadPointInstanceData);

    PathInfo* nextPathInfo = fPathsInfo.begin();
    Sk2f devToAtlasOffset;
    PrimitiveTallies instanceIndices[2] = {fBaseInstances[0], fBaseInstances[1]};
    PrimitiveTallies* currIndices = nullptr;
    SkSTArray<256, int32_t, true> currFan;
    bool currFanIsTessellated = false;

    const SkTArray<SkPoint, true>& pts = fGeometry.points();
    int ptsIdx = -1;
    int nextConicWeightIdx = 0;

    // Expand the ccpr verbs into GPU instance buffers.
    for (GrCCGeometry::Verb verb : fGeometry.verbs()) {
        switch (verb) {
            case GrCCGeometry::Verb::kBeginPath:
                SkASSERT(currFan.empty());
                currIndices = &instanceIndices[(int)nextPathInfo->scissorMode()];
                devToAtlasOffset = Sk2f(static_cast<float>(nextPathInfo->devToAtlasOffset().fX),
                                        static_cast<float>(nextPathInfo->devToAtlasOffset().fY));
                currFanIsTessellated = nextPathInfo->hasFanTessellation();
                if (currFanIsTessellated) {
                    emit_tessellated_fan(nextPathInfo->fanTessellation(),
                                         nextPathInfo->fanTessellationCount(), devToAtlasOffset,
                                         triPointInstanceData, quadPointInstanceData, currIndices);
                }
                ++nextPathInfo;
                continue;

            case GrCCGeometry::Verb::kBeginContour:
                SkASSERT(currFan.empty());
                ++ptsIdx;
                if (!currFanIsTessellated) {
                    currFan.push_back(ptsIdx);
                }
                continue;

            case GrCCGeometry::Verb::kLineTo:
                ++ptsIdx;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case GrCCGeometry::Verb::kMonotonicQuadraticTo:
                triPointInstanceData[currIndices->fQuadratics++].set(&pts[ptsIdx],
                                                                     devToAtlasOffset);
                ptsIdx += 2;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case GrCCGeometry::Verb::kMonotonicCubicTo:
                quadPointInstanceData[currIndices->fCubics++].set(&pts[ptsIdx], devToAtlasOffset[0],
                                                                  devToAtlasOffset[1]);
                ptsIdx += 3;
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.push_back(ptsIdx);
                }
                continue;

            case GrCCGeometry::Verb::kMonotonicConicTo:
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

            case GrCCGeometry::Verb::kEndClosedContour:  // endPt == startPt.
                if (!currFanIsTessellated) {
                    SkASSERT(!currFan.empty());
                    currFan.pop_back();
                }
            // fallthru.
            case GrCCGeometry::Verb::kEndOpenContour:  // endPt != startPt.
                SkASSERT(!currFanIsTessellated || currFan.empty());
                if (!currFanIsTessellated && currFan.count() >= 3) {
                    int fanSize = currFan.count();
                    // Reserve space for emit_recursive_fan. Technically this can grow to
                    // fanSize + log3(fanSize), but we approximate with log2.
                    currFan.push_back_n(SkNextLog2(fanSize));
                    SkDEBUGCODE(TriPointInstance* end =)
                            emit_recursive_fan(pts, currFan, 0, fanSize, devToAtlasOffset,
                                               triPointInstanceData + currIndices->fTriangles);
                    currIndices->fTriangles += fanSize - 2;
                    SkASSERT(triPointInstanceData + currIndices->fTriangles == end);
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

void GrCCPathParser::drawCoverageCount(GrOpFlushState* flushState, CoverageCountBatchID batchID,
                                       const SkIRect& drawBounds) const {
    using PrimitiveType = GrCCCoverageProcessor::PrimitiveType;

    SkASSERT(fInstanceBuffer);

    const PrimitiveTallies& batchTotalCounts = fCoverageCountBatches[batchID].fTotalPrimitiveCounts;

    GrPipeline pipeline(flushState->drawOpArgs().fProxy, GrPipeline::ScissorState::kEnabled,
                        SkBlendMode::kPlus);

    if (batchTotalCounts.fTriangles) {
        this->drawPrimitives(flushState, pipeline, batchID, PrimitiveType::kTriangles,
                             &PrimitiveTallies::fTriangles, drawBounds);
    }

    if (batchTotalCounts.fWeightedTriangles) {
        this->drawPrimitives(flushState, pipeline, batchID, PrimitiveType::kWeightedTriangles,
                             &PrimitiveTallies::fWeightedTriangles, drawBounds);
    }

    if (batchTotalCounts.fQuadratics) {
        this->drawPrimitives(flushState, pipeline, batchID, PrimitiveType::kQuadratics,
                             &PrimitiveTallies::fQuadratics, drawBounds);
    }

    if (batchTotalCounts.fCubics) {
        this->drawPrimitives(flushState, pipeline, batchID, PrimitiveType::kCubics,
                             &PrimitiveTallies::fCubics, drawBounds);
    }

    if (batchTotalCounts.fConics) {
        this->drawPrimitives(flushState, pipeline, batchID, PrimitiveType::kConics,
                             &PrimitiveTallies::fConics, drawBounds);
    }
}

void GrCCPathParser::drawPrimitives(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                    CoverageCountBatchID batchID,
                                    GrCCCoverageProcessor::PrimitiveType primitiveType,
                                    int PrimitiveTallies::*instanceType,
                                    const SkIRect& drawBounds) const {
    SkASSERT(pipeline.isScissorEnabled());

    // Don't call reset(), as that also resets the reserve count.
    fMeshesScratchBuffer.pop_back_n(fMeshesScratchBuffer.count());
    fScissorRectScratchBuffer.pop_back_n(fScissorRectScratchBuffer.count());

    GrCCCoverageProcessor proc(flushState->resourceProvider(), primitiveType);

    SkASSERT(batchID > 0);
    SkASSERT(batchID < fCoverageCountBatches.count());
    const CoverageCountBatch& previousBatch = fCoverageCountBatches[batchID - 1];
    const CoverageCountBatch& batch = fCoverageCountBatches[batchID];
    SkDEBUGCODE(int totalInstanceCount = 0);

    if (int instanceCount = batch.fEndNonScissorIndices.*instanceType -
                            previousBatch.fEndNonScissorIndices.*instanceType) {
        SkASSERT(instanceCount > 0);
        int baseInstance = fBaseInstances[(int)ScissorMode::kNonScissored].*instanceType +
                           previousBatch.fEndNonScissorIndices.*instanceType;
        proc.appendMesh(fInstanceBuffer.get(), instanceCount, baseInstance, &fMeshesScratchBuffer);
        fScissorRectScratchBuffer.push_back().setXYWH(0, 0, drawBounds.width(),
                                                      drawBounds.height());
        SkDEBUGCODE(totalInstanceCount += instanceCount);
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
