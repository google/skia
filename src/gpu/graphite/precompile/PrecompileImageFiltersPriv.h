/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED

#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

class KeyContext;
class PipelineDataGatherer;

namespace PrecompileImageFiltersPriv {

    // Used by both the BlurMaskFilter and the BlurImageFilter
    void CreateBlurImageFilterPipelines(const KeyContext&,
                                        PipelineDataGatherer*,
                                        const RenderPassDesc&,
                                        const PaintOptionsPriv::ProcessCombination&);

} // namespace PrecompileImageFiltersPriv

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileImageFiltersPriv_DEFINED
