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
    using InputFlags = GrSimpleMeshDrawOpHelper::InputFlags;

    using GrSimpleMeshDrawOpHelper::visitProxies;
    using GrSimpleMeshDrawOpHelper::createPipeline;

    GrProgramInfo* createProgramInfoWithStencil(const GrCaps*,
                                                SkArenaAlloc*,
                                                const GrSurfaceProxyView& writeView,
                                                bool usesMSAASurface,
                                                GrAppliedClip&&,
                                                const GrDstProxyView&,
                                                GrGeometryProcessor*,
                                                GrPrimitiveType,
                                                GrXferBarrierFlags renderPassXferBarriers,
                                                GrLoadOp colorLoadOp);

    // using declarations can't be templated, so this is a pass through function instead.
    template <typename Op, typename... OpArgs>
    static GrOp::Owner FactoryHelper(GrRecordingContext* context, GrPaint&& paint,
                                     OpArgs... opArgs) {
        return GrSimpleMeshDrawOpHelper::FactoryHelper<Op, OpArgs...>(
                context, std::move(paint), std::forward<OpArgs>(opArgs)...);
    }

    GrSimpleMeshDrawOpHelperWithStencil(GrProcessorSet*, GrAAType, const GrUserStencilSettings*,
                                        InputFlags = InputFlags::kNone);

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    GrProcessorSet::Analysis finalizeProcessors(const GrCaps& caps, const GrAppliedClip* clip,
                                                GrClampType clampType,
                                                GrProcessorAnalysisCoverage geometryCoverage,
                                                GrProcessorAnalysisColor* geometryColor) {
        return this->INHERITED::finalizeProcessors(caps, clip, fStencilSettings, clampType,
                                                   geometryCoverage, geometryColor);
    }

    GrProcessorSet::Analysis finalizeProcessors(const GrCaps&, const GrAppliedClip*, GrClampType,
                                                GrProcessorAnalysisCoverage geometryCoverage,
                                                SkPMColor4f* geometryColor, bool* wideColor);

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

#if GR_TEST_UTILS
    SkString dumpInfo() const;
#endif

    const GrUserStencilSettings* stencilSettings() const { return fStencilSettings; }

private:
    const GrUserStencilSettings* fStencilSettings;
    using INHERITED = GrSimpleMeshDrawOpHelper;
};

#endif
