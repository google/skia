/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/GrStrokePatchBuilder.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

static SkPMColor4f get_paint_constant_blended_color(const GrPaint& paint) {
    SkPMColor4f constantColor;
    // Patches can overlap, so until a stencil technique is implemented, the provided paints must be
    // constant blended colors.
    SkAssertResult(paint.isConstantBlendedColor(&constantColor));
    return constantColor;
}

GrStrokeTessellateOp::GrStrokeTessellateOp(GrAAType aaType, const SkMatrix& viewMatrix,
                                           const SkPath& path, const SkStrokeRec& stroke,
                                           GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fPathStrokes(path, stroke)
        , fTotalCombinedVerbCnt(path.countVerbs())
        , fAAType(aaType)
        , fColor(get_paint_constant_blended_color(paint))
        , fProcessors(std::move(paint)) {
    SkASSERT(fAAType != GrAAType::kCoverage);  // No mixed samples support yet.
    if (stroke.getJoin() == SkPaint::kMiter_Join) {
        float miter = stroke.getMiter();
        if (miter <= 0) {
            fPathStrokes.head().fStroke.setStrokeParams(stroke.getCap(), SkPaint::kBevel_Join, 0);
        } else {
            fMiterLimitOrZero = miter;
        }
    }
    if (!(viewMatrix.getType() & ~SkMatrix::kScale_Mask) &&
        viewMatrix.getScaleX() == viewMatrix.getScaleY()) {
        fMatrixScale = viewMatrix.getScaleX();
        fSkewMatrix = SkMatrix::I();
    } else {
        SkASSERT(!viewMatrix.hasPerspective());  // getMaxScale() doesn't work with perspective.
        fMatrixScale = viewMatrix.getMaxScale();
        float invScale = SkScalarInvert(fMatrixScale);
        fSkewMatrix = viewMatrix;
        fSkewMatrix.preScale(invScale, invScale);
    }
    SkASSERT(fMatrixScale >= 0);
    SkRect devBounds = fPathStrokes.head().fPath.getBounds();
    float inflationRadius = fPathStrokes.head().fStroke.getInflationRadius();
    devBounds.outset(inflationRadius, inflationRadius);
    viewMatrix.mapRect(&devBounds, devBounds);
    this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
}

GrDrawOp::FixedFunctionFlags GrStrokeTessellateOp::fixedFunctionFlags() const {
    auto flags = FixedFunctionFlags::kNone;
    if (GrAAType::kNone != fAAType) {
        flags |= FixedFunctionFlags::kUsesHWAA;
    }
    return flags;
}

GrProcessorSet::Analysis GrStrokeTessellateOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip,
                                                        bool hasMixedSampledCoverage,
                                                        GrClampType clampType) {
    return fProcessors.finalize(fColor, GrProcessorAnalysisCoverage::kNone, clip,
                                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
                                clampType, &fColor);
}

GrOp::CombineResult GrStrokeTessellateOp::onCombineIfPossible(GrOp* grOp,
                                                              GrRecordingContext::Arenas* arenas,
                                                              const GrCaps&) {
    auto* op = grOp->cast<GrStrokeTessellateOp>();
    if (fColor != op->fColor ||
        // TODO: When stroking is finished, we may want to consider whether a unique matrix scale
        // can be stored with each PathStroke instead. This might improve batching.
        fMatrixScale != op->fMatrixScale ||
        fSkewMatrix != op->fSkewMatrix ||
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

void GrStrokeTessellateOp::onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView* writeView,
                                        GrAppliedClip*, const GrXferProcessor::DstProxyView&) {
}

void GrStrokeTessellateOp::onPrepare(GrOpFlushState* flushState) {
    GrStrokePatchBuilder builder(flushState, &fVertexChunks, fMatrixScale, fTotalCombinedVerbCnt);
    for (auto& [path, stroke] : fPathStrokes) {
        builder.addPath(path, stroke);
    }
}

void GrStrokeTessellateOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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

    GrStrokeTessellateShader strokeShader(fSkewMatrix, fColor, fMiterLimitOrZero);
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
