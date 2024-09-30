/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileMaskFilter.h"

#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/RenderPassDesc.h"
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

        RenderPassDesc coverageRenderPassDesc = RenderPassDesc::Make(caps,
                                                                     info,
                                                                     LoadOp::kClear,
                                                                     StoreOp::kStore,
                                                                     DepthStencilFlags::kDepth,
                                                                     { 0.0f, 0.0f, 0.0f, 0.0f },
                                                                     /* requiresMSAA= */ false,
                                                                     skgpu::Swizzle("a000"));

        PrecompileImageFiltersPriv::CreateBlurImageFilterPipelines(keyContext, gatherer,
                                                                   coverageRenderPassDesc,
                                                                   processCombination);
    }
};

sk_sp<PrecompileMaskFilter> PrecompileMaskFilters::Blur() {
    return sk_make_sp<PrecompileBlurMaskFilter>();
}

} // namespace skgpu::graphite
