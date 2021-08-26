/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrRect.h"

GrSimpleMeshDrawOpHelper::GrSimpleMeshDrawOpHelper(GrProcessorSet* processorSet,
                                                   GrAAType aaType,
                                                   InputFlags inputFlags)
        : fProcessors(processorSet)
        , fPipelineFlags((GrPipeline::InputFlags)inputFlags)
        , fAAType((int)aaType)
        , fUsesLocalCoords(false)
        , fCompatibleWithCoverageAsAlpha(false) {
    SkDEBUGCODE(fDidAnalysis = false);
    SkDEBUGCODE(fMadePipeline = false);
}

GrSimpleMeshDrawOpHelper::~GrSimpleMeshDrawOpHelper() {
    if (fProcessors) {
        fProcessors->~GrProcessorSet();
    }
}

GrDrawOp::FixedFunctionFlags GrSimpleMeshDrawOpHelper::fixedFunctionFlags() const {
    return GrAATypeIsHW(this->aaType()) ? GrDrawOp::FixedFunctionFlags::kUsesHWAA
                                        : GrDrawOp::FixedFunctionFlags::kNone;
}

bool GrSimpleMeshDrawOpHelper::isCompatible(const GrSimpleMeshDrawOpHelper& that,
                                            const GrCaps& caps, const SkRect& thisBounds,
                                            const SkRect& thatBounds, bool ignoreAAType) const {
    if (SkToBool(fProcessors) != SkToBool(that.fProcessors)) {
        return false;
    }
    if (fProcessors) {
        if (*fProcessors != *that.fProcessors) {
            return false;
        }
    }

#ifdef SK_DEBUG
    if (ignoreAAType) {
        // If we're ignoring AA it should be bc we already know they are the same or that
        // the are different but are compatible (i.e., one is AA and the other is None)
        SkASSERT(fAAType == that.fAAType ||
                 GrMeshDrawOp::CanUpgradeAAOnMerge(this->aaType(), that.aaType()));
    }
#endif

    bool result = fPipelineFlags == that.fPipelineFlags &&
                  (ignoreAAType || fAAType == that.fAAType);
    SkASSERT(!result || fCompatibleWithCoverageAsAlpha == that.fCompatibleWithCoverageAsAlpha);
    SkASSERT(!result || fUsesLocalCoords == that.fUsesLocalCoords);
    return result;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelper::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, GrClampType clampType,
        GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(caps, clip, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = !geometryColor->fitsInBytes();
    }
    return result;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelper::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, const GrUserStencilSettings* userStencil,
        GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
        GrProcessorAnalysisColor* geometryColor) {
    SkDEBUGCODE(fDidAnalysis = true);
    GrProcessorSet::Analysis analysis;
    if (fProcessors) {
        GrProcessorAnalysisCoverage coverage = geometryCoverage;
        if (GrProcessorAnalysisCoverage::kNone == coverage) {
            coverage = (clip && clip->hasCoverageFragmentProcessor())
                               ? GrProcessorAnalysisCoverage::kSingleChannel
                               : GrProcessorAnalysisCoverage::kNone;
        }
        SkPMColor4f overrideColor;
        analysis = fProcessors->finalize(*geometryColor, coverage, clip, userStencil, caps,
                                         clampType, &overrideColor);
        if (analysis.inputColorIsOverridden()) {
            *geometryColor = overrideColor;
        }
    } else {
        analysis = GrProcessorSet::EmptySetAnalysis();
    }
    fUsesLocalCoords = analysis.usesLocalCoords();
    fCompatibleWithCoverageAsAlpha = analysis.isCompatibleWithCoverageAsAlpha();
    return analysis;
}

const GrPipeline* GrSimpleMeshDrawOpHelper::CreatePipeline(
                                                const GrCaps* caps,
                                                SkArenaAlloc* arena,
                                                GrSwizzle writeViewSwizzle,
                                                GrAppliedClip&& appliedClip,
                                                const GrDstProxyView& dstProxyView,
                                                GrProcessorSet&& processorSet,
                                                GrPipeline::InputFlags pipelineFlags) {
    GrPipeline::InitArgs pipelineArgs;

    pipelineArgs.fInputFlags = pipelineFlags;
    pipelineArgs.fCaps = caps;
    pipelineArgs.fDstProxyView = dstProxyView;
    pipelineArgs.fWriteSwizzle = writeViewSwizzle;

    return arena->make<GrPipeline>(pipelineArgs,
                                   std::move(processorSet),
                                   std::move(appliedClip));
}

const GrPipeline* GrSimpleMeshDrawOpHelper::CreatePipeline(
                                                GrOpFlushState* flushState,
                                                GrProcessorSet&& processorSet,
                                                GrPipeline::InputFlags pipelineFlags) {
    return CreatePipeline(&flushState->caps(),
                          flushState->allocator(),
                          flushState->writeView().swizzle(),
                          flushState->detachAppliedClip(),
                          flushState->dstProxyView(),
                          std::move(processorSet),
                          pipelineFlags);
}

const GrPipeline* GrSimpleMeshDrawOpHelper::createPipeline(GrOpFlushState* flushState) {
    return CreatePipeline(&flushState->caps(),
                          flushState->allocator(),
                          flushState->writeView().swizzle(),
                          flushState->detachAppliedClip(),
                          flushState->dstProxyView(),
                          this->detachProcessorSet(),
                          this->pipelineFlags());
}

const GrPipeline* GrSimpleMeshDrawOpHelper::createPipeline(
        const GrCaps* caps,
        SkArenaAlloc* arena,
        GrSwizzle writeViewSwizzle,
        GrAppliedClip&& appliedClip,
        const GrDstProxyView& dstProxyView) {
    return GrSimpleMeshDrawOpHelper::CreatePipeline(caps,
                                                    arena,
                                                    writeViewSwizzle,
                                                    std::move(appliedClip),
                                                    dstProxyView,
                                                    this->detachProcessorSet(),
                                                    this->pipelineFlags());
}

GrProgramInfo* GrSimpleMeshDrawOpHelper::CreateProgramInfo(
            const GrCaps* caps,
            SkArenaAlloc* arena,
            const GrSurfaceProxyView& writeView,
            bool usesMSAASurface,
            GrAppliedClip&& appliedClip,
            const GrDstProxyView& dstProxyView,
            GrGeometryProcessor* geometryProcessor,
            GrProcessorSet&& processorSet,
            GrPrimitiveType primitiveType,
            GrXferBarrierFlags renderPassXferBarriers,
            GrLoadOp colorLoadOp,
            GrPipeline::InputFlags pipelineFlags,
            const GrUserStencilSettings* stencilSettings) {
    auto pipeline = CreatePipeline(caps,
                                   arena,
                                   writeView.swizzle(),
                                   std::move(appliedClip),
                                   dstProxyView,
                                   std::move(processorSet),
                                   pipelineFlags);

    return CreateProgramInfo(caps, arena, pipeline, writeView, usesMSAASurface, geometryProcessor,
                             primitiveType, renderPassXferBarriers, colorLoadOp, stencilSettings);
}

GrProgramInfo* GrSimpleMeshDrawOpHelper::CreateProgramInfo(const GrCaps* caps,
                                                           SkArenaAlloc* arena,
                                                           const GrPipeline* pipeline,
                                                           const GrSurfaceProxyView& writeView,
                                                           bool usesMSAASurface,
                                                           GrGeometryProcessor* geometryProcessor,
                                                           GrPrimitiveType primitiveType,
                                                           GrXferBarrierFlags xferBarrierFlags,
                                                           GrLoadOp colorLoadOp,
                                                           const GrUserStencilSettings* stencilSettings) {
    auto tmp = arena->make<GrProgramInfo>(*caps,
                                          writeView,
                                          usesMSAASurface,
                                          pipeline,
                                          stencilSettings,
                                          geometryProcessor,
                                          primitiveType,
                                          0,
                                          xferBarrierFlags,
                                          colorLoadOp);
    return tmp;
}

GrProgramInfo* GrSimpleMeshDrawOpHelper::createProgramInfo(
                                            const GrCaps* caps,
                                            SkArenaAlloc* arena,
                                            const GrSurfaceProxyView& writeView,
                                            bool usesMSAASurface,
                                            GrAppliedClip&& appliedClip,
                                            const GrDstProxyView& dstProxyView,
                                            GrGeometryProcessor* gp,
                                            GrPrimitiveType primType,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp) {
    return CreateProgramInfo(caps,
                             arena,
                             writeView,
                             usesMSAASurface,
                             std::move(appliedClip),
                             dstProxyView,
                             gp,
                             this->detachProcessorSet(),
                             primType,
                             renderPassXferBarriers,
                             colorLoadOp,
                             this->pipelineFlags());
}

#if GR_TEST_UTILS
static void dump_pipeline_flags(GrPipeline::InputFlags flags, SkString* result) {
    if (GrPipeline::InputFlags::kNone != flags) {
        if (flags & GrPipeline::InputFlags::kSnapVerticesToPixelCenters) {
            result->append("Snap vertices to pixel center.\n");
        }
        if (flags & GrPipeline::InputFlags::kWireframe) {
            result->append("Wireframe enabled.\n");
        }
        if (flags & GrPipeline::InputFlags::kConservativeRaster) {
            result->append("Conservative raster enabled.\n");
        }
        return;
    }
    result->append("No pipeline flags\n");
}

SkString GrSimpleMeshDrawOpHelper::dumpInfo() const {
    const GrProcessorSet& processors = fProcessors ? *fProcessors : GrProcessorSet::EmptySet();
    SkString result = processors.dumpProcessors();
    result.append("AA Type: ");
    switch (this->aaType()) {
        case GrAAType::kNone:
            result.append(" none\n");
            break;
        case GrAAType::kCoverage:
            result.append(" coverage\n");
            break;
        case GrAAType::kMSAA:
            result.append(" msaa\n");
            break;
    }
    dump_pipeline_flags(fPipelineFlags, &result);
    return result;
}
#endif
