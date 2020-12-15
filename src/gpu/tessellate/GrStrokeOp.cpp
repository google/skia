/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

GrStrokeOp::GrStrokeOp(uint32_t classID, GrAAType aaType, const SkMatrix& viewMatrix,
                       const SkStrokeRec& stroke, const SkPath& path, GrPaint&& paint)
        : GrDrawOp(classID)
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fStroke(stroke)
        , fColor(paint.getColor4f())
        , fProcessors(std::move(paint))
        , fPathList(path)
        , fTotalCombinedVerbCnt(path.countVerbs())
        , fTotalConicWeightCnt(SkPathPriv::ConicWeightCnt(path)) {
    SkRect devBounds = path.getBounds();
    float inflationRadius = fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

void GrStrokeOp::visitProxies(const VisitProxyFunc& fn) const {
    if (fFillProgram) {
        fFillProgram->visitFPProxies(fn);
    } else if (fStencilProgram) {
        fStencilProgram->visitFPProxies(fn);
    } else {
        fProcessors.visitProxies(fn);
    }
}

GrDrawOp::FixedFunctionFlags GrStrokeOp::fixedFunctionFlags() const {
    // We might not actually end up needing stencil, but won't know for sure until finalize().
    // Request it just in case we do end up needing it.
    auto flags = FixedFunctionFlags::kUsesStencil;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrStrokeOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                              bool hasMixedSampledCoverage, GrClampType clampType) {
    // Make sure the finalize happens before combining. We might change fNeedsStencil here.
    SkASSERT(fPathList.begin().fCurr->fNext == nullptr);
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
            hasMixedSampledCoverage, caps, clampType, &fColor);
    fNeedsStencil = !analysis.unaffectedByDstValue();
    return analysis;
}

GrOp::CombineResult GrStrokeOp::onCombineIfPossible(GrOp* grOp, SkArenaAlloc* alloc,
                                                    const GrCaps&) {
    SkASSERT(grOp->classID() == this->classID());
    auto* op = static_cast<GrStrokeOp*>(grOp);
    if (fNeedsStencil ||
        op->fNeedsStencil ||
        fColor != op->fColor ||
        fViewMatrix != op->fViewMatrix ||
        fAAType != op->fAAType ||
        !fStroke.hasEqualEffect(op->fStroke) ||
        fProcessors != op->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    fPathList.concat(std::move(op->fPathList), alloc);
    fTotalCombinedVerbCnt += op->fTotalCombinedVerbCnt;
    fTotalConicWeightCnt += op->fTotalConicWeightCnt;

    return CombineResult::kMerged;
}

// Marks every stencil value as "1".
constexpr static GrUserStencilSettings kMarkStencil(
    GrUserStencilSettings::StaticInit<
        0x0001,
        GrUserStencilTest::kLessIfInClip,  // Match kTestAndResetStencil.
        0x0000,  // Always fail.
        GrUserStencilOp::kZero,
        GrUserStencilOp::kReplace,
        0xffff>());

// Passes if the stencil value is nonzero. Also resets the stencil value to zero on pass. This is
// formulated to match kMarkStencil everywhere except the ref and compare mask. This will allow us
// to use the same pipeline for both stencil and fill if dynamic stencil state is supported.
constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kLessIfInClip,  // i.e., "not equal to zero, if in clip".
        0x0001,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kReplace,
        0xffff>());

void GrStrokeOp::prePreparePrograms(GrStrokeTessellateShader::Mode shaderMode, SkArenaAlloc* arena,
                                    const GrSurfaceProxyView& writeView, GrAppliedClip&& clip,
                                    const GrXferProcessor::DstProxyView& dstProxyView,
                                    GrXferBarrierFlags renderPassXferBarriers,
                                    GrLoadOp colorLoadOp, const GrCaps& caps) {
    using InputFlags = GrPipeline::InputFlags;
    SkASSERT(!fFillProgram);
    SkASSERT(!fStencilProgram);

    // This will be created iff the stencil pass can't share a pipeline with the fill pass.
    GrPipeline* standaloneStencilPipeline = nullptr;

    GrPipeline::InitArgs fillArgs;
    fillArgs.fCaps = &caps;
    fillArgs.fDstProxyView = dstProxyView;
    fillArgs.fWriteSwizzle = writeView.swizzle();
    if (fAAType != GrAAType::kNone) {
        if (writeView.asRenderTargetProxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the cover geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(GrAAType::kCoverage == fAAType);
            if (caps.conservativeRasterSupport()) {
                fillArgs.fInputFlags |= InputFlags::kHWAntialias | InputFlags::kConservativeRaster;
            }
            // Since we either need conservative raster enabled or MSAA disabled during fill, we
            // need a separate pipeline for the stencil pass.
            SkASSERT(fNeedsStencil);  // Mixed samples always needs stencil.
            GrPipeline::InitArgs stencilArgs;
            stencilArgs.fCaps = &caps;
            stencilArgs.fInputFlags = InputFlags::kHWAntialias;
            stencilArgs.fWriteSwizzle = writeView.swizzle();
            standaloneStencilPipeline = arena->make<GrPipeline>(
                    stencilArgs, GrDisableColorXPFactory::MakeXferProcessor(), clip.hardClip());
        } else {
            // We are standard MSAA. Leave MSAA enabled for both the fill and stencil passes.
            fillArgs.fInputFlags |= InputFlags::kHWAntialias;
        }
    }

    auto* strokeTessellateShader = arena->make<GrStrokeTessellateShader>(
            shaderMode, fTotalConicWeightCnt, fStroke, fViewMatrix, fColor);
    auto fillPipeline = arena->make<GrPipeline>(fillArgs, std::move(fProcessors), std::move(clip));
    auto fillStencil = &GrUserStencilSettings::kUnused;
    auto fillXferFlags = renderPassXferBarriers;
    if (fNeedsStencil) {
        auto* stencilPipeline = (standaloneStencilPipeline) ? standaloneStencilPipeline
                                                            : fillPipeline;
        fStencilProgram = GrPathShader::MakeProgramInfo(strokeTessellateShader, arena, writeView,
                                                        stencilPipeline, dstProxyView,
                                                        renderPassXferBarriers, colorLoadOp,
                                                        &kMarkStencil, caps);
        fillStencil = &kTestAndResetStencil;
        fillXferFlags = GrXferBarrierFlags::kNone;
    }
    fFillProgram = GrPathShader::MakeProgramInfo(strokeTessellateShader, arena, writeView,
                                                 fillPipeline, dstProxyView, fillXferFlags,
                                                 colorLoadOp, fillStencil, caps);
}
