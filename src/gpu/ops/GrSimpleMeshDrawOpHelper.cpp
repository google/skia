/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

GrSimpleMeshDrawOpHelper::GrSimpleMeshDrawOpHelper(const MakeArgs& args, GrAAType aaType,
                                                   InputFlags inputFlags)
        : fProcessors(args.fProcessorSet)
        , fPipelineFlags((GrPipeline::InputFlags)inputFlags)
        , fAAType((int)aaType)
        , fUsesLocalCoords(false)
        , fCompatibleWithCoverageAsAlpha(false) {
    SkDEBUGCODE(fDidAnalysis = false);
    SkDEBUGCODE(fMadePipeline = false);
    if (GrAATypeIsHW(aaType)) {
        fPipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
}

GrSimpleMeshDrawOpHelper::~GrSimpleMeshDrawOpHelper() {
    if (fProcessors) {
        fProcessors->~GrProcessorSet();
    }
}

GrDrawOp::FixedFunctionFlags GrSimpleMeshDrawOpHelper::fixedFunctionFlags() const {
    return GrAATypeIsHW((this->aaType())) ? GrDrawOp::FixedFunctionFlags::kUsesHWAA
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
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
        SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(
            caps, clip, hasMixedSampledCoverage, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = !geometryColor->fitsInBytes();
    }
    return result;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelper::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, const GrUserStencilSettings* userStencil,
        bool hasMixedSampledCoverage, GrClampType clampType,
        GrProcessorAnalysisCoverage geometryCoverage, GrProcessorAnalysisColor* geometryColor) {
    SkDEBUGCODE(fDidAnalysis = true);
    GrProcessorSet::Analysis analysis;
    if (fProcessors) {
        GrProcessorAnalysisCoverage coverage = geometryCoverage;
        if (GrProcessorAnalysisCoverage::kNone == coverage) {
            coverage = clip->numClipCoverageFragmentProcessors()
                               ? GrProcessorAnalysisCoverage::kSingleChannel
                               : GrProcessorAnalysisCoverage::kNone;
        }
        SkPMColor4f overrideColor;
        analysis = fProcessors->finalize(*geometryColor, coverage, clip, userStencil,
                                         hasMixedSampledCoverage, caps, clampType, &overrideColor);
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
                                                    GrOpFlushState* flushState,
                                                    GrProcessorSet&& processorSet,
                                                    GrPipeline::InputFlags pipelineFlags,
                                                    const GrUserStencilSettings* stencilSettings) {
    GrPipeline::InitArgs pipelineArgs;

    pipelineArgs.fInputFlags = pipelineFlags;
    pipelineArgs.fDstProxyView = flushState->dstProxyView();
    pipelineArgs.fCaps = &flushState->caps();
    pipelineArgs.fUserStencil = stencilSettings;
    pipelineArgs.fOutputSwizzle = flushState->view()->swizzle();

    return flushState->allocator()->make<GrPipeline>(pipelineArgs,
                                                     std::move(processorSet),
                                                     flushState->detachAppliedClip());
}

#ifdef SK_DEBUG
static void dump_pipeline_flags(GrPipeline::InputFlags flags, SkString* result) {
    if (GrPipeline::InputFlags::kNone != flags) {
        if (flags & GrPipeline::InputFlags::kSnapVerticesToPixelCenters) {
            result->append("Snap vertices to pixel center.\n");
        }
        if (flags & GrPipeline::InputFlags::kHWAntialias) {
            result->append("HW Antialiasing enabled.\n");
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

GrSimpleMeshDrawOpHelperWithStencil::GrSimpleMeshDrawOpHelperWithStencil(
        const MakeArgs& args, GrAAType aaType, const GrUserStencilSettings* stencilSettings,
        InputFlags inputFlags)
        : INHERITED(args, aaType, inputFlags)
        , fStencilSettings(stencilSettings ? stencilSettings : &GrUserStencilSettings::kUnused) {}

GrDrawOp::FixedFunctionFlags GrSimpleMeshDrawOpHelperWithStencil::fixedFunctionFlags() const {
    GrDrawOp::FixedFunctionFlags flags = INHERITED::fixedFunctionFlags();
    if (fStencilSettings != &GrUserStencilSettings::kUnused) {
        flags |= GrDrawOp::FixedFunctionFlags::kUsesStencil;
    }
    return flags;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelperWithStencil::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
        SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(
            caps, clip, hasMixedSampledCoverage, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = !geometryColor->fitsInBytes();
    }
    return result;
}

bool GrSimpleMeshDrawOpHelperWithStencil::isCompatible(
        const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps& caps,
        const SkRect& thisBounds, const SkRect& thatBounds, bool ignoreAAType) const {
    return INHERITED::isCompatible(that, caps, thisBounds, thatBounds, ignoreAAType) &&
           fStencilSettings == that.fStencilSettings;
}

#ifdef SK_DEBUG
SkString GrSimpleMeshDrawOpHelperWithStencil::dumpInfo() const {
    SkString result = INHERITED::dumpInfo();
    result.appendf("Stencil settings: %s\n", (fStencilSettings ? "yes" : "no"));
    return result;
}
#endif
