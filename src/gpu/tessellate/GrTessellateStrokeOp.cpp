/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellateStrokeOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokePatchBuilder.h"
#include "src/gpu/tessellate/GrTessellateStrokeShader.h"

static SkPath transform_path(const SkMatrix& viewMatrix, const SkPath& path) {
    SkPath devPath;
    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokePatchBuilder with minimal modifications.
    SkASSERT(viewMatrix.isSimilarity());
    path.transform(viewMatrix, &devPath);
    return devPath;
}

static SkStrokeRec transform_stroke(const SkMatrix& viewMatrix, const SkStrokeRec& stroke) {
    SkStrokeRec devStroke = stroke;
    // kStrokeAndFill_Style is not yet supported.
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    float strokeWidth = (stroke.getStyle() == SkStrokeRec::kHairline_Style) ?
            1 : viewMatrix.getMaxScale() * stroke.getWidth();
    devStroke.setStrokeStyle(strokeWidth, /*strokeAndFill=*/false);
    return devStroke;
}

static SkPMColor4f get_paint_constant_blended_color(const GrPaint& paint) {
    SkPMColor4f constantColor;
    // Patches can overlap, so until a stencil technique is implemented, the provided paints must be
    // constant blended colors.
    SkAssertResult(paint.isConstantBlendedColor(&constantColor));
    return constantColor;
}

GrTessellateStrokeOp::GrTessellateStrokeOp(const SkMatrix& viewMatrix, const SkPath& path,
                                           const SkStrokeRec& stroke, GrPaint&& paint,
                                           GrAAType aaType)
        : GrDrawOp(ClassID())
        , fPathStrokes(transform_path(viewMatrix, path), transform_stroke(viewMatrix, stroke))
        , fTotalCombinedVerbCnt(path.countVerbs())
        , fColor(get_paint_constant_blended_color(paint))
        , fAAType(aaType)
        , fProcessors(std::move(paint)) {
    SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples support yet.
    SkStrokeRec& headStroke = fPathStrokes.head().fStroke;
    if (headStroke.getJoin() == SkPaint::kMiter_Join) {
        float miter = headStroke.getMiter();
        if (miter <= 0) {
            headStroke.setStrokeParams(headStroke.getCap(), SkPaint::kBevel_Join, 0);
        } else {
            fMiterLimitOrZero = miter;
        }
    }
    SkRect devBounds = fPathStrokes.head().fPath.getBounds();
    float inflationRadius = fPathStrokes.head().fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

GrDrawOp::FixedFunctionFlags GrTessellateStrokeOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrTessellateStrokeOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        bool hasMixedSampledCoverage,
                                                        GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip,
                                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
                                clampType, &fColor);
}

GrOp::CombineResult GrTessellateStrokeOp::onCombineIfPossible(GrOp* grOp,
                                                              GrRecordingContext::Arenas* arenas,
                                                              const GrCaps&) {
    auto* op = grOp->cast<GrTessellateStrokeOp>();
    if (fColor != op->fColor ||
        fViewMatrix != op->fViewMatrix ||
        fAAType != op->fAAType ||
        ((fMiterLimitOrZero * op->fMiterLimitOrZero != 0) &&  // Are both non-zero?
         fMiterLimitOrZero != op->fMiterLimitOrZero) ||
        fProcessors != op->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    fPathStrokes.concat(std::move(op->fPathStrokes), arenas->recordTimeAllocator());
    if (op->fMiterLimitOrZero != 0) {
        SkASSERT(fMiterLimitOrZero == 0 || fMiterLimitOrZero == op->fMiterLimitOrZero);
        fMiterLimitOrZero = op->fMiterLimitOrZero;
    }
    fTotalCombinedVerbCnt += op->fTotalCombinedVerbCnt;

    return CombineResult::kMerged;
}

void GrTessellateStrokeOp::onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView* writeView,
                                        GrAppliedClip*, const GrXferProcessor::DstProxyView&) {
}

void GrTessellateStrokeOp::onPrepare(GrOpFlushState* flushState) {
    GrStrokePatchBuilder builder(flushState, &fVertexChunks, fTotalCombinedVerbCnt);
    for (auto& [path, stroke] : fPathStrokes) {
        builder.addPath(path, stroke);
    }
}

void GrTessellateStrokeOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    GrPipeline::InitArgs initArgs;
    if (GrAAType::kNone != fAAType) {
        initArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        SkASSERT(flushState->proxy()->numSamples() > 1);  // No mixed samples yet.
        SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples yet.
    }
    initArgs.fCaps = &flushState->caps();
    initArgs.fDstProxyView = flushState->drawOpArgs().dstProxyView();
    initArgs.fWriteSwizzle = flushState->drawOpArgs().writeSwizzle();
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    SkASSERT(fViewMatrix.isIdentity());  // Only identity matrices supported for now.
    GrTessellateStrokeShader strokeShader(fViewMatrix, fColor, fMiterLimitOrZero);
    GrPathShader::ProgramInfo programInfo(flushState->writeView(), &pipeline, &strokeShader);

    SkASSERT(chainBounds == this->bounds());
    flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
    flushState->bindTextures(strokeShader, nullptr, pipeline);

    for (const auto& chunk : fVertexChunks) {
        if (chunk.fVertexBuffer) {
            flushState->bindBuffers(nullptr, nullptr, std::move(chunk.fVertexBuffer));
            flushState->draw(chunk.fVertexCount, chunk.fBaseVertex);
        }
    }
}
