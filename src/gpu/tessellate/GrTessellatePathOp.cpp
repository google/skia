/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellatePathOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrMidpointContourParser.h"
#include "src/gpu/tessellate/GrResolveLevelCounter.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"

constexpr static int kMaxResolveLevel = GrMiddleOutCubicShader::kMaxResolveLevel;
constexpr static float kTessellationIntolerance = 4;  // 1/4 of a pixel.

GrTessellatePathOp::FixedFunctionFlags GrTessellatePathOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrTessellatePathOp::onPrePrepare(GrRecordingContext*,
                                      const GrSurfaceProxyView* writeView,
                                      GrAppliedClip*,
                                      const GrXferProcessor::DstProxyView&) {
}

void GrTessellatePathOp::onPrepare(GrOpFlushState* flushState) {
    // If we won't use hardware tessellation, then we need a resolveLevel counter to decide how many
    // cubics we will draw at each resolveLevel, and how many resolve levels there will be that have
    // at least one cubic.
    GrResolveLevelCounter resolveLevelCounter;

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
            if (numCountedCubics) {
                // Always use indirect draws for cubics instead of tessellation here. Our goal in
                // this mode is to maximize GPU performance, and the middle-out topology used by our
                // indirect draws is easier on the rasterizer than a tessellated fan. There also
                // seems to be a small amount of fixed tessellation overhead that this avoids.
                //
                // NOTE: This will count fewer cubics than above if it discards any whose
                // resolveLevel=0.
                numCountedCubics = resolveLevelCounter.reset(fPath, fViewMatrix,
                                                             kTessellationIntolerance);
                this->prepareOuterCubics(flushState, numCountedCubics,
                                         CubicDataAlignment::kInstanceBoundary,
                                         &resolveLevelCounter);
            }
            return;
        }
    }

    if (flushState->caps().shaderCaps()->tessellationSupport()) {
        // When we have hardware tessellation, see if we can save some CPU cycles by rendering
        // wedges. But wedges fan out from the center, so only use them if it won't be too much work
        // for the raseterizer.
        // NOTE: Raster-edge work is 1-dimensional, so sum height and width instead of multiplying.
        float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * numVerbs;
        if (rasterEdgeWork < 300 * 300) {
            this->prepareStencilWedges(flushState);
            return;
        }
    }

    // There seems to be a fixed amount of overhead when using hardware tessellation. Never use it
    // if there are only a few verbs.
    bool drawTrianglesSeparately = numVerbs > 256;
    if (!drawTrianglesSeparately || !flushState->caps().shaderCaps()->tessellationSupport()) {
        // Draw outer cubics with indirect draws.
        this->prepareMiddleOutTrianglesAndCubics(flushState, &resolveLevelCounter,
                                                 drawTrianglesSeparately);
    } else {
        // Draw outer cubics with indirect hardware tessellation.
        this->prepareMiddleOutTrianglesAndCubics(flushState, nullptr, true);
    }
}

bool GrTessellatePathOp::prepareNonOverlappingInnerTriangles(GrMeshDrawOp::Target* target,
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
    if (((Flags::kStencilOnly | Flags::kWireframe) & fFlags) || GrAAType::kCoverage == fAAType ||
        (target->appliedClip() && target->appliedClip()->hasStencilClip())) {
        // If we have certain flags, mixed samples, or a stencil clip then we unfortunately
        // can't fill the inner polygon directly. Indicate that these triangles need to be
        // stencilled.
        fDoStencilTriangleBuffer = true;
    }
    if (!(Flags::kStencilOnly & fFlags)) {
        fDoFillTriangleBuffer = true;
    }
    return true;
}

void GrTessellatePathOp::prepareMiddleOutTrianglesAndCubics(
        GrMeshDrawOp::Target* target, GrResolveLevelCounter* resolveLevelCounter,
        bool drawTrianglesSeparately) {
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
    if (drawTrianglesSeparately) {
        vertexAdvancePerTriangle = 3;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), maxInnerTriangles * 3, &fTriangleBuffer, &fBaseTriangleVertex));
    } else {
        vertexAdvancePerTriangle = 4;
        SkASSERT(resolveLevelCounter);
        int baseTriangleInstance;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, maxInnerTriangles + maxCubics, &fCubicBuffer,
                &baseTriangleInstance));
        fBaseCubicVertex = baseTriangleInstance * 4;
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
                continue;
            case SkPathVerb::kLine:
                middleOut.pushVertex(pts[1]);
                continue;
            case SkPathVerb::kQuad:
                middleOut.pushVertex(pts[2]);
                if (resolveLevelCounter) {
                    int resolveLevel = GrWangsFormula::quadratic_log2(kTessellationIntolerance,
                                                                      pts, xform);
                    // Quadratics get converted to cubics before rendering.
                    if (!resolveLevelCounter->countCubic(resolveLevel)) {
                        continue;
                    }
                }
                ++numCountedCurves;
                continue;
            case SkPathVerb::kCubic:
                middleOut.pushVertex(pts[3]);
                if (resolveLevelCounter) {
                    int resolveLevel = GrWangsFormula::cubic_log2(kTessellationIntolerance, pts,
                                                                  xform);
                    if (!resolveLevelCounter->countCubic(resolveLevel)) {
                        continue;
                    }
                }
                ++numCountedCurves;
                continue;
            case SkPathVerb::kClose:
                middleOut.close();
                continue;
            case SkPathVerb::kConic:
                SkUNREACHABLE;
        }
    }
    int triangleCount = middleOut.close();
    SkASSERT(triangleCount <= maxInnerTriangles);

    if (drawTrianglesSeparately) {
        SkASSERT(vertexAdvancePerTriangle == 3);
        target->putBackVertices(maxInnerTriangles - triangleCount, sizeof(SkPoint) * 3);
        fTriangleVertexCount = triangleCount * 3;
        if (fTriangleVertexCount) {
            fDoStencilTriangleBuffer = true;
        }
        if (numCountedCurves) {
            // We will fill the path with a bounding box instead local cubic convex hulls, so we
            // can prepare the cubics on a vertex boundary if using hardware tessellation.
            auto alignment = (resolveLevelCounter) ? CubicDataAlignment::kInstanceBoundary
                                                   : CubicDataAlignment::kVertexBoundary;
            this->prepareOuterCubics(target, numCountedCurves, alignment, resolveLevelCounter);
        }
    } else {
        SkASSERT(resolveLevelCounter);
        int totalInstanceCount = triangleCount + resolveLevelCounter->totalCubicInstanceCount();
        if (totalInstanceCount) {
            this->prepareOuterCubicsWithTriangles(target, vertexData, triangleCount,
                                                  resolveLevelCounter->totalCubicInstanceCount(),
                                                  resolveLevelCounter);
        }
        SkASSERT(vertexAdvancePerTriangle == 4);
        target->putBackVertices(maxInnerTriangles + maxCubics - totalInstanceCount,
                                sizeof(SkPoint) * 4);
    }
}

void GrTessellatePathOp::prepareOuterCubics(GrMeshDrawOp::Target* target, int numCubics,
                                            CubicDataAlignment alignment,
                                            const GrResolveLevelCounter* resolveLevelCounter) {
    SkASSERT(!resolveLevelCounter || resolveLevelCounter->totalCubicInstanceCount() == numCubics);
    if (numCubics <= 0) {
        return;
    }

    // Allocate a buffer to store the cubic data.
    SkPoint* cubicData;
    if (alignment == CubicDataAlignment::kInstanceBoundary) {
        int baseInstance;
        cubicData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, numCubics, &fCubicBuffer, &baseInstance));
        fBaseCubicVertex = baseInstance * 4;
    } else {
        cubicData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), numCubics * 4, &fCubicBuffer, &fBaseCubicVertex));
    }
    if (!cubicData) {
        return;
    }

    this->prepareOuterCubicsWithTriangles(target, cubicData, /*numTrianglesAtBeginningOfData=*/0,
                                          numCubics, resolveLevelCounter);
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

void GrTessellatePathOp::prepareOuterCubicsWithTriangles(
        GrMeshDrawOp::Target* target, SkPoint* cubicData, int numTrianglesAtBeginningOfData,
        int numCubics, const GrResolveLevelCounter* resolveLevelCounter) {
    SkASSERT(!fStencilCubicsShader);
    SkASSERT(cubicData);

    SkPoint* instanceLocations[kMaxResolveLevel + 1];
    SkDEBUGCODE(SkPoint* endLocations[kMaxResolveLevel + 1]);
    if (resolveLevelCounter) {
        // We will be using indirect draws. Prepare the indirect draw data and determine where to
        // begin each resolve level in the instance buffer.
        SkASSERT(fBaseCubicVertex % 4 == 0);
        this->prepareIndirectDraws(target, *resolveLevelCounter, cubicData,
                                   numTrianglesAtBeginningOfData, instanceLocations);
#ifdef SK_DEBUG
        memcpy(endLocations, instanceLocations + 1, kMaxResolveLevel * sizeof(SkPoint*));
        int totalInstanceCount = numTrianglesAtBeginningOfData +
                                 resolveLevelCounter->totalCubicInstanceCount();
        endLocations[kMaxResolveLevel] = cubicData + totalInstanceCount * 4;
#endif
        fCubicVertexCount = numTrianglesAtBeginningOfData * 4;
    } else {
        SkASSERT(target->caps().shaderCaps()->tessellationSupport());
        SkASSERT(!numTrianglesAtBeginningOfData);
        // We will be using hardware tessellation. Just use instanceLocations[0] for everything and
        // emit the cubics in order.
        instanceLocations[0] = cubicData;
        fCubicVertexCount = 0;
    }

    if (numCubics) {
        GrVectorXform xform(fViewMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
            int level = 0;
            switch (verb) {
                default:
                    continue;
                case SkPathVerb::kQuad:
                    if (resolveLevelCounter) {
                        level = GrWangsFormula::quadratic_log2(kTessellationIntolerance, pts,
                                                               xform);
                        if (level == 0) {
                            continue;
                        }
                        level = std::min(level, kMaxResolveLevel);
                    }
                    quad2cubic(pts, instanceLocations[level]);
                    break;
                case SkPathVerb::kCubic:
                    if (resolveLevelCounter) {
                        level = GrWangsFormula::cubic_log2(kTessellationIntolerance, pts, xform);
                        if (level == 0) {
                            continue;
                        }
                        level = std::min(level, kMaxResolveLevel);
                    }
                    memcpy(instanceLocations[level], pts, sizeof(SkPoint) * 4);
                    break;
            }
            instanceLocations[level] += 4;
            fCubicVertexCount += 4;
        }
    }

#ifdef SK_DEBUG
    if (resolveLevelCounter) {
        for (int i = 1; i <= kMaxResolveLevel; ++i) {
            SkASSERT(instanceLocations[i] == endLocations[i]);
        }
    }
#endif

    if (resolveLevelCounter) {
        fStencilCubicsShader = target->allocator()->make<GrMiddleOutCubicShader>(fViewMatrix);
        SkASSERT(fIndirectDrawCount > 0);
    } else {
        fStencilCubicsShader = target->allocator()->make<GrTessellateCubicShader>(fViewMatrix);
    }

    SkASSERT(fCubicVertexCount == (numTrianglesAtBeginningOfData + numCubics) * 4);
}

void GrTessellatePathOp::prepareIndirectDraws(GrMeshDrawOp::Target* target,
                                              const GrResolveLevelCounter& resolveLevelCounter,
                                              SkPoint* instanceData,
                                              int numTrianglesAtBeginningOfData,
                                              SkPoint* instanceLocations[]) {
    // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with the base
    // vertex on an instance boundary in order to accommodate this.
    SkASSERT(fBaseCubicVertex % 4 == 0);
    int baseInstance = fBaseCubicVertex >> 2;
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
    int indirectIdx = 0;
    int runningInstanceCount = 0;
    if (numTrianglesAtBeginningOfData) {
        // The caller has already packed "triangleInstanceCount" triangles into 4-point instances
        // at the beginning of the instance buffer. Add a special-case indirect draw here that will
        // emit the triangles [P0, P1, P2] from these 4-point instances.
        indirectData[0] = GrMiddleOutCubicShader::MakeDrawTrianglesCmd(
                numTrianglesAtBeginningOfData, baseInstance);
        indirectIdx = 1;
        runningInstanceCount = numTrianglesAtBeginningOfData;
    }
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        instanceLocations[resolveLevel] = instanceData + runningInstanceCount * 4;
        if (int instanceCountAtCurrLevel = resolveLevelCounter[resolveLevel]) {
            indirectData[indirectIdx++] = GrMiddleOutCubicShader::MakeDrawCubicsCmd(
                    resolveLevel, instanceCountAtCurrLevel, baseInstance + runningInstanceCount);
            runningInstanceCount += instanceCountAtCurrLevel;
        }
    }
    SkASSERT(indirectIdx == fIndirectDrawCount);
    SkASSERT(runningInstanceCount == numTrianglesAtBeginningOfData +
                                     resolveLevelCounter.totalCubicInstanceCount());
}

void GrTessellatePathOp::prepareStencilWedges(GrMeshDrawOp::Target* target) {
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());

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
        fStencilCubicsShader = target->allocator()->make<GrTessellateWedgeShader>(fViewMatrix);
    }
}

void GrTessellatePathOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    this->drawStencilPass(flushState);
    if (!(Flags::kStencilOnly & fFlags)) {
        this->drawCoverPass(flushState);
    }
}

void GrTessellatePathOp::drawStencilPass(GrOpFlushState* flushState) {
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
    if (flushState->caps().wireframeSupport() && (Flags::kWireframe & fFlags)) {
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
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer.get());
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);
    }

    if (fStencilCubicsShader) {
        SkASSERT(fCubicBuffer);
        GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline,
                                              fStencilCubicsShader);
        flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
        if (fIndirectDrawBuffer) {
            auto indexBuffer = GrMiddleOutCubicShader::FindOrMakeMiddleOutIndexBuffer(
                    flushState->resourceProvider());
            flushState->bindBuffers(indexBuffer.get(), fCubicBuffer.get(), nullptr);
            flushState->drawIndexedIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset,
                                            fIndirectDrawCount);
        } else {
            flushState->bindBuffers(nullptr, nullptr, fCubicBuffer.get());
            flushState->draw(fCubicVertexCount, fBaseCubicVertex);
            // http://skbug.com/9739
            if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
                flushState->gpu()->insertManualFramebufferBarrier();
            }
        }
    }
}

void GrTessellatePathOp::drawCoverPass(GrOpFlushState* flushState) {
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
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        if (1 == flushState->proxy()->numSamples()) {
            SkASSERT(GrAAType::kCoverage == fAAType);
            // We are mixed sampled. Use conservative raster to make the sample coverage mask 100%
            // at every fragment. This way we will still get a double hit on shared edges, but
            // whichever side comes first will cover every sample and will clear the stencil. The
            // other side will then be discarded and not cause a double blend.
            initArgs.fInputFlags |= GrPipeline::InputFlags::kConservativeRaster;
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
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer.get());
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
            flushState->bindBuffers(nullptr, fCubicBuffer.get(), nullptr);
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
