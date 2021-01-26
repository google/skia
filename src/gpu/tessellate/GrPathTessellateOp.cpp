/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathTessellateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

using OpFlags = GrTessellationPathRenderer::OpFlags;

void GrPathTessellateOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fPipelineForFills) {
        fPipelineForFills->visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrPathTessellateOp::FixedFunctionFlags GrPathTessellateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

void GrPathTessellateOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrXferProcessor::DstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    SkArenaAlloc* recordTimeAllocator = context->priv().recordTimeAllocator();
    GrAppliedHardClip hardClip = GrAppliedHardClip(
            (clip) ? clip->hardClip() : GrAppliedHardClip::Disabled());
    PrePrepareArgs args{recordTimeAllocator, writeView, &hardClip, clip, &dstProxyView,
                        renderPassXferBarriers, colorLoadOp, context->priv().caps()};

    this->prePreparePrograms(args);

    if (fStencilTrianglesProgram) {
        context->priv().recordProgramInfo(fStencilTrianglesProgram);
    }
    if (fStencilCubicsProgram) {
        context->priv().recordProgramInfo(fStencilCubicsProgram);
    }
    if (fFillTrianglesProgram) {
        context->priv().recordProgramInfo(fFillTrianglesProgram);
    }
    if (fFillPathProgram) {
        context->priv().recordProgramInfo(fFillPathProgram);
    }
}

void GrPathTessellateOp::prePreparePrograms(const PrePrepareArgs& args) {
    using DrawInnerFan = GrPathIndirectTessellator::DrawInnerFan;
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
        bool isLinear;
        // This will fail if the inner triangles do not form a simple polygon (e.g., self
        // intersection, double winding).
        if (this->prePrepareInnerPolygonTriangulation(args, &isLinear)) {
            if (!isLinear) {
                // Always use indirect draws for cubics instead of tessellation here. Our goal in
                // this mode is to maximize GPU performance, and the middle-out topology used by our
                // indirect draws is easier on the rasterizer than a tessellated fan. There also
                // seems to be a small amount of fixed tessellation overhead that this avoids.
                this->prePrepareStencilCubicsProgram<GrMiddleOutCubicShader>(args);
                // We will need one final pass to cover the convex hulls of the cubics after
                // drawing the inner triangles.
                this->prePrepareFillCubicHullsProgram(args);
                fTessellator = args.fArena->make<GrPathIndirectTessellator>(fViewMatrix, fPath,
                                                                            DrawInnerFan::kNo);
            }
            return;
        }
    }

    // If we didn't triangulate the inner fan then the fill program will be a simple bounding box.
    this->prePrepareFillBoundingBoxProgram(args);

    // When there are only a few verbs, it seems to always be fastest to make a single indirect draw
    // that contains both the inner triangles and the outer cubics, instead of using hardware
    // tessellation. Also take this path if tessellation is not supported.
    bool drawTrianglesAsIndirectCubicDraw = (numVerbs < 50);
    if (drawTrianglesAsIndirectCubicDraw || (fOpFlags & OpFlags::kDisableHWTessellation)) {
        if (!drawTrianglesAsIndirectCubicDraw) {
            this->prePrepareStencilTrianglesProgram(args);
        }
        this->prePrepareStencilCubicsProgram<GrMiddleOutCubicShader>(args);
        fTessellator = args.fArena->make<GrPathIndirectTessellator>(
                fViewMatrix, fPath, DrawInnerFan(drawTrianglesAsIndirectCubicDraw));
        return;
    }

    // The caller should have sent Flags::kDisableHWTessellation if it was not supported.
    SkASSERT(args.fCaps->shaderCaps()->tessellationSupport());

    // Next see if we can split up the inner triangles and outer cubics into two draw calls. This
    // allows for a more efficient inner triangle topology that can reduce the rasterizer load by a
    // large margin on complex paths, but also causes greater CPU overhead due to the extra shader
    // switches and draw calls.
    // NOTE: Raster-edge work is 1-dimensional, so we sum height and width instead of multiplying.
    float rasterEdgeWork = (bounds.height() + bounds.width()) * scales[1] * fPath.countVerbs();
    if (rasterEdgeWork > 300 * 300) {
        this->prePrepareStencilTrianglesProgram(args);
        this->prePrepareStencilCubicsProgram<GrCubicTessellateShader>(args);
        fTessellator = args.fArena->make<GrPathOuterCurveTessellator>();
        return;
    }

    // Fastest CPU approach: emit one cubic wedge per verb, fanning out from the center.
    this->prePrepareStencilCubicsProgram<GrWedgeTessellateShader>(args);
    fTessellator = args.fArena->make<GrPathWedgeTessellator>();
}

bool GrPathTessellateOp::prePrepareInnerPolygonTriangulation(const PrePrepareArgs& args,
                                                             bool* isLinear) {
    SkASSERT(!fTriangleBuffer);
    SkASSERT(fTriangleVertexCount == 0);
    SkASSERT(!fStencilTrianglesProgram);
    SkASSERT(!fFillTrianglesProgram);
    fInnerFanTriangulator = args.fArena->make<GrInnerFanTriangulator>(fPath, args.fArena, nullptr);
    fInnerFanPolys = fInnerFanTriangulator->pathToPolys(isLinear);
    if (!fInnerFanPolys) {
        // pathToPolys will fail if the inner polygon(s) are not simple.
        return false;
    }
    if ((fOpFlags & (OpFlags::kStencilOnly | OpFlags::kWireframe)) ||
        GrAAType::kCoverage == fAAType ||
        (args.fClip && args.fClip->hasStencilClip())) {
        // If we have certain flags, mixed samples, or a stencil clip then we unfortunately
        // can't fill the inner polygon directly. Indicate that these triangles need to be
        // stencilled.
        this->prePrepareStencilTrianglesProgram(args);
    }
    this->prePrepareFillTrianglesProgram(args, *isLinear);
    return true;
}

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

constexpr static const GrUserStencilSettings* stencil_pass_settings(SkPathFillType fillType) {
    return (fillType == SkPathFillType::kWinding) ? &kIncrDecrStencil : &kInvertStencil;
}

void GrPathTessellateOp::prePrepareStencilTrianglesProgram(const PrePrepareArgs& args) {
    SkASSERT(!fStencilTrianglesProgram);

    this->prePreparePipelineForStencils(args);

    auto* shader = args.fArena->make<GrStencilTriangleShader>(fViewMatrix);
    fStencilTrianglesProgram = GrPathShader::MakeProgramInfo(
            shader, args.fArena, args.fWriteView, fPipelineForStencils, *args.fDstProxyView,
            args.fXferBarrierFlags, args.fColorLoadOp, stencil_pass_settings(fPath.getFillType()),
            *args.fCaps);
}

template<typename ShaderType>
void GrPathTessellateOp::prePrepareStencilCubicsProgram(const PrePrepareArgs& args) {
    SkASSERT(!fStencilCubicsProgram);

    this->prePreparePipelineForStencils(args);

    auto* shader = args.fArena->make<ShaderType>(fViewMatrix);
    fStencilCubicsProgram = GrPathShader::MakeProgramInfo(
            shader, args.fArena, args.fWriteView, fPipelineForStencils, *args.fDstProxyView,
            args.fXferBarrierFlags, args.fColorLoadOp, stencil_pass_settings(fPath.getFillType()),
            *args.fCaps);
}

void GrPathTessellateOp::prePreparePipelineForStencils(const PrePrepareArgs& args) {
    if (fPipelineForStencils) {
        return;
    }

    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    if (args.fCaps->wireframeSupport() && (OpFlags::kWireframe & fOpFlags)) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
    }
    SkASSERT(SkPathFillType::kWinding == fPath.getFillType() ||
             SkPathFillType::kEvenOdd == fPath.getFillType());
    initArgs.fCaps = args.fCaps;
    fPipelineForStencils = args.fArena->make<GrPipeline>(
            initArgs, GrDisableColorXPFactory::MakeXferProcessor(), *args.fHardClip);
}

// Allows non-zero stencil values to pass and write a color, and resets the stencil value back to
// zero; discards immediately on stencil values of zero.
// NOTE: It's ok to not check the clip here because the previous stencil pass will have only written
// to samples already inside the clip.
constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>());

void GrPathTessellateOp::prePrepareFillTrianglesProgram(const PrePrepareArgs& args, bool isLinear) {
    SkASSERT(!fFillTrianglesProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    // These are a twist on the standard red book stencil settings that allow us to fill the inner
    // polygon directly to the final render target. At this point, the curves are already stencilled
    // in. So if the stencil value is zero, then it means the path at our sample is not affected by
    // any curves and we fill the path in directly. If the stencil value is nonzero, then we don't
    // fill and instead continue the standard red book stencil process.
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

    this->prePreparePipelineForFills(args);

    const GrUserStencilSettings* stencil;
    if (fStencilTrianglesProgram) {
        // The path was already stencilled. Here we just need to do a cover pass.
        stencil = &kTestAndResetStencil;
    } else if (isLinear) {
        // There are no stencilled curves. We can ignore stencil and fill the path directly.
        stencil = &GrUserStencilSettings::kUnused;
    } else if (SkPathFillType::kWinding == fPath.getFillType()) {
        // Fill in the path pixels not touched by curves, incr/decr stencil otherwise.
        SkASSERT(!fPipelineForFills->hasStencilClip());
        stencil = &kFillOrIncrDecrStencil;
    } else {
        // Fill in the path pixels not touched by curves, invert stencil otherwise.
        SkASSERT(!fPipelineForFills->hasStencilClip());
        stencil = &kFillOrInvertStencil;
    }

    auto* fillTriangleShader = args.fArena->make<GrFillTriangleShader>(fViewMatrix, fColor);
    fFillTrianglesProgram = GrPathShader::MakeProgramInfo(
            fillTriangleShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, stencil, *args.fCaps);
}

void GrPathTessellateOp::prePrepareFillCubicHullsProgram(const PrePrepareArgs& args) {
    SkASSERT(!fFillPathProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    this->prePreparePipelineForFills(args);

    auto* fillCubicHullsShader = args.fArena->make<GrFillCubicHullShader>(fViewMatrix, fColor);
    fFillPathProgram = GrPathShader::MakeProgramInfo(
            fillCubicHullsShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, &kTestAndResetStencil,
            *args.fCaps);
}

void GrPathTessellateOp::prePrepareFillBoundingBoxProgram(const PrePrepareArgs& args) {
    SkASSERT(!fFillPathProgram);

    if (fOpFlags & OpFlags::kStencilOnly) {
        return;
    }

    this->prePreparePipelineForFills(args);

    auto* fillBoundingBoxShader = args.fArena->make<GrFillBoundingBoxShader>(fViewMatrix, fColor,
                                                                             fPath.getBounds());
    fFillPathProgram = GrPathShader::MakeProgramInfo(
            fillBoundingBoxShader, args.fArena, args.fWriteView, fPipelineForFills,
            *args.fDstProxyView, args.fXferBarrierFlags, args.fColorLoadOp, &kTestAndResetStencil,
            *args.fCaps);
}

void GrPathTessellateOp::prePreparePipelineForFills(const PrePrepareArgs& args) {
    SkASSERT(!(fOpFlags & OpFlags::kStencilOnly));

    if (fPipelineForFills) {
        return;
    }

    auto pipelineFlags = GrPipeline::InputFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        if (args.fWriteView.asRenderTargetProxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the cover geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(GrAAType::kCoverage == fAAType);
            if (args.fCaps->conservativeRasterSupport()) {
                pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
                pipelineFlags |= GrPipeline::InputFlags::kConservativeRaster;
            }
        } else {
            // We are standard MSAA. Leave MSAA enabled for the cover geometry.
            pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
    }

    fPipelineForFills = GrSimpleMeshDrawOpHelper::CreatePipeline(
            args.fCaps, args.fArena, args.fWriteView.swizzle(),
            (args.fClip) ? std::move(*args.fClip) : GrAppliedClip::Disabled(), *args.fDstProxyView,
            std::move(fProcessors), pipelineFlags);
}

void GrPathTessellateOp::onPrepare(GrOpFlushState* flushState) {
    int numVerbs = fPath.countVerbs();
    if (numVerbs <= 0) {
        return;
    }

    if (!fPipelineForStencils && !fPipelineForFills) {
        // Nothing has been prePrepared yet. Do it now.
        GrAppliedHardClip hardClip = GrAppliedHardClip(flushState->appliedHardClip());
        GrAppliedClip clip = flushState->detachAppliedClip();
        PrePrepareArgs args{flushState->allocator(), flushState->writeView(), &hardClip,
                            &clip, &flushState->dstProxyView(),
                            flushState->renderPassBarriers(), flushState->colorLoadOp(),
                            &flushState->caps()};
        this->prePreparePrograms(args);
    }

    if (fInnerFanPolys) {
        // prePreparePrograms was able to generate an inner polygon triangulation. It will exist in
        // either fOffThreadInnerTriangulation or fTriangleBuffer exclusively.
        SkASSERT(fInnerFanTriangulator);
        GrEagerDynamicVertexAllocator alloc(flushState, &fTriangleBuffer, &fBaseTriangleVertex);
        fTriangleVertexCount = fInnerFanTriangulator->polysToTriangles(fInnerFanPolys, &alloc);
    } else if (fStencilTrianglesProgram) {
        // The inner fan isn't built into the tessellator. Generate a standard Redbook fan with a
        // middle-out topology.
        GrEagerDynamicVertexAllocator vertexAlloc(flushState, &fTriangleBuffer,
                                                  &fBaseTriangleVertex);
        // No initial moveTo, plus an implicit close at the end; n-2 triangles fill an n-gon.
        int maxInnerTriangles = fPath.countVerbs() - 1;
        auto* triangleVertexData = vertexAlloc.lock<SkPoint>(maxInnerTriangles * 3);
        fTriangleVertexCount = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                triangleVertexData, 3/*perTriangleVertexAdvance*/, fPath) * 3;
        vertexAlloc.unlock(fTriangleVertexCount);
    }

    if (fTessellator) {
        fTessellator->prepare(flushState, fViewMatrix, fPath);
    }
}

void GrPathTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    this->drawStencilPass(flushState);
    this->drawCoverPass(flushState);
}

void GrPathTessellateOp::drawStencilPass(GrOpFlushState* flushState) {
    if (fStencilTrianglesProgram && fTriangleVertexCount > 0) {
        SkASSERT(fTriangleBuffer);
        flushState->bindPipelineAndScissorClip(*fStencilTrianglesProgram, this->bounds());
        flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
        flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);
    }

    if (fTessellator) {
        flushState->bindPipelineAndScissorClip(*fStencilCubicsProgram, this->bounds());
        fTessellator->draw(flushState);
    }
}

void GrPathTessellateOp::drawCoverPass(GrOpFlushState* flushState) {
    if (fFillTrianglesProgram) {
        SkASSERT(fTriangleBuffer);

        // We have a triangulation of the path's inner polygon. This is the fast path. Fill those
        // triangles directly to the screen.
        if (fTriangleVertexCount > 0) {
            flushState->bindPipelineAndScissorClip(*fFillTrianglesProgram, this->bounds());
            flushState->bindTextures(fFillTrianglesProgram->primProc(), nullptr,
                                     *fPipelineForFills);
            flushState->bindBuffers(nullptr, nullptr, fTriangleBuffer);
            flushState->draw(fTriangleVertexCount, fBaseTriangleVertex);
        }

        if (fTessellator) {
            // At this point, every pixel is filled in except the ones touched by curves.
            // fFillPathProgram will issue a final cover pass over the curves by drawing their
            // convex hulls. This will fill in any remaining samples and reset the stencil buffer.
            SkASSERT(fFillPathProgram);
            flushState->bindPipelineAndScissorClip(*fFillPathProgram, this->bounds());
            flushState->bindTextures(fFillPathProgram->primProc(), nullptr, *fPipelineForFills);
            fTessellator->drawHullInstances(flushState);
        }
    } else if (fFillPathProgram) {
        // There are no triangles to fill. Just draw a bounding box.
        flushState->bindPipelineAndScissorClip(*fFillPathProgram, this->bounds());
        flushState->bindTextures(fFillPathProgram->primProc(), nullptr, *fPipelineForFills);
        flushState->bindBuffers(nullptr, nullptr, nullptr);
        flushState->draw(4, 0);
    }
}
