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
#include "src/gpu/tessellate/GrStencilPathShader.h"

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

void GrTessellatePathOp::onPrepare(GrOpFlushState* state) {
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
        if (this->prepareNonOverlappingInnerTriangles(state, &numCountedCurves)) {
            // Prepare cubics on an instance boundary so we can use the buffer to fill local convex
            // hulls as well.
            this->prepareOuterCubics(state, numCountedCurves,
                                     CubicDataAlignment::kInstanceBoundary);
            return;
        }
    }

    // Next see if we can split up inner polygon triangles and curves, and triangulate the inner
    // polygon(s) more efficiently. This causes greater CPU overhead due to the extra shaders and
    // draw calls, but the better triangulation can reduce the rasterizer load by a great deal on
    // complex paths.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    if (rasterEdgeWork > 1000 * 1000) {
        int numCountedCurves;
        this->prepareMiddleOutInnerTriangles(state, &numCountedCurves);
        // We will fill the path with a bounding box instead local cubic convex hulls, so there is
        // no need to prepare the cubics on an instance boundary.
        this->prepareOuterCubics(state, numCountedCurves, CubicDataAlignment::kVertexBoundary);
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    this->prepareCubicWedges(state);
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

void GrTessellatePathOp::prepareMiddleOutInnerTriangles(GrMeshDrawOp::Target* target,
                                                        int* numCountedCurves) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(!fDoStencilTriangleBuffer);
    SkASSERT(!fDoFillTriangleBuffer);

    // No initial moveTo, plus an implicit close at the end; n-2 triangles fill an n-gon.
    // Each triangle has 3 vertices.
    int maxVertices = (fPath.countVerbs() - 1) * 3;

    GrEagerDynamicVertexAllocator vertexAlloc(target, &fTriangleBuffer, &fBaseTriangleVertex);
    auto* vertexData = vertexAlloc.lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return;
    }

    GrMiddleOutPolygonTriangulator middleOut(vertexData, maxVertices);
    int localCurveCount = 0;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                middleOut.closeAndMove(*pts++);
                break;
            case SkPathVerb::kLine:
                middleOut.pushVertex(pts[1]);
                break;
            case SkPathVerb::kQuad:
                middleOut.pushVertex(pts[2]);
                ++localCurveCount;
                break;
            case SkPathVerb::kCubic:
                middleOut.pushVertex(pts[3]);
                ++localCurveCount;
                break;
            case SkPathVerb::kClose:
                middleOut.close();
                break;
            case SkPathVerb::kConic:
                SkUNREACHABLE;
        }
    }
    fTriangleVertexCount = middleOut.close();
    *numCountedCurves = localCurveCount;

    vertexAlloc.unlock(fTriangleVertexCount);

    if (fTriangleVertexCount) {
        fDoStencilTriangleBuffer = true;
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

void GrTessellatePathOp::prepareOuterCubics(GrMeshDrawOp::Target* target, int numCountedCurves,
                                            CubicDataAlignment alignment) {
    SkASSERT(!fCubicBuffer);
    SkASSERT(!fStencilCubicsShader);

    if (numCountedCurves == 0) {
        return;
    }

    bool instanceAligned = (alignment == CubicDataAlignment::kInstanceBoundary);
    int instanceOrVertexStride = (instanceAligned) ? sizeof(SkPoint) * 4 : sizeof(SkPoint);
    int instanceOrVertexCount = (instanceAligned) ? numCountedCurves : numCountedCurves * 4;
    int baseInstanceOrVertex;

    auto* vertexData = static_cast<SkPoint*>(target->makeVertexSpace(
            instanceOrVertexStride, instanceOrVertexCount, &fCubicBuffer, &baseInstanceOrVertex));
    if (!vertexData) {
        return;
    }
    fBaseCubicVertex = (instanceAligned) ? baseInstanceOrVertex * 4 : baseInstanceOrVertex;
    fCubicVertexCount = 0;

    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kQuad:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                quad2cubic(pts, vertexData + fCubicVertexCount);
                fCubicVertexCount += 4;
                break;
            case SkPathVerb::kCubic:
                SkASSERT(fCubicVertexCount < numCountedCurves * 4);
                memcpy(vertexData + fCubicVertexCount, pts, sizeof(SkPoint) * 4);
                fCubicVertexCount += 4;
                break;
            default:
                break;
        }
    }
    SkASSERT(fCubicVertexCount == numCountedCurves * 4);

    fStencilCubicsShader = target->allocator()->make<GrStencilCubicShader>(fViewMatrix);
}

void GrTessellatePathOp::prepareCubicWedges(GrMeshDrawOp::Target* target) {
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
        fStencilCubicsShader = target->allocator()->make<GrStencilWedgeShader>(fViewMatrix);
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
        GrPathShader::ProgramInfo programInfo(state->writeView(), &pipeline, fStencilCubicsShader);
        state->bindPipelineAndScissorClip(programInfo, this->bounds());
        state->bindBuffers(nullptr, nullptr, fCubicBuffer.get());
        state->draw(fCubicVertexCount, fBaseCubicVertex);
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
