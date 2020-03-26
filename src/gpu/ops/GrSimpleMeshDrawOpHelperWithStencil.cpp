/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.h"

const GrPipeline* GrSimpleMeshDrawOpHelperWithStencil::createPipelineWithStencil(
                                            const GrCaps* caps,
                                            SkArenaAlloc* arena,
                                            GrSwizzle outputViewSwizzle,
                                            GrAppliedClip&& appliedClip,
                                            const GrXferProcessor::DstProxyView& dstProxyView) {
    return GrSimpleMeshDrawOpHelper::CreatePipeline(caps,
                                                    arena,
                                                    outputViewSwizzle,
                                                    std::move(appliedClip),
                                                    dstProxyView,
                                                    this->detachProcessorSet(),
                                                    this->pipelineFlags(),
                                                    this->stencilSettings());
}

const GrPipeline* GrSimpleMeshDrawOpHelperWithStencil::createPipelineWithStencil(
        GrOpFlushState* flushState) {
    return this->createPipelineWithStencil(&flushState->caps(),
                                           flushState->allocator(),
                                           flushState->outputView()->swizzle(),
                                           flushState->detachAppliedClip(),
                                           flushState->dstProxyView());
}

GrSimpleMeshDrawOpHelperWithStencil::GrSimpleMeshDrawOpHelperWithStencil(
                                                    const MakeArgs& args,
                                                    GrAAType aaType,
                                                    const GrUserStencilSettings* stencilSettings,
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

GrProgramInfo* GrSimpleMeshDrawOpHelperWithStencil::createProgramInfoWithStencil(
                                            const GrCaps* caps,
                                            SkArenaAlloc* arena,
                                            const GrSurfaceProxyView* outputView,
                                            GrAppliedClip&& appliedClip,
                                            const GrXferProcessor::DstProxyView& dstProxyView,
                                            GrGeometryProcessor* gp,
                                            GrPrimitiveType primType) {
    return CreateProgramInfo(caps,
                             arena,
                             outputView,
                             std::move(appliedClip),
                             dstProxyView,
                             gp,
                             this->detachProcessorSet(),
                             primType,
                             this->pipelineFlags(),
                             this->stencilSettings());
}

#ifdef SK_DEBUG
SkString GrSimpleMeshDrawOpHelperWithStencil::dumpInfo() const {
    SkString result = INHERITED::dumpInfo();
    result.appendf("Stencil settings: %s\n", (fStencilSettings ? "yes" : "no"));
    return result;
}
#endif
