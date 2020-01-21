/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellatePathOp.h"

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/tessellate/GrCoverShader.h"
#include "src/gpu/tessellate/GrPathParser.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"

GrTessellatePathOp::FixedFunctionFlags GrTessellatePathOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrTessellatePathOp::onPrepare(GrOpFlushState* state) {
    // First see if we should split up inner polygon triangles and curves, and triangulate the inner
    // polygon(s) more efficiently. This causes greater CPU overhead due to the extra shaders and
    // draw calls, but the better triangulation can reduce the rasterizer load by a great deal on
    // complex paths.
    const SkRect& bounds = fPath.getBounds();
    float scale = fViewMatrix.getMaxScale();
    // Raster-edge work is 1-dimensional, so we sum height and width rather than multiplying them.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scale * fPath.countVerbs();
    if (rasterEdgeWork > 1000 * 1000) {
        int numCurves = 0;
        int maxVertexCount = GrPathParser::MaxInnerPolygonVertices(fPath);
        if (auto* triangleData = (SkPoint*)state->makeVertexSpace(
                sizeof(SkPoint), maxVertexCount, &fPathVertexBuffer, &fBasePathVertex)) {
            if ((fPathVertexCount =
                     GrPathParser::EmitInnerPolygonTriangles(fPath, triangleData, &numCurves))) {
                fPathShader = state->allocator()->make<GrStencilTriangleShader>(fViewMatrix);
            } else {
                fPathVertexBuffer.reset();
            }
            state->putBackVertices(maxVertexCount - fPathVertexCount, sizeof(SkPoint));
        }
        if (numCurves) {
            if (auto* cubicData = (SkPoint*)state->makeVertexSpace(
                    sizeof(SkPoint) * 4, numCurves, &fCubicInstanceBuffer, &fBaseCubicInstance)) {
                fCubicInstanceCount = GrPathParser::EmitCubicInstances(fPath, cubicData);
                SkASSERT(fCubicInstanceCount == numCurves);
            }
        }
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    int maxVertexCount = GrPathParser::MaxWedgeVertices(fPath);
    if (auto* patchData = (SkPoint*)state->makeVertexSpace(
            sizeof(SkPoint), maxVertexCount, &fPathVertexBuffer, &fBasePathVertex)) {
        if ((fPathVertexCount = GrPathParser::EmitCenterWedgePatches(fPath, patchData))) {
            fPathShader = state->allocator()->make<GrStencilWedgeShader>(fViewMatrix);
        } else {
            fPathVertexBuffer.reset();
        }
        state->putBackVertices(maxVertexCount - fPathVertexCount, sizeof(SkPoint));
    }
}

void GrTessellatePathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    GrAppliedClip clip = state->detachAppliedClip();
    GrPipeline::FixedDynamicState fixedDynamicState;
    if (clip.scissorState().enabled()) {
        fixedDynamicState.fScissorRect = clip.scissorState().rect();
    }

    this->drawStencilPass(state, clip.hardClip(), &fixedDynamicState);

    if (!(Flags::kStencilOnly & fFlags)) {
        this->drawCoverPass(state, std::move(clip), &fixedDynamicState);
    }
}

void GrTessellatePathOp::drawStencilPass(GrOpFlushState* state, const GrAppliedHardClip& hardClip,
                                         const GrPipeline::FixedDynamicState* fixedDynamicState) {
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

    GrPipeline pipeline(initArgs, GrDisableColorXPFactory::MakeXferProcessor(), hardClip);

    if (fPathVertexBuffer) {
        GrProgramInfo programInfo(state->proxy()->numSamples(), state->proxy()->numStencilSamples(),
                                  state->proxy()->backendFormat(), state->view()->origin(),
                                  &pipeline, fPathShader, fixedDynamicState, nullptr, 0,
                                  fPathShader->primitiveType(),
                                  fPathShader->tessellationPatchVertexCount());

        GrMesh mesh(fPathShader->primitiveType(), fPathShader->tessellationPatchVertexCount());
        mesh.setNonIndexedNonInstanced(fPathVertexCount);
        mesh.setVertexData(fPathVertexBuffer, fBasePathVertex);

        state->opsRenderPass()->draw(programInfo, &mesh, 1, this->bounds());
    }

    if (fCubicInstanceBuffer) {
        // Here we treat the cubic instance buffer as tessellation patches.
        GrStencilCubicShader shader(fViewMatrix);
        GrProgramInfo programInfo(state->proxy()->numSamples(), state->proxy()->numStencilSamples(),
                                  state->proxy()->backendFormat(), state->view()->origin(),
                                  &pipeline, &shader, fixedDynamicState, nullptr, 0,
                                  GrPrimitiveType::kPatches, 4);

        GrMesh mesh(GrPrimitiveType::kPatches, 4);
        mesh.setNonIndexedNonInstanced(fCubicInstanceCount * 4);
        mesh.setVertexData(fCubicInstanceBuffer, fBaseCubicInstance * 4);

        state->opsRenderPass()->draw(programInfo, &mesh, 1, this->bounds());
    }

    // http://skbug.com/9739
    if (state->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
        state->gpu()->insertManualFramebufferBarrier();
    }
}

void GrTessellatePathOp::drawCoverPass(GrOpFlushState* state, GrAppliedClip&& clip,
                                       const GrPipeline::FixedDynamicState* fixedDynamicState) {
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
    initArgs.fUserStencil = &kTestAndResetStencil;
    initArgs.fCaps = &state->caps();
    initArgs.fDstProxyView = state->drawOpArgs().dstProxyView();
    initArgs.fOutputSwizzle = state->drawOpArgs().outputSwizzle();

    GrPipeline pipeline(initArgs, std::move(fProcessors), std::move(clip));
    GrCoverShader shader(fViewMatrix, fPath.getBounds(), fColor);
    GrProgramInfo programInfo(state->proxy()->numSamples(), state->proxy()->numStencilSamples(),
                              state->proxy()->backendFormat(), state->view()->origin(), &pipeline,
                              &shader, fixedDynamicState, nullptr, 0,
                              GrPrimitiveType::kTriangleStrip);

    GrMesh mesh(GrPrimitiveType::kTriangleStrip);
    mesh.setNonIndexedNonInstanced(4);

    state->opsRenderPass()->draw(programInfo, &mesh, 1, this->bounds());
}
