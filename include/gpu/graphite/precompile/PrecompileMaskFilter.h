/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileMaskFilter_DEFINED
#define skgpu_graphite_precompile_PrecompileMaskFilter_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include "include/gpu/graphite/precompile/PaintOptions.h"

namespace skgpu::graphite {

/** \class PrecompileMaskFilter
    This class corresponds to the SkMaskFilter class in the main API.
*/
class SK_API PrecompileMaskFilter : public PrecompileBase {
protected:
    friend class PaintOptions;  // for createPipelines() access

    PrecompileMaskFilter() : PrecompileBase(Type::kMaskFilter) {}
    ~PrecompileMaskFilter() override;

    void addToKey(const KeyContext&,
                  PaintParamsKeyBuilder*,
                  PipelineDataGatherer*,
                  int desiredCombination) const final;

    virtual void createPipelines(const KeyContext&,
                                 PipelineDataGatherer*,
                                 const PaintOptions&,
                                 const RenderPassDesc&,
                                 const PaintOptions::ProcessCombination&) const = 0;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkMaskFilter class' factories in the main API
namespace PrecompileMaskFilters {

    // This corresponds to the SkMaskFilter::MakeBlur factory in the main Skia API.
    // The specific details of the blur have been elided since they don't impact the generated
    // shader.
    SK_API sk_sp<PrecompileMaskFilter> Blur();

} // namespace PrecompileMaskFilters

} // namespace skgpu::graphite


#endif // skgpu_graphite_precompile_PrecompileMaskFilter_DEFINED
