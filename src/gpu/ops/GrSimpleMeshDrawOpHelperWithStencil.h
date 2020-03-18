/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMeshDrawOpHelperWithStencil_DEFINED
#define GrSimpleMeshDrawOpHelperWithStencil_DEFINED

#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

/**
 * This class extends GrSimpleMeshDrawOpHelper to support an optional GrUserStencilSettings. This
 * uses private inheritance because it non-virtually overrides methods in the base class and should
 * never be used with a GrSimpleMeshDrawOpHelper pointer or reference.
 */
class GrSimpleMeshDrawOpHelperWithStencil : private GrSimpleMeshDrawOpHelper {
public:
    using MakeArgs = GrSimpleMeshDrawOpHelper::MakeArgs;
    using InputFlags = GrSimpleMeshDrawOpHelper::InputFlags;

    using GrSimpleMeshDrawOpHelper::visitProxies;

    const GrPipeline* createPipelineWithStencil(const GrCaps*,
                                                SkArenaAlloc*,
                                                GrSwizzle outputViewSwizzle,
                                                GrAppliedClip&&,
                                                const GrXferProcessor::DstProxyView&);

    const GrPipeline* createPipelineWithStencil(GrOpFlushState* flushState);

    GrProgramInfo* createProgramInfoWithStencil(const GrCaps*,
                                                SkArenaAlloc*,
                                                const GrSurfaceProxyView* outputView,
                                                GrAppliedClip&&,
                                                const GrXferProcessor::DstProxyView&,
                                                GrGeometryProcessor*,
                                                GrPrimitiveType);


    // using declarations can't be templated, so this is a pass through function instead.
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrRecordingContext* context, GrPaint&& paint,
                                                   OpArgs... opArgs) {
        return GrSimpleMeshDrawOpHelper::FactoryHelper<Op, OpArgs...>(
                context, std::move(paint), std::forward<OpArgs>(opArgs)...);
    }

    GrSimpleMeshDrawOpHelperWithStencil(const MakeArgs&, GrAAType, const GrUserStencilSettings*,
                                        InputFlags = InputFlags::kNone);

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
            GrProcessorAnalysisColor* geometryColor) {
        return this->INHERITED::finalizeProcessors(
                caps, clip, fStencilSettings, hasMixedSampledCoverage, clampType, geometryCoverage,
                geometryColor);
    }

    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps&, const GrAppliedClip*, bool hasMixedSampledCoverage, GrClampType,
            GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor, bool*
            wideColor);

    using GrSimpleMeshDrawOpHelper::aaType;
    using GrSimpleMeshDrawOpHelper::setAAType;
    using GrSimpleMeshDrawOpHelper::isTrivial;
    using GrSimpleMeshDrawOpHelper::usesLocalCoords;
    using GrSimpleMeshDrawOpHelper::compatibleWithCoverageAsAlpha;
    using GrSimpleMeshDrawOpHelper::detachProcessorSet;
    using GrSimpleMeshDrawOpHelper::pipelineFlags;

    bool isCompatible(const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps&,
                      const SkRect& thisBounds, const SkRect& thatBounds,
                      bool ignoreAAType = false) const;

#ifdef SK_DEBUG
    SkString dumpInfo() const;
#endif

    const GrUserStencilSettings* stencilSettings() const { return fStencilSettings; }

private:
    const GrUserStencilSettings* fStencilSettings;
    typedef GrSimpleMeshDrawOpHelper INHERITED;
};

#endif
