/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSimpleMeshDrawOpHelper.h"
#include "GrAppliedClip.h"
#include "GrProcessorSet.h"
#include "GrRect.h"
#include "GrUserStencilSettings.h"
#include "SkGr.h"

GrSimpleMeshDrawOpHelper::GrSimpleMeshDrawOpHelper(const MakeArgs& args, GrAAType aaType,
                                                   Flags flags)
        : fProcessors(args.fProcessorSet)
        , fPipelineFlags(0)
        , fAAType((int)aaType)
        , fUsesLocalCoords(false)
        , fCompatibleWithCoverageAsAlpha(false) {
    SkDEBUGCODE(fDidAnalysis = false);
    SkDEBUGCODE(fMadePipeline = false);
    if (GrAATypeIsHW(aaType)) {
        fPipelineFlags |= GrPipeline::kHWAntialias_Flag;
    }
    if (flags & Flags::kSnapVerticesToPixelCenters) {
        fPipelineFlags |= GrPipeline::kSnapVerticesToPixelCenters_Flag;
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

static bool none_as_coverage_aa_compatible(GrAAType aa1, GrAAType aa2) {
    return (aa1 == GrAAType::kNone && aa2 == GrAAType::kCoverage) ||
           (aa1 == GrAAType::kCoverage && aa2 == GrAAType::kNone);
}

bool GrSimpleMeshDrawOpHelper::isCompatible(const GrSimpleMeshDrawOpHelper& that,
                                            const GrCaps& caps, const SkRect& thisBounds,
                                            const SkRect& thatBounds, bool noneAsCoverageAA) const {
    if (SkToBool(fProcessors) != SkToBool(that.fProcessors)) {
        return false;
    }
    if (fProcessors) {
        if (*fProcessors != *that.fProcessors) {
            return false;
        }
    }
    bool result = fPipelineFlags == that.fPipelineFlags && (fAAType == that.fAAType ||
            (noneAsCoverageAA && none_as_coverage_aa_compatible(this->aaType(), that.aaType())));
    SkASSERT(!result || fCompatibleWithCoverageAsAlpha == that.fCompatibleWithCoverageAsAlpha);
    SkASSERT(!result || fUsesLocalCoords == that.fUsesLocalCoords);
    return result;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelper::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType, GrClampType clampType,
        GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(
            caps, clip, fsaaType, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = SkPMColor4fNeedsWideColor(*geometryColor, clampType, caps);
    }
    return result;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelper::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, const GrUserStencilSettings* userStencil,
        GrFSAAType fsaaType, GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
        GrProcessorAnalysisColor* geometryColor) {
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
        analysis = fProcessors->finalize(*geometryColor, coverage, clip, userStencil, fsaaType,
                                         caps, clampType, &overrideColor);
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

void GrSimpleMeshDrawOpHelper::executeDrawsAndUploads(
        const GrOp* op, GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fProcessors) {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                op, chainBounds, std::move(*fProcessors), fPipelineFlags);
    } else {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                op, chainBounds, GrProcessorSet::MakeEmptySet(), fPipelineFlags);
    }
}

#ifdef SK_DEBUG
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
        case GrAAType::kMixedSamples:
            result.append(" mixed samples\n");
            break;
    }
    result.append(GrPipeline::DumpFlags(fPipelineFlags));
    return result;
}
#endif

GrSimpleMeshDrawOpHelperWithStencil::GrSimpleMeshDrawOpHelperWithStencil(
        const MakeArgs& args, GrAAType aaType, const GrUserStencilSettings* stencilSettings,
        Flags flags)
        : INHERITED(args, aaType, flags)
        , fStencilSettings(stencilSettings ? stencilSettings : &GrUserStencilSettings::kUnused) {}

GrDrawOp::FixedFunctionFlags GrSimpleMeshDrawOpHelperWithStencil::fixedFunctionFlags() const {
    GrDrawOp::FixedFunctionFlags flags = INHERITED::fixedFunctionFlags();
    if (fStencilSettings != &GrUserStencilSettings::kUnused) {
        flags |= GrDrawOp::FixedFunctionFlags::kUsesStencil;
    }
    return flags;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelperWithStencil::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType, GrClampType clampType,
        GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(
            caps, clip, fsaaType, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = SkPMColor4fNeedsWideColor(*geometryColor, clampType, caps);
    }
    return result;
}

bool GrSimpleMeshDrawOpHelperWithStencil::isCompatible(
        const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps& caps,
        const SkRect& thisBounds, const SkRect& thatBounds, bool noneAsCoverageAA) const {
    return INHERITED::isCompatible(that, caps, thisBounds, thatBounds, noneAsCoverageAA) &&
           fStencilSettings == that.fStencilSettings;
}

void GrSimpleMeshDrawOpHelperWithStencil::executeDrawsAndUploads(
        const GrOp* op, GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (fProcessors) {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                op, chainBounds, std::move(*fProcessors), fPipelineFlags, fStencilSettings);
    } else {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                op, chainBounds, GrProcessorSet::MakeEmptySet(), fPipelineFlags, fStencilSettings);
    }
}

#ifdef SK_DEBUG
SkString GrSimpleMeshDrawOpHelperWithStencil::dumpInfo() const {
    SkString result = INHERITED::dumpInfo();
    result.appendf("Stencil settings: %s\n", (fStencilSettings ? "yes" : "no"));
    return result;
}
#endif
