/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathTessellateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrMidpointContourParser.h"
#include "src/gpu/tessellate/GrResolveLevelCounter.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

constexpr static float kLinearizationIntolerance =
        GrTessellationPathRenderer::kLinearizationIntolerance;

constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

using OpFlags = GrTessellationPathRenderer::OpFlags;

GrPathTessellateOp::FixedFunctionFlags GrPathTessellateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrPathTessellateOp::onPrePrepare(GrRecordingContext*,
                                      const GrSurfaceProxyView* writeView,
                                      GrAppliedClip*,
                                      const GrXferProcessor::DstProxyView&) {
}

void GrPathTessellateOp::onPrepare(GrOpFlushState* flushState) {
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }

    // First check if the path is large and/or simple enough that we can actually triangulate the
    // inner polygon(s) on the CPU. This is our fastest approach. It allows us to stencil only the
    // curves, and then fill the internal polygons directly to the final render target, thus drawing
    // the majority of pixels in a single render pass.
    SkScalar scales[2];
    SkAssertResult(fViewMatrix.getMinMaxScales(scales));  // Will fail if perspective.
    const SkRect& bounds = fPath.getBounds();
    float gpuFragmentWork = bounds.height() * scales[0] * bounds.width() * scales[1];
    float cpuTessellationWork = (float)numVerbs * SkNextLog2(numVerbs);  // N log N.
    if (cpuTessellationWork * 500 + (256 * 256) < gpuFragmentWork) {  // Don't try below 256x256.
        int numCountedCubics;
        // This will fail if the inner triangles do not form a simple polygon (e.g., self
        // intersection, double winding).
        if (this->prepareNonOverlappingInnerTriangles(flushState, &numCountedCubics)) {
            if (!numCountedCubics) {
                return;
            }
            // Always use indirect draws for cubics instead of tessellation here. Our goal in this
            // mode is to maximize GPU performance, and the middle-out topology used by our indirect
            // draws is easier on the rasterizer than a tessellated fan. There also seems to be a
            // small amount of fixed tessellation overhead that this avoids.
            GrResolveLevelCounter resolveLevelCounter;
            resolveLevelCounter.reset(fPath, fViewMatrix, kLinearizationIntolerance);
            this->prepareIndirectOuterCubics(flushState, resolveLevelCounter);
            return;
        }
    }

    // When there are only a few verbs, it seems to always be fastest to make a single indirect draw
    // that contains both the inner triangles and the outer cubics, instead of using hardware
    // tessellation. Also take this path if tessellation is not supported.
    bool drawTrianglesAsIndirectCubicDraw = (numVerbs < 50);
    if (drawTrianglesAsIndirectCubicDraw || (fOpFlags & OpFlags::kDisableHWTessellation)) {
        // Prepare outer cubics with indirect draws.
        GrResolveLevelCounter resolveLevelCounter;
        this->prepareMiddleOutTrianglesAndCubics(flushState, &resolveLevelCounter,
                                                 drawTrianglesAsIndirectCubicDraw);
        return;
    }

    // The caller should have sent Flags::kDisableHWTessellation if it was not supported.
    SkASSERT(flushState->caps().shaderCaps()->tessellationSupport());

    // Next see if we can split up the inner triangles and outer cubics into two draw calls. This
    // allows for a more efficient inner triangle topology that can reduce the rasterizer load by a
    // large margin on complex paths, but also causes greater CPU overhead due to the extra shader
    // switches and draw calls.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    if (rasterEdgeWork > 300 * 300) {
        this->prepareMiddleOutTrianglesAndCubics(flushState);
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    this->prepareTessellatedCubicWedges(flushState);
}

bool GrPathTessellateOp::prepareNonOverlappingInnerTriangles(GrMeshDrawOp::Target* target,
                                                             int* numCountedCurves) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(!fDoStencilTriangleBuffer);
    SkASSERT(!fDoFillTriangleBuffer);

    using GrTriangulator::Mode;

    GrEagerDynamicVertexAllocator vertexAlloc(target, &fTriangleBuffer, &fBaseTriangleVertex);
    fTriangleVertexCount = GrTriangulator::PathToTriangles(fPath, 0, SkRect::MakeEmpty(),
                                                           &vertexAlloc, Mode::kSimpleInnerPolygons,
                                                           numCountedCurves);
    if (fTriangleVertexCount == 0) {
        // Mode::kSimpleInnerPolygons causes PathToTriangles to fail if the inner polygon(s) are not
        // simple.
        return false;
    }
    if (((OpFlags::kStencilOnly | OpFlags::kWireframe) & fOpFlags) ||
        GrAAType::kCoverage == fAAType ||
        (target->appliedClip() && target->appliedClip()->hasStencilClip())) {
        // If we have certain flags, mixed samples, or a stencil clip then we unfortunately
        // can't fill the inner polygon directly. Indicate that these triangles need to be
        // stencilled.
        fDoStencilTriangleBuffer = true;
    }
    if (!(OpFlags::kStencilOnly & fOpFlags)) {
        fDoFillTriangleBuffer = true;
    }
    return true;
}

void GrPathTessellateOp::prepareMiddleOutTrianglesAndCubics(
        GrMeshDrawOp::Target* target, GrResolveLevelCounter* resolveLevelCounter,
        bool drawTrianglesAsIndirectCubicDraw) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(!fDoStencilTriangleBuffer);
    SkASSERT(!fDoFillTriangleBuffer);
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);
    SkASSERT(!fIndirectDrawBuffer);

    // No initial moveTo, plus an implicit close at the end; n-2 triangles fill an n-gon.
    int maxInnerTriangles = fPath.countVerbs() - 1;
    int maxCubics = fPath.countVerbs();

    SkPoint* vertexData;
    int vertexAdvancePerTriangle;
    if (drawTrianglesAsIndirectCubicDraw) {
        // Allocate the triangles as 4-point instances at the beginning of the cubic buffer.
        SkASSERT(resolveLevelCounter);
        vertexAdvancePerTriangle = 4;
        int baseTriangleInstance;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, maxInnerTriangles + maxCubics, &fCubicBuffer,
                &baseTriangleInstance));
        fBaseCubicVertex = baseTriangleInstance * 4;
    } else {
        // Allocate the triangles as normal 3-point instances in the triangle buffer.
        vertexAdvancePerTriangle = 3;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), maxInnerTriangles * 3, &fTriangleBuffer, &fBaseTriangleVertex));
    }
    if (!vertexData) {
        return;
    }

    GrVectorXform xform(fViewMatrix);
    GrMiddleOutPolygonTriangulator middleOut(vertexData, vertexAdvancePerTriangle,
                                             fPath.countVerbs());
    if (resolveLevelCounter) {
        resolveLevelCounter->reset();
    }
    int numCountedCurves = 0;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                middleOut.closeAndMove(pts[0]);
                break;
            case SkPathVerb::kLine:
                middleOut.pushVertex(pts[1]);
                break;
            case SkPathVerb::kQuad:
                middleOut.pushVertex(pts[2]);
                if (resolveLevelCounter) {
                    // Quadratics get converted to cubics before rendering.
                    resolveLevelCounter->countCubic(GrWangsFormula::quadratic_log2(
                            kLinearizationIntolerance, pts, xform));
                    break;
                }
                ++numCountedCurves;
                break;
            case SkPathVerb::kCubic:
                middleOut.pushVertex(pts[3]);
                if (resolveLevelCounter) {
                    resolveLevelCounter->countCubic(GrWangsFormula::cubic_log2(
                            kLinearizationIntolerance, pts, xform));
                    break;
                }
                ++numCountedCurves;
                break;
            case SkPathVerb::kClose:
                middleOut.close();
                break;
            case SkPathVerb::kConic:
                SkUNREACHABLE;
        }
    }
    int triangleCount = middleOut.close();
    SkASSERT(triangleCount <= maxInnerTriangles);

    if (drawTrianglesAsIndirectCubicDraw) {
        SkASSERT(resolveLevelCounter);
        int totalInstanceCount = triangleCount + resolveLevelCounter->totalCubicInstanceCount();
        SkASSERT(vertexAdvancePerTriangle == 4);
        target->putBackVertices(maxInnerTriangles + maxCubics - totalInstanceCount,
                                sizeof(SkPoint) * 4);
        if (totalInstanceCount) {
            this->prepareIndirectOuterCubicsAndTriangles(target, *resolveLevelCounter, vertexData,
                                                         triangleCount);
        }
    } else {
        SkASSERT(vertexAdvancePerTriangle == 3);
        target->putBackVertices(maxInnerTriangles - triangleCount, sizeof(SkPoint) * 3);
        fTriangleVertexCount = triangleCount * 3;
        if (fTriangleVertexCount) {
            fDoStencilTriangleBuffer = true;
        }
        if (resolveLevelCounter) {
            this->prepareIndirectOuterCubics(target, *resolveLevelCounter);
        } else {
            this->prepareTessellatedOuterCubics(target, numCountedCurves);
        }
    }
}

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static void line2cubic(const SkPoint& p0, const SkPoint& p1, SkPoint* out) {
    out[0] = p0;
    out[1] = lerp(p0, p1, 1/3.f);
    out[2] = lerp(p0, p1, 2/3.f);
    out[3] = p1;
}

static void quad2cubic(const SkPoint pts[], SkPoint* out) {
    out[0] = pts[0];
    out[1] = lerp(pts[0], pts[1], 2/3.f);
    out[2] = lerp(pts[1], pts[2], 1/3.f);
    out[3] = pts[2];
}

void GrPathTessellateOp::prepareIndirectOuterCubics(
        GrMeshDrawOp::Target* target, const GrResolveLevelCounter& resolveLevelCounter) {
    SkASSERT(resolveLevelCounter.totalCubicInstanceCount() >= 0);
    if (resolveLevelCounter.totalCubicInstanceCount() == 0) {
        return;
    }
    // Allocate a buffer to store the cubic data.
    SkPoint* cubicData;
    int baseInstance;
    cubicData = static_cast<SkPoint*>(target->makeVertexSpace(
            sizeof(SkPoint) * 4, resolveLevelCounter.totalCubicInstanceCount(), &fCubicBuffer,
            &baseInstance));
    if (!cubicData) {
        return;
    }
    fBaseCubicVertex = baseInstance * 4;
    this->prepareIndirectOuterCubicsAndTriangles(target, resolveLevelCounter, cubicData,
                                                 /*numTrianglesAtBeginningOfData=*/0);
}

void GrPathTessellateOp::prepareIndirectOuterCubicsAndTriangles(
        GrMeshDrawOp::Target* target, const GrResolveLevelCounter& resolveLevelCounter,
        SkPoint* cubicData, int numTrianglesAtBeginningOfData) {
    SkASSERT(target->caps().drawInstancedSupport());
    SkASSERT(numTrianglesAtBeginningOfData + resolveLevelCounter.totalCubicInstanceCount() > 0);
    SkASSERT(!fStencilCubicsShader);
    SkASSERT(cubicData);

    fIndirectIndexBuffer = GrMiddleOutCubicShader::FindOrMakeMiddleOutIndexBuffer(
            target->resourceProvider());
    if (!fIndirectIndexBuffer) {
        return;
    }

    // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with the base
    // vertex on an instance boundary in order to accommodate this.
    SkASSERT(fBaseCubicVertex % 4 == 0);
    int baseInstance = fBaseCubicVertex >> 2;

    // Start preparing the indirect draw buffer.
    fIndirectDrawCount = resolveLevelCounter.totalCubicIndirectDrawCount();
    if (numTrianglesAtBeginningOfData) {
        ++fIndirectDrawCount;  // Add an indirect draw for the triangles at the beginning.
    }

    // Allocate space for the GrDrawIndexedIndirectCommand structs.
    GrDrawIndexedIndirectCommand* indirectData = target->makeDrawIndexedIndirectSpace(
            fIndirectDrawCount, &fIndirectDrawBuffer, &fIndirectDrawOffset);
    if (!indirectData) {
        SkASSERT(!fIndirectDrawBuffer);
        return;
    }

    // Fill out the GrDrawIndexedIndirectCommand structs and determine the starting instance data
    // location at each resolve level.
    SkPoint* instanceLocations[kMaxResolveLevel + 1];
    int indirectIdx = 0;
    int runningInstanceCount = 0;
    if (numTrianglesAtBeginningOfData) {
        // The caller has already packed "triangleInstanceCount" triangles into 4-point instances
        // at the beginning of the instance buffer. Add a special-case indirect draw here that will
        // emit the triangles [P0, P1, P2] from these 4-point instances.
        indirectData[0] = GrMiddleOutCubicShader::MakeDrawTrianglesIndirectCmd(
                numTrianglesAtBeginningOfData, baseInstance);
        indirectIdx = 1;
        runningInstanceCount = numTrianglesAtBeginningOfData;
    }
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        int instanceCountAtCurrLevel = resolveLevelCounter[resolveLevel];
        if (!instanceCountAtCurrLevel) {
            SkDEBUGCODE(instanceLocations[resolveLevel] = nullptr;)
            continue;
        }
        instanceLocations[resolveLevel] = cubicData + runningInstanceCount * 4;
        indirectData[indirectIdx++] = GrMiddleOutCubicShader::MakeDrawCubicsIndirectCmd(
                resolveLevel, instanceCountAtCurrLevel, baseInstance + runningInstanceCount);
        runningInstanceCount += instanceCountAtCurrLevel;
    }

#ifdef SK_DEBUG
    SkASSERT(indirectIdx == fIndirectDrawCount);
    SkASSERT(runningInstanceCount == numTrianglesAtBeginningOfData +
                                     resolveLevelCounter.totalCubicInstanceCount());
    SkASSERT(fIndirectDrawCount > 0);

    SkPoint* endLocations[kMaxResolveLevel + 1];
    int lastResolveLevel = 0;
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        if (!instanceLocations[resolveLevel]) {
            endLocations[resolveLevel] = nullptr;
            continue;
        }
        endLocations[lastResolveLevel] = instanceLocations[resolveLevel];
        lastResolveLevel = resolveLevel;
    }
    int totalInstanceCount = numTrianglesAtBeginningOfData +
                             resolveLevelCounter.totalCubicInstanceCount();
    endLocations[lastResolveLevel] = cubicData + totalInstanceCount * 4;
#endif

    fCubicVertexCount = numTrianglesAtBeginningOfData * 4;

    if (resolveLevelCounter.totalCubicInstanceCount()) {
        GrVectorXform xform(fViewMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
            int level;
            switch (verb) {
                default:
                    continue;
                case SkPathVerb::kQuad:
                    level = GrWangsFormula::quadratic_log2(kLinearizationIntolerance, pts, xform);
                    if (level == 0) {
                        continue;
                    }
                    level = std::min(level, kMaxResolveLevel);
                    quad2cubic(pts, instanceLocations[level]);
                    break;
                case SkPathVerb::kCubic:
                    level = GrWangsFormula::cubic_log2(kLinearizationIntolerance, pts, xform);
                    if (level == 0) {
                        continue;
                    }
                    level = std::min(level, kMaxResolveLevel);
                    memcpy(instanceLocations[level], pts, sizeof(SkPoint) * 4);
                    break;
            }
            instanceLocations[level] += 4;
            fCubicVertexCount += 4;
        }
    }

#ifdef SK_DEBUG
    for (int i = 1; i <= kMaxResolveLevel; ++i) {
        SkASSERT(instanceLocations[i] == endLocations[i]);
    }
    SkASSERT(fCubicVertexCount == (numTrianglesAtBeginningOfData +
                                   resolveLevelCounter.totalCubicInstanceCount()) * 4);
#endif

    fStencilCubicsShader = target->allocator()->make<GrMiddleOutCubicShader>(fViewMatrix);
}

void GrPathTessellateOp::prepareTessellatedOuterCubics(GrMeshDrawOp::Target* target,
                                                       int numCountedCurves) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(numCountedCurves >= 0);
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);

    if (numCountedCurves == 0) {
        return;
    }

    auto* vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
            sizeof(SkPoint), numCountedCurves * 4, &fCubicBuffer, &fBaseCubicVertex));
    if (!vertexData) {
        return;
    }
    fCubicVertexCount = 0;

    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            default:
                continue;
            case SkPathVerb::kQuad:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                quad2cubic(pts, vertexData + fCubicVertexCount);
                break;
            case SkPathVerb::kCubic:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                memcpy(vertexData + fCubicVertexCount, pts, sizeof(SkPoint) * 4);
                break;
        }
        fCubicVertexCount += 4;
    }
    SkASSERT(fCubicVertexCount == numCountedCurves * 4);

    fStencilCubicsShader = target->allocator()->make<GrCubicTessellateShader>(fViewMatrix);
}

void GrPathTessellateOp::prepareTessellatedCubicWedges(GrMeshDrawOp::Target* target) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);

    // No initial moveTo, one wedge per verb, plus an implicit close at the end.
    // Each wedge has 5 vertices.
    int maxVertices = (fPath.countVerbs() + 1) * 5;

    GrEagerDynamicVertexAllocator vertexAlloc(target, &fCubicBuffer, &fBaseCubicVertex);
    auto* vertexData = vertexAlloc.lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return;
    }
    fCubicVertexCount = 0;

    GrMidpointContourParser parser(fPath);
    while (parser.parseNextContour()) {
        SkPoint midpoint = parser.currentMidpoint();
        SkPoint startPoint = {0, 0};
        SkPoint lastPoint = startPoint;
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove:
                    startPoint = lastPoint = pts[0];
                    continue;
                case SkPathVerb::kClose:
                    continue;  // Ignore. We can assume an implicit close at the end.
                case SkPathVerb::kLine:
                    line2cubic(pts[0], pts[1], vertexData + fCubicVertexCount);
                    lastPoint = pts[1];
                    break;
                case SkPathVerb::kQuad:
                    quad2cubic(pts, vertexData + fCubicVertexCount);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    memcpy(vertexData + fCubicVertexCount, pts, sizeof(SkPoint) * 4);
                    lastPoint = pts[3];
                    break;
                case SkPathVerb::kConic:
                    SkUNREACHABLE;
            }
            vertexData[fCubicVertexCount + 4] = midpoint;
            fCubicVertexCount += 5;
        }
        if (lastPoint != startPoint) {
            line2cubic(lastPoint, startPoint, vertexData + fCubicVertexCount);
            vertexData[fCubicVertexCount + 4] = midpoint;
            fCubicVertexCount += 5;
        }
    }

    vertexAlloc.unlock(fCubicVertexCount);

    if (fCubicVertexCount) {
        fStencilCubicsShader = target->allocator()->make<GrWedgeTessellateShader>(fViewMatrix);
    }
}

void GrPathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    this->drawStencilPass(flushState);
    if (!(OpFlags::kStencilOnly & fOpFlags)) {
        this->drawCoverPass(flushState);
    }
}

void GrPathTessellateOp::drawStencilPass(GrOpFlushState* flushState) {
    // Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
    constexpr static GrUserStencilSettings kIncrDecrStencil(
        GrUserStencilSettings::StaticInitSeparate<
            0x0000,                                0x0000,
            GrUserStencilTest::kAlwaysIfInClip,    GrUserStencilTest::kAlwaysIfInClip,
            0xffff,                                0xffff,
            GrUserStencilOp::kIncWrap,             GrUserStencilOp::kDecWrap,
            GrUserStencilOp::kKeep,                GrUserStencilOp::kKeep,
            0xffff,                                0xffff>());

    // Inverts the bottom stencil bit. Used for "even/odd" fill.
    constexpr static GrUserStencilSettings kInvertStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kAlwaysIfInClip,
            0xffff,
            GrUserStencilOp::kInvert,
            GrUserStencilOp::kKeep,
            0x0001>());

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    if (flushState->caps().wireframeSupport() && (OpFlags::kWireframe & fOpFlags)) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    SkASSERT(SkPathFillType::kWinding == fPath.getFillType() ||
             SkPathFillType::kEvenOdd == fPath.getFillType());
    initArgs.fUserStencil = (SkPathFillType::kWinding == fPath.getFillType()) ?
            &kIncrDecrStencil : &kInvertStencil;
    initArgs.fCaps = &flushState->caps();
    GrPipeline pipeline(initArgs, GrDisableColorXPFactory::MakeXferProcessor(),
                        flushState->appliedHardClip());

    if (fDoStencilTriangleBuffer) {
        SkASSERT(fTriangleBuffer);
        GrStencilTriangleShader stencilTriangleShader(fViewMatrix);
        GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                              &stencilTriangleShader);
        flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);
    }

    if (fStencilCubicsShader) {
        SkASSERT(fCubicBuffer);
        GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                              fStencilCubicsShader);
        flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
        if (fIndirectDrawBuffer) {
            SkASSERT(fIndirectIndexBuffer);
            flushState->bindBuffers(fIndirectIndexBuffer, fCubicBuffer, nullptr);
            flushState->drawIndexedIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset,
                                            fIndirectDrawCount);
        } else {
            flushState->bindBuffers(nullptr, nullptr, fCubicBuffer);
            flushState->draw(fCubicVertexCount, fBaseCubicVertex);
            if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
                flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
            }
        }
    }
}

void GrPathTessellateOp::drawCoverPass(GrOpFlushState* flushState) {
    // Allows non-zero stencil values to pass and write a color, and resets the stencil value back
    // to zero; discards immediately on stencil values of zero.
    // NOTE: It's ok to not check the clip here because the previous stencil pass only wrote to
    // samples already inside the clip.
    constexpr static GrUserStencilSettings kTestAndResetStencil(
        GrUserStencilSettings::StaticInit<
            0x0000,
            GrUserStencilTest::kNotEqual,
            0xffff,
            GrUserStencilOp::kZero,
            GrUserStencilOp::kKeep,
            0xffff>());

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        if (flushState->proxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the cover geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(GrAAType::kCoverage == fAAType);
            if (flushState->caps().conservativeRasterSupport()) {
                initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
                initArgs.fInputFlags |= GrPipeline::InputFlags::kConservativeRaster;
            }
        } else {
            // We are standard MSAA. Leave MSAA enabled for the cover geometry.
            initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
    }
    initArgs.fCaps = &flushState->caps();
    initArgs.fDstProxyView = flushState->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = flushState->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    if (fDoFillTriangleBuffer) {
        SkASSERT(fTriangleBuffer);

        // These are a twist on the standard red book stencil settings that allow us to fill the
        // inner polygon directly to the final render target. At this point, the curves are already
        // stencilled in. So if the stencil value is zero, then it means the path at our sample is
        // not affected by any curves and we fill the path in directly. If the stencil value is
        // nonzero, then we don't fill and instead continue the standard red book stencil process.
        //
        // NOTE: These settings are currently incompatible with a stencil clip.
        constexpr static GrUserStencilSettings kFillOrIncrDecrStencil(
            GrUserStencilSettings::StaticInitSeparate<
                0x0000,                        0x0000,
                GrUserStencilTest::kEqual,     GrUserStencilTest::kEqual,
                0xffff,                        0xffff,
                GrUserStencilOp::kKeep,        GrUserStencilOp::kKeep,
                GrUserStencilOp::kIncWrap,     GrUserStencilOp::kDecWrap,
                0xffff,                        0xffff>());

        constexpr static GrUserStencilSettings kFillOrInvertStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kZero,
                0xffff>());

        if (fDoStencilTriangleBuffer) {
            // The path was already stencilled. Here we just need to do a cover pass.
            pipeline.setUserStencil(&kTestAndResetStencil);
        } else if (!fStencilCubicsShader) {
            // There are no stencilled curves. We can ignore stencil and fill the path directly.
            pipeline.setUserStencil(&GrUserStencilSettings::kUnused);
        } else if (SkPathFillType::kWinding == fPath.getFillType()) {
            // Fill in the path pixels not touched by curves, incr/decr stencil otherwise.
            SkASSERT(!pipeline.hasStencilClip());
            pipeline.setUserStencil(&kFillOrIncrDecrStencil);
        } else {
            // Fill in the path pixels not touched by curves, invert stencil otherwise.
            SkASSERT(!pipeline.hasStencilClip());
            pipeline.setUserStencil(&kFillOrInvertStencil);
        }

        GrFillTriangleShader fillTriangleShader(fViewMatrix, fColor);
        GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                              &fillTriangleShader);
        flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
        flushState->bindTextures(fillTriangleShader, nullptr, pipeline);
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);

        if (fStencilCubicsShader) {
            SkASSERT(fCubicBuffer);

            // At this point, every pixel is filled in except the ones touched by curves. Issue a
            // final cover pass over the curves by drawing their convex hulls. This will fill in any
            // remaining samples and reset the stencil buffer.
            pipeline.setUserStencil(&kTestAndResetStencil);
            GrFillCubicHullShader fillCubicHullShader(fViewMatrix, fColor);
            GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                                  &fillCubicHullShader);
            flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
            flushState->bindTextures(fillCubicHullShader, nullptr, pipeline);

            // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with
            // the base vertex on an instance boundary in order to accommodate this.
            SkASSERT((fCubicVertexCount % 4) == 0);
            SkASSERT((fBaseCubicVertex % 4) == 0);
            flushState->bindBuffers(nullptr, fCubicBuffer, nullptr);
            flushState->drawInstanced(fCubicVertexCount >> 2, fBaseCubicVertex >> 2, 4, 0);
        }
        return;
    }

    // There are no triangles to fill. Just draw a bounding box.
    pipeline.setUserStencil(&kTestAndResetStencil);
    GrFillBoundingBoxShader fillBoundingBoxShader(fViewMatrix, fColor, fPath.getBounds());
    GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                          &fillBoundingBoxShader);
    flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
    flushState->bindTextures(fillBoundingBoxShader, nullptr, pipeline);
    flushState->bindBuffers(nullptr, nullptr, nullptr);
    flushState->draw(4, 0);
}
