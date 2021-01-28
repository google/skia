/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathInnerTriangulateOp.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrFillPathShader.h"
#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

using OpFlags = GrTessellationPathRenderer::OpFlags;

void GrPathInnerTriangulateOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fPipelineForFills) {
        fPipelineForFills->visitProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrPathInnerTriangulateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrPathInnerTriangulateOp::finalize(const GrCaps& caps,
                                                            const GrAppliedClip* clip,
                                                            bool hasMixedSampledCoverage,
                                                            GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip, nullptr,
                                hasMixedSampledCoverage, caps, clampType, &fColor);
}

void GrPathInnerTriangulateOp::pushFanStencilProgram(const GrPathShader::ProgramArgs& args,
                                                     const GrPipeline* pipelineForStencils,
                                                     const GrUserStencilSettings* stencil) {
    SkASSERT(pipelineForStencils);
    fFanPrograms.push_back(GrStencilPathShader::MakeStencilProgram<GrStencilTriangleShader>(
            args, fViewMatrix, pipelineForStencils, stencil));
}

void GrPathInnerTriangulateOp::pushFanFillProgram(const GrPathShader::ProgramArgs& args,
                                                  const GrUserStencilSettings* stencil) {
    SkASSERT(fPipelineForFills);
    auto* shader = args.fArena->make<GrFillTriangleShader>(fViewMatrix, fColor);
    fFanPrograms.push_back(GrPathShader::MakeProgram(args, shader, fPipelineForFills, stencil));
}

void GrPathInnerTriangulateOp::prePreparePrograms(const GrPathShader::ProgramArgs& args,
                                                  GrAppliedClip&& appliedClip) {
    SkASSERT(!fFanTriangulator);
    SkASSERT(!fFanPolys);
    SkASSERT(!fPipelineForFills);
    SkASSERT(!fTessellator);
    SkASSERT(!fStencilCurvesProgram);
    SkASSERT(fFanPrograms.empty());
    SkASSERT(!fFillHullsProgram);

    if (fPath.countVerbs() <= 0) {
        return;
    }

    // If using wireframe or mixed samples, we have to fall back on a standard Redbook "stencil then
    // fill" algorithm instead of bypassing the stencil buffer to fill the fan directly.
    bool forceRedbookStencilPass = (fOpFlags & (OpFlags::kStencilOnly | OpFlags::kWireframe)) ||
                                   fAAType == GrAAType::kCoverage;  // i.e., mixed samples!
    bool doFill = !(fOpFlags & OpFlags::kStencilOnly);

    bool isLinear;
    fFanTriangulator = args.fArena->make<GrInnerFanTriangulator>(fPath, args.fArena);
    fFanPolys = fFanTriangulator->pathToPolys(&fFanBreadcrumbs, &isLinear);

    // Create a pipeline for stencil passes if needed.
    const GrPipeline* pipelineForStencils = nullptr;
    if (forceRedbookStencilPass || !isLinear) {  // Curves always get stencilled.
        pipelineForStencils = GrStencilPathShader::MakeStencilPassPipeline(args, fAAType, fOpFlags,
                                                                           appliedClip.hardClip());
    }

    // Create a pipeline for fill passes if needed.
    if (doFill) {
        fPipelineForFills = GrFillPathShader::MakeFillPassPipeline(args, fAAType,
                                                                   std::move(appliedClip),
                                                                   std::move(fProcessors));
    }

    // Pass 1: Tessellate the outer curves into the stencil buffer.
    if (!isLinear) {
        // Always use indirect draws for now. Our goal in this op is to maximize GPU performance,
        // and the middle-out topology used by indirect draws is easier on the rasterizer than what
        // we can do with hw tessellation. So far we haven't found any platforms where trying to use
        // hw tessellation here is worth it.
        using DrawInnerFan = GrPathIndirectTessellator::DrawInnerFan;
        fTessellator = args.fArena->make<GrPathIndirectTessellator>(fViewMatrix, fPath,
                                                                    DrawInnerFan::kNo);
        fStencilCurvesProgram = GrStencilPathShader::MakeStencilProgram<GrMiddleOutCubicShader>(
                args, fViewMatrix, pipelineForStencils, fPath.getFillType());
    }

    // Pass 2: Fill the path's inner fan with a stencil test against the curves.
    if (fFanPolys) {
        if (forceRedbookStencilPass) {
            // Use a standard Redbook "stencil then fill" algorithm instead of bypassing the stencil
            // buffer to fill the fan directly.
            this->pushFanStencilProgram(
                    args, pipelineForStencils,
                    GrStencilPathShader::StencilPassSettings(fPath.getFillType()));
            if (doFill) {
                this->pushFanFillProgram(args, GrFillPathShader::TestAndResetStencilSettings());
            }
        } else if (isLinear) {
            // There are no outer curves! Ignore stencil and fill the path directly.
            SkASSERT(!pipelineForStencils);
            this->pushFanFillProgram(args, &GrUserStencilSettings::kUnused);
        } else if (!fPipelineForFills->hasStencilClip()) {
            // These are a twist on the standard Redbook stencil settings that allow us to fill the
            // inner polygon directly to the final render target. By the time these programs
            // execute, the outer curves will already be stencilled in. So if the stencil value is
            // zero, then it means the sample in question is not affected by any curves and we can
            // fill it in directly. If the stencil value is nonzero, then we don't fill and instead
            // continue the standard Redbook counting process.
            constexpr static GrUserStencilSettings kFillOrIncrDecrStencil(
                GrUserStencilSettings::StaticInitSeparate<
                    0x0000,                       0x0000,
                    GrUserStencilTest::kEqual,    GrUserStencilTest::kEqual,
                    0xffff,                       0xffff,
                    GrUserStencilOp::kKeep,       GrUserStencilOp::kKeep,
                    GrUserStencilOp::kIncWrap,    GrUserStencilOp::kDecWrap,
                    0xffff,                       0xffff>());

            constexpr static GrUserStencilSettings kFillOrInvertStencil(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    GrUserStencilTest::kEqual,
                    0xffff,
                    GrUserStencilOp::kKeep,
                    // "Zero" instead of "Invert" because the fan only touches any given pixel once.
                    GrUserStencilOp::kZero,
                    0xffff>());

            auto* stencil = (fPath.getFillType() == SkPathFillType::kWinding)
                    ? &kFillOrIncrDecrStencil
                    : &kFillOrInvertStencil;
            this->pushFanFillProgram(args, stencil);
        } else {
            // This is the same idea as above, but we use two passes instead of one because there is
            // a stencil clip. The stencil test isn't expressive enough to do the above tests and
            // also check the clip bit in a single pass.
            constexpr static GrUserStencilSettings kFillIfZeroAndInClip(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    GrUserStencilTest::kEqualIfInClip,
                    0xffff,
                    GrUserStencilOp::kKeep,
                    GrUserStencilOp::kKeep,
                    0xffff>());

            constexpr static GrUserStencilSettings kIncrDecrStencilIfNonzero(
                GrUserStencilSettings::StaticInitSeparate<
                    0x0000,                         0x0000,
                    // No need to check the clip because the previous stencil pass will have only
                    // written to samples already inside the clip.
                    GrUserStencilTest::kNotEqual,   GrUserStencilTest::kNotEqual,
                    0xffff,                         0xffff,
                    GrUserStencilOp::kIncWrap,      GrUserStencilOp::kDecWrap,
                    GrUserStencilOp::kKeep,         GrUserStencilOp::kKeep,
                    0xffff,                         0xffff>());

            constexpr static GrUserStencilSettings kInvertStencilIfNonZero(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    // No need to check the clip because the previous stencil pass will have only
                    // written to samples already inside the clip.
                    GrUserStencilTest::kNotEqual,
                    0xffff,
                    // "Zero" instead of "Invert" because the fan only touches any given pixel once.
                    GrUserStencilOp::kZero,
                    GrUserStencilOp::kKeep,
                    0xffff>());

            // Pass 2a: Directly fill fan samples whose stencil values (from curves) are zero.
            this->pushFanFillProgram(args, &kFillIfZeroAndInClip);

            // Pass 2b: Redbook counting on fan samples whose stencil values (from curves) != 0.
            auto* stencil = (fPath.getFillType() == SkPathFillType::kWinding)
                    ? &kIncrDecrStencilIfNonzero
                    : &kInvertStencilIfNonZero;
            this->pushFanStencilProgram(args, pipelineForStencils, stencil);
        }
    }

    // Pass 3: Draw convex hulls around each curve.
    if (doFill && !isLinear) {
        // By the time this program executes, every pixel will be filled in except the ones touched
        // by curves. We issue a final cover pass over the curves by drawing their convex hulls.
        // This will fill in any remaining samples and reset the stencil values back to zero.
        SkASSERT(fTessellator);
        auto* hullShader = args.fArena->make<GrFillCubicHullShader>(fViewMatrix, fColor);
        fFillHullsProgram = GrPathShader::MakeProgram(
                args, hullShader, fPipelineForFills,
                GrFillPathShader::TestAndResetStencilSettings());
    }
}

void GrPathInnerTriangulateOp::onPrePrepare(GrRecordingContext* context,
                                            const GrSurfaceProxyView& writeView,
                                            GrAppliedClip* clip,
                                            const GrXferProcessor::DstProxyView& dstProxyView,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp) {
    this->prePreparePrograms({context->priv().recordTimeAllocator(), writeView, &dstProxyView,
                             renderPassXferBarriers, colorLoadOp, context->priv().caps()},
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled());
    if (fStencilCurvesProgram) {
        context->priv().recordProgramInfo(fStencilCurvesProgram);
    }
    for (const GrProgramInfo* fanProgram : fFanPrograms) {
        context->priv().recordProgramInfo(fanProgram);
    }
    if (fFillHullsProgram) {
        context->priv().recordProgramInfo(fFillHullsProgram);
    }
}

void GrPathInnerTriangulateOp::onPrepare(GrOpFlushState* flushState) {
    if (!fFanTriangulator) {
        this->prePreparePrograms({flushState->allocator(), flushState->writeView(),
                                 &flushState->dstProxyView(), flushState->renderPassBarriers(),
                                 flushState->colorLoadOp(), &flushState->caps()},
                                 flushState->detachAppliedClip());
        if (!fFanTriangulator) {
            return;
        }
    }

    if (fFanPolys) {
        GrEagerDynamicVertexAllocator alloc(flushState, &fFanBuffer, &fBaseFanVertex);
        fFanVertexCount = fFanTriangulator->polysToTriangles(fFanPolys, &alloc, &fFanBreadcrumbs);
    }

    if (fTessellator) {
        // Must be called after polysToTriangles() in order for fFanBreadcrumbs to be complete.
        fTessellator->prepare(flushState, fViewMatrix, fPath, &fFanBreadcrumbs);
    }
}

void GrPathInnerTriangulateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fStencilCurvesProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fStencilCurvesProgram, this->bounds());
        fTessellator->draw(flushState);
    }

    for (const GrProgramInfo* fanProgram : fFanPrograms) {
        SkASSERT(fFanBuffer);
        flushState->bindPipelineAndScissorClip(*fanProgram, this->bounds());
        flushState->bindTextures(fanProgram->primProc(), nullptr, fanProgram->pipeline());
        flushState->bindBuffers(nullptr, nullptr, fFanBuffer);
        flushState->draw(fFanVertexCount, fBaseFanVertex);
    }

    if (fFillHullsProgram) {
        SkASSERT(fTessellator);
        flushState->bindPipelineAndScissorClip(*fFillHullsProgram, this->bounds());
        flushState->bindTextures(fFillHullsProgram->primProc(), nullptr, *fPipelineForFills);
        fTessellator->drawHullInstances(flushState);
    }
}
