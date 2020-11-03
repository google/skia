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

static SkPMColor4f get_paint_constant_blended_color(const GrPaint& paint) {
    SkPMColor4f constantColor;
    // Patches can overlap, so until a stencil technique is implemented, the provided paints must be
    // constant blended colors.
    SkAssertResult(paint.isConstantBlendedColor(&constantColor));
    return constantColor;
}

GrStrokeOp::GrStrokeOp(uint32_t classID, GrAAType aaType, const SkMatrix& viewMatrix,
                       const SkStrokeRec& stroke, const SkPath& path, GrPaint&& paint)
        : GrDrawOp(classID)
        , fAAType(aaType)
        , fViewMatrix(viewMatrix)
        , fStroke(stroke)
        , fParametricIntolerance(
                fViewMatrix.getMaxScale() * GrTessellationPathRenderer::kLinearizationIntolerance)
        , fNumRadialSegmentsPerRadian(
                .5f / acosf(std::max(1 - 2/(fParametricIntolerance * fStroke.getWidth()), -1.f)))
        , fColor(get_paint_constant_blended_color(paint))
        , fProcessors(std::move(paint))
        , fPathList(path)
        , fTotalCombinedVerbCnt(path.countVerbs()) {
    // We don't support hairline strokes. For now, the client can transform the path into device
    // space and then use a stroke width of 1.
    SkASSERT(fStroke.getWidth() > 0);
    SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples support yet.
    SkASSERT(fParametricIntolerance >= 0);
    SkRect devBounds = path.getBounds();
    float inflationRadius = fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

GrDrawOp::FixedFunctionFlags GrStrokeOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrStrokeOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                              bool hasMixedSampledCoverage, GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip,
                                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
                                clampType, &fColor);
}

GrOp::CombineResult GrStrokeOp::onCombineIfPossible(GrOp* grOp, SkArenaAlloc* alloc,
                                                    const GrCaps&) {
    SkASSERT(grOp->classID() == this->classID());
    auto* op = static_cast<GrStrokeOp*>(grOp);
    if (fColor != op->fColor ||
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

void GrStrokeOp::prePrepareColorProgram(SkArenaAlloc* arena,
                                        GrStrokeTessellateShader* strokeTessellateShader,
                                        const GrSurfaceProxyView* writeView, GrAppliedClip&& clip,
                                        const GrXferProcessor::DstProxyView& dstProxyView,
                                        GrXferBarrierFlags renderPassXferBarriers,
                                        const GrCaps& caps) {
    SkASSERT(!fColorProgram);
    auto pipelineFlags = GrPipeline::InputFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        SkASSERT(writeView->asRenderTargetProxy()->numSamples() > 1);  // No mixed samples yet.
        SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples yet.
    }
    fColorProgram = GrPathShader::MakeProgramInfo(strokeTessellateShader, arena, writeView,
                                                  pipelineFlags, std::move(fProcessors),
                                                  std::move(clip), dstProxyView,
                                                  renderPassXferBarriers,
                                                  &GrUserStencilSettings::kUnused, caps);
}
