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
    // First check if the path is large and/or simple enough that we can actually triangulate the
    // inner polygon(s) on the CPU. This is our fastest approach. It allows us to stencil only the
    // curves, and then fill the internal polygons directly to the final render target, thus filling
    // in the majority of pixels in a single render pass.
    SkScalar scales[2];
    SkAssertResult(fViewMatrix.getMinMaxScales(scales));  // Will fail if perspective.
    const SkRect& bounds = fPath.getBounds();
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }
    float gpuFragmentWork = bounds.height() * scales[0] * bounds.width() * scales[1];
    float cpuTessellationWork = (float)numVerbs * SkNextLog2(numVerbs);  // N log N.
    if (cpuTessellationWork * 500 + (256 * 256) < gpuFragmentWork) {  // Don't try below 256x256.
        int numCountedCurves;
        // This will fail if the inner triangles do not form a simple polygon (e.g., self
        // intersection, double winding).
        if (this->prepareNonOverlappingInnerTriangles(flushState, &numCountedCurves)) {
            std::swap(fColor.fR, fColor.fB);
            if (!numCountedCurves) {
                return;
            }
            if (flushState->caps().shaderCaps()->tessellationSupport()) {
                // Prepare cubics on an instance boundary so we can use the buffer to fill local
                // convex hulls as well.
                SkPoint* cubicData = this->makeCubicBuffer(flushState, numCountedCurves,
                                                           CubicDataAlignment::kInstanceBoundary);
                this->prepareOuterCubics(flushState, cubicData, nullptr);
                SkASSERT(fCubicVertexCount == numCountedCurves * 4);
                return;
            }
            GrResolveLevelCounter resolveLevelCounter(numCountedCurves);
            resolveLevelCounter.countAllBeziersInPath(kTessellationIntolerance, fPath, fViewMatrix);
            SkPoint* cubicData = this->makeCubicBuffer(
                    flushState, resolveLevelCounter.totalCurveInstanceCount(),
                    CubicDataAlignment::kInstanceBoundary);
            this->prepareOuterCubics(flushState, cubicData, &resolveLevelCounter);
            SkASSERT(fCubicVertexCount == resolveLevelCounter.totalCurveInstanceCount() * 4);
            return;
        }
    }

    // Next see if we can split up inner polygon triangles and curves, and triangulate the inner
    // polygon(s) more efficiently. This causes greater CPU overhead due to the extra shaders and
    // draw calls, but the better triangulation can reduce the rasterizer load by a great deal on
    // complex paths.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    bool drawTrianglesSeparately = (rasterEdgeWork > 1000 * 1000);
    if (drawTrianglesSeparately || !flushState->caps().shaderCaps()->tessellationSupport()) {
        if (flushState->caps().shaderCaps()->tessellationSupport()) {
            this->prepareMiddleOutStencilGeometry(flushState, nullptr, true);
        } else {
            GrResolveLevelCounter resolveLevelCounter(fPath.countVerbs());
            this->prepareMiddleOutStencilGeometry(flushState, &resolveLevelCounter,
                                                  drawTrianglesSeparately);
        }
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    this->prepareStencilWedges(flushState);
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

SkPoint* GrTessellatePathOp::makeCubicBuffer(GrMeshDrawOp::Target* target, int numCubics,
                                             CubicDataAlignment alignment) {
    if (numCubics <= 0) {
        return nullptr;
    }
    if (alignment == CubicDataAlignment::kInstanceBoundary) {
        int baseInstance;
        auto cubicData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, numCubics, &fCubicBuffer, &baseInstance));
        fBaseCubicVertex = baseInstance * 4;
        return cubicData;
    } else {
        return static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), numCubics * 4, &fCubicBuffer, &fBaseCubicVertex));
    }
}

void GrTessellatePathOp::prepareMiddleOutStencilGeometry(GrMeshDrawOp::Target* target,
                                                         GrResolveLevelCounter* resolveLevelCounter,
                                                         bool drawTrianglesSeparately) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(!fDoStencilTriangleBuffer);
    SkASSERT(!fDoFillTriangleBuffer);
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);
    SkASSERT(!fIndirectBuffer);

    // No initial moveTo, plus an implicit close at the end; n-2 triangles fill an n-gon.
    int maxInnerTriangles = fPath.countVerbs() - 1;
    int maxCubics = fPath.countVerbs();
    SkPoint* vertexData;
    int vertexAdvancePerTriangle;
    if (drawTrianglesSeparately) {
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint), maxInnerTriangles * 3, &fTriangleBuffer, &fBaseTriangleVertex));
        vertexAdvancePerTriangle = 3;
    } else {
        SkASSERT(resolveLevelCounter);
        int baseTriangleInstance;
        vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
                sizeof(SkPoint) * 4, maxInnerTriangles + maxCubics, &fCubicBuffer,
                &baseTriangleInstance));
        fBaseCubicVertex = baseTriangleInstance * 4;
        vertexAdvancePerTriangle = 4;
    }
    if (!vertexData) {
        return;
    }

    GrVectorXform xform(fViewMatrix);
    GrMiddleOutPolygonTriangulator middleOut(vertexData, vertexAdvancePerTriangle,
                                             fPath.countVerbs());
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
                    if (!resolveLevelCounter->countBezier(resolveLevel)) {
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
                    if (!resolveLevelCounter->countBezier(resolveLevel)) {
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
    SkASSERT(triangleCount < maxInnerTriangles);

    if (drawTrianglesSeparately) {
        target->putBackVertices(maxInnerTriangles - triangleCount, sizeof(SkPoint) * 3);
        fTriangleVertexCount = triangleCount * 3;
        if (fTriangleVertexCount) {
            fDoStencilTriangleBuffer = true;
        }
        if (numCountedCurves) {
            // We will fill the path with a bounding box instead local cubic convex hulls, so there
            // is no need to prepare the cubics on an instance boundary.
            auto alignment = (resolveLevelCounter) ? CubicDataAlignment::kInstanceBoundary
                                           : CubicDataAlignment::kVertexBoundary;
            SkPoint* cubicData = this->makeCubicBuffer(target, numCountedCurves, alignment);
            this->prepareOuterCubics(target, cubicData, resolveLevelCounter);
            SkASSERT(fCubicVertexCount == numCountedCurves * 4);
            std::swap(fColor.fR, fColor.fB);
        }
    } else {
        SkASSERT(resolveLevelCounter);
        resolveLevelCounter->setTriangleInstanceCount(triangleCount);
        int totalInstanceCount = triangleCount + resolveLevelCounter->totalCurveInstanceCount();
        if (totalInstanceCount) {
            this->prepareOuterCubics(target, vertexData, resolveLevelCounter);
            SkASSERT(fCubicVertexCount == (resolveLevelCounter->totalCurveInstanceCount()) * 4);
            std::swap(fColor.fG, fColor.fB);
        }
        target->putBackVertices(maxInnerTriangles + maxCubics - totalInstanceCount,
                                sizeof(SkPoint) * 4);
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

void GrTessellatePathOp::prepareOuterCubics(GrMeshDrawOp::Target* target, SkPoint* cubicData,
                                            const GrResolveLevelCounter* resolveLevelCounter) {
    SkASSERT(!fStencilCubicsShader);

    if (!cubicData) {
        SkASSERT(!fCubicBuffer);
        return;
    }
    fCubicVertexCount = 0;

    SkPoint* instanceLocations[kMaxResolveLevel + 1];
    SkDEBUGCODE(SkPoint* endLocations[kMaxResolveLevel + 1]);
    if (resolveLevelCounter) {
        SkASSERT(fBaseCubicVertex % 4 == 0);
        this->prepareIndirectDraws(target, *resolveLevelCounter, cubicData, instanceLocations);
#ifdef SK_DEBUG
        memcpy(endLocations, instanceLocations + 1, kMaxResolveLevel * sizeof(SkPoint*));
        int totalInstanceCount = resolveLevelCounter->triangleInstanceCount() +
                                 resolveLevelCounter->totalCurveInstanceCount();
        endLocations[kMaxResolveLevel] = cubicData + totalInstanceCount * 4;
#endif
    } else {
        instanceLocations[0] = cubicData;
    }

    if (!resolveLevelCounter || resolveLevelCounter->totalCurveInstanceCount() != 0) {
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
        SkASSERT(fIndirectCount > 0);
    } else {
        fStencilCubicsShader = target->allocator()->make<GrTessellateCubicShader>(fViewMatrix);
        SkASSERT(fCubicVertexCount > 0);
    }
}

void GrTessellatePathOp::prepareIndirectDraws(GrMeshDrawOp::Target* target,
                                              const GrResolveLevelCounter& resolveLevelCounter,
                                              SkPoint* instanceData,
                                              SkPoint* instanceLocations[]) {
    // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with the base
    // vertex on an instance boundary in order to accommodate this.
    SkASSERT(fBaseCubicVertex % 4 == 0);
    int baseInstance = fBaseCubicVertex >> 2;
    fIndirectCount = resolveLevelCounter.totalCurveDrawIndirectCount();
    if (resolveLevelCounter.triangleInstanceCount()) {
        ++fIndirectCount;
    }

    // Allocate space for the GrDrawIndexedIndirectCommand structs.
    GrDrawIndexedIndirectCommand* indirectData = target->makeDrawIndexedIndirectSpace(
            fIndirectCount, &fIndirectBuffer, &fIndirectOffset);
    if (!indirectData) {
        SkASSERT(!fIndirectBuffer);
        return;
    }

    // Fill out the drawIndirect structs and determine the starting instance data location at
    // each resolve level.
    int indirectIdx = 0;
    uint32_t runningInstanceCount = 0;
    if (int triangleInstanceCount = resolveLevelCounter.triangleInstanceCount()) {
        indirectData[0] = GrMiddleOutCubicShader::MakeDrawTrianglesCmd(triangleInstanceCount,
                                                                       baseInstance);
        indirectIdx = 1;
        runningInstanceCount = triangleInstanceCount;
    }
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        instanceLocations[resolveLevel] = instanceData + runningInstanceCount * 4;
        uint32_t instanceCountAtCurrLevel = resolveLevelCounter[resolveLevel];
        if (!instanceCountAtCurrLevel) {
            continue;
        }
        indirectData[indirectIdx++] = GrMiddleOutCubicShader::MakeDrawCubicsCmd(
                resolveLevel, instanceCountAtCurrLevel, baseInstance + runningInstanceCount);
        runningInstanceCount += instanceCountAtCurrLevel;
    }
    SkASSERT(indirectIdx == fIndirectCount);
    SkASSERT((int)runningInstanceCount == resolveLevelCounter.totalCurveInstanceCount() +
                                          resolveLevelCounter.triangleInstanceCount());
}

void GrTessellatePathOp::prepareStencilWedges(GrMeshDrawOp::Target* target) {
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
        fStencilCubicsShader = target->allocator()->make<GrTessellateWedgeShader>(fViewMatrix);
    }
}

void GrTessellatePathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    this->drawStencilPass(state);
    if (!(Flags::kStencilOnly & fFlags)) {
        this->drawCoverPass(state);
    }
}

void GrTessellatePathOp::drawStencilPass(GrOpFlushState* state) {
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
    if (state->caps().wireframeSupport() && (Flags::kWireframe & fFlags)) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    SkASSERT(SkPathFillType::kWinding == fPath.getFillType() ||
             SkPathFillType::kEvenOdd == fPath.getFillType());
    initArgs.fUserStencil = (SkPathFillType::kWinding == fPath.getFillType()) ?
            &kIncrDecrStencil : &kInvertStencil;
    initArgs.fCaps = &state->caps();
    GrPipeline pipeline(initArgs, GrDisableColorXPFactory::MakeXferProcessor(),
                        state->appliedHardClip());

    if (fDoStencilTriangleBuffer) {
        SkASSERT(fTriangleBuffer);
        GrStencilTriangleShader stencilTriangleShader(fViewMatrix);
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline,
                                              &stencilTriangleShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        state->bindBuffers(nullptr, nullptr, fTriangleBuffer.get());
        state->draw(fTriangleVertexCount, fBaseTriangleVertex);
    }

    if (fStencilCubicsShader) {
        SkASSERT(fCubicBuffer);
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, fStencilCubicsShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        if (fIndirectBuffer) {
            auto indexBuffer = GrMiddleOutCubicShader::FindOrMakeMiddleOutIndexBuffer(
                    state->resourceProvider());
            state->bindBuffers(indexBuffer.get(), fCubicBuffer.get(), nullptr);
            state->drawIndexedIndirect(fIndirectBuffer.get(), fIndirectOffset, fIndirectCount);
        } else {
            state->bindBuffers(nullptr, nullptr, fCubicBuffer.get());
            state->draw(fCubicVertexCount, fBaseCubicVertex);
        }
    }

    // http://skbug.com/9739
    if (state->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
        state->gpu()->insertManualFramebufferBarrier();
    }
}

void GrTessellatePathOp::drawCoverPass(GrOpFlushState* state) {
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
        if (1 == state->proxy()->numSamples()) {
            SkASSERT(GrAAType::kCoverage == fAAType);
            // We are mixed sampled. Use conservative raster to make the sample coverage mask 100%
            // at every fragment. This way we will still get a double hit on shared edges, but
            // whichever side comes first will cover every sample and will clear the stencil. The
            // other side will then be discarded and not cause a double blend.
            initArgs.fInputFlags |= GrPipeline::InputFlags::kConservativeRaster;
        }
    }
    initArgs.fCaps = &state->caps();
    initArgs.fDstProxyView = state->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = state->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), state->detachAppliedClip());

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
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, &fillTriangleShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        state->bindTextures(fillTriangleShader, nullptr, pipeline);
        state->bindBuffers(nullptr, nullptr, fTriangleBuffer.get());
        state->draw(fTriangleVertexCount, fBaseTriangleVertex);

        if (fStencilCubicsShader) {
            SkASSERT(fCubicBuffer);

            // At this point, every pixel is filled in except the ones touched by curves. Issue a
            // final cover pass over the curves by drawing their convex hulls. This will fill in any
            // remaining samples and reset the stencil buffer.
            pipeline.setUserStencil(&kTestAndResetStencil);
            GrFillCubicHullShader fillCubicHullShader(fViewMatrix, fColor);
            GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline,
                                                  &fillCubicHullShader);
            state->bindPipelineAndScissorClip(programInfo, this->bounds());
            state->bindTextures(fillCubicHullShader, nullptr, pipeline);

            // Here we treat fCubicBuffer as an instance buffer. It should have been prepared with
            // the base vertex on an instance boundary in order to accommodate this.
            SkASSERT((fCubicVertexCount % 4) == 0);
            SkASSERT((fBaseCubicVertex % 4) == 0);
            state->bindBuffers(nullptr, fCubicBuffer.get(), nullptr);
            state->drawInstanced(fCubicVertexCount >> 2, fBaseCubicVertex >> 2, 4, 0);
        }
        return;
    }

    // There are no triangles to fill. Just draw a bounding box.
    pipeline.setUserStencil(&kTestAndResetStencil);
    GrFillBoundingBoxShader fillBoundingBoxShader(fViewMatrix, fColor, fPath.getBounds());
    GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, &fillBoundingBoxShader);
    state->bindPipelineAndScissorClip(programInfo, this->bounds());
    state->bindTextures(fillBoundingBoxShader, nullptr, pipeline);
    state->bindBuffers(nullptr, nullptr, nullptr);
    state->draw(4, 0);
}
