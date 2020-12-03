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
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

GrStrokeOp::GrStrokeOp(uint32_t classID, GrAAType aaType, const SkMatrix& viewMatrix,
                       const SkStrokeRec& stroke, const SkPath& path, GrPaint&& paint)
        : GrDrawOp(classID)
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fStroke(stroke)
        , fParametricIntolerance(
                viewMatrix.getMaxScale() * GrTessellationPathRenderer::kLinearizationIntolerance)
        , fNumRadialSegmentsPerRadian(
                .5f / acosf(std::max(1 - 2/(fParametricIntolerance * fStroke.getWidth()), -1.f)))
        , fColor(paint.getColor4f())
        , fProcessors(std::move(paint))
        , fPathList(path)
        , fTotalCombinedVerbCnt(path.countVerbs()) {
    // We don't support hairline strokes. For now, the client can transform the path into device
    // space and then use a stroke width of 1.
    SkASSERT(fStroke.getWidth() > 0);
    SkASSERT(fParametricIntolerance >= 0);
    SkRect devBounds = path.getBounds();
    float inflationRadius = fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
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

    return CombineResult::kMerged;
}

// Marks every stencil value as "1".
constexpr static GrUserStencilSettings kMarkStencil(
    GrUserStencilSettings::StaticInit<
        0x0001,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kReplace,
        GrUserStencilOp::kKeep,
        0xffff>());

// Fills in color if the stencil value is nonzero. Resets the stencil value to zero.
constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>());

void GrStrokeOp::prePreparePrograms(SkArenaAlloc* arena,
                                    GrStrokeTessellateShader* strokeTessellateShader,
                                    const GrSurfaceProxyView& writeView, GrAppliedClip&& clip,
                                    const GrXferProcessor::DstProxyView& dstProxyView,
                                    GrXferBarrierFlags renderPassXferBarriers,
                                    GrLoadOp colorLoadOp, const GrCaps& caps) {
    SkASSERT(!fFillProgram);
    SkASSERT(!fStencilProgram);
    const GrUserStencilSettings* fillStencilSettings = &GrUserStencilSettings::kUnused;
    if (fNeedsStencil) {
        auto stencilPipeFlags = (fAAType != GrAAType::kNone) ? GrPipeline::InputFlags::kHWAntialias
                                                             : GrPipeline::InputFlags::kNone;
        GrPipeline::InitArgs initArgs;
        initArgs.fInputFlags = stencilPipeFlags;
        initArgs.fCaps = &caps;
        GrPipeline* stencilPipeline = arena->make<GrPipeline>(
                initArgs, GrDisableColorXPFactory::MakeXferProcessor(), clip.hardClip());
        fStencilProgram = GrPathShader::MakeProgramInfo(
                strokeTessellateShader, arena, writeView, stencilPipeline, dstProxyView,
                renderPassXferBarriers, colorLoadOp, &kMarkStencil, caps);
        fillStencilSettings = &kTestAndResetStencil;
    }
    auto fillPipeFlags = GrPipeline::InputFlags::kNone;
    if (fAAType != GrAAType::kNone) {
        if (writeView.asRenderTargetProxy()->numSamples() == 1) {
            // We are mixed sampled. We need to either enable conservative raster (preferred) or
            // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
            // the cover geometry, the stencil test is still multisampled and will still produce
            // smooth results.)
            SkASSERT(GrAAType::kCoverage == fAAType);
            if (caps.conservativeRasterSupport()) {
                fillPipeFlags |= GrPipeline::InputFlags::kHWAntialias;
                fillPipeFlags |= GrPipeline::InputFlags::kConservativeRaster;
            }
        } else {
            // We are standard MSAA. Leave MSAA enabled for the cover geometry.
            fillPipeFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
    }
    fFillProgram = GrPathShader::MakeProgramInfo(strokeTessellateShader, arena, writeView,
                                                 fillPipeFlags, std::move(fProcessors),
                                                 std::move(clip), dstProxyView,
                                                 renderPassXferBarriers, colorLoadOp,
                                                 fillStencilSettings, caps);
}
