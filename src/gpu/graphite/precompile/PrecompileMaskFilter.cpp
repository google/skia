/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileMaskFilter.h"

#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/InternalDrawTypeFlags.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/precompile/PrecompileImageFiltersPriv.h"

namespace skgpu::graphite {

PrecompileMaskFilter::~PrecompileMaskFilter() = default;

// The PrecompileMaskFilter-derived classes do not use the PrecompileBase::addToKey virtual since
// they, in general, do not themselves contribute to a given SkPaint/Pipeline but, rather,
// create separate SkPaints/Pipelines from whole cloth (in createPipelines).
void PrecompileMaskFilter::addToKey(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    int desiredCombination) const {
    SkASSERT(false);
}

//--------------------------------------------------------------------------------------------------
// TODO(b/342413572): the analytic blurmasks are triggered off of the simple DrawType thus
// over-generate when a simple draw doesn't have a blur mask.
class PrecompileBlurMaskFilter : public PrecompileMaskFilter {
public:
    PrecompileBlurMaskFilter() {}

private:
    void createPipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptions& paintOptions,
            const RenderPassDesc& renderPassDescIn,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {
        const Caps* caps = keyContext.caps();
        // TODO: pull Protected-ness from 'renderPassDescIn'
        TextureInfo info = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType,
                                                              Mipmapped::kNo,
                                                              Protected::kNo,
                                                              Renderable::kYes);

        RenderPassDesc coverageRenderPassDesc = RenderPassDesc::Make(
                caps,
                info,
                LoadOp::kClear,
                StoreOp::kStore,
                DepthStencilFlags::kDepth,
                { 0.0f, 0.0f, 0.0f, 0.0f },
                /* requiresMSAA= */ false,
                skgpu::Swizzle("a000"),
                caps->getDstReadStrategy());

        PrecompileImageFiltersPriv::CreateBlurImageFilterPipelines(keyContext, gatherer,
                                                                   coverageRenderPassDesc,
                                                                   processCombination);

        // c.f. AutoLayerForImageFilter::addMaskFilterLayer. The following PaintOptions handle
        // the case where an explicit layer must be created.
        {
            // The restore draw takes over all the shading effects. The mask filter blur will have
            // been converted to an image filter applied to the coverage layer. That coverage
            // will then be used as the coverage mask for the restoreOptions.
            PaintOptions restoreOptions = paintOptions;
            restoreOptions.setMaskFilters({});
            restoreOptions.priv().buildCombinations(
                    keyContext,
                    gatherer,
                    static_cast<DrawTypeFlags>(InternalDrawTypeFlags::kCoverageMask),
                    /* withPrimitiveBlender= */ false,
                    Coverage::kSingleChannel,
                    renderPassDescIn,
                    processCombination);
        }

        {
            // The initial draw into the coverage layer is just a solid white kSrcOver SkPaint
            // These options cover the case where the coverage draw can be done with the
            // AnalyticRRect RenderStep.
            // TODO: gate the inclusion of this option on the drawType being kSimple
            PaintOptions coverageOptions;
            coverageOptions.setShaders({ PrecompileShaders::Color() });
            coverageOptions.setBlendModes({ SkBlendMode::kSrcOver });

            coverageOptions.priv().buildCombinations(
                    keyContext,
                    gatherer,
                    DrawTypeFlags::kAnalyticRRect,
                    /* withPrimitiveBlender= */ false,
                    Coverage::kSingleChannel,
                    coverageRenderPassDesc,
                    processCombination);
        }
    }
};

sk_sp<PrecompileMaskFilter> PrecompileMaskFilters::Blur() {
    return sk_make_sp<PrecompileBlurMaskFilter>();
}

} // namespace skgpu::graphite
