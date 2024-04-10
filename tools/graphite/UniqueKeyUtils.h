/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef UniqueKeyUtils_DEFINED
#define UniqueKeyUtils_DEFINED

#include "include/core/SkTypes.h"

#include <vector>

namespace skgpu {
    class UniqueKey;
}

namespace skgpu::graphite {
    class Context;
    class GlobalCache;
    class GraphicsPipelineDesc;
    struct RenderPassDesc;
    class RendererProvider;
    class ShaderCodeDictionary;
}

namespace UniqueKeyUtils {

void FetchUniqueKeys(skgpu::graphite::GlobalCache* globalCache,
                     std::vector<skgpu::UniqueKey>* keys);

#ifdef SK_DEBUG
void DumpDescs(const skgpu::graphite::RendererProvider*,
               const skgpu::graphite::ShaderCodeDictionary*,
               const skgpu::graphite::GraphicsPipelineDesc&,
               const skgpu::graphite::RenderPassDesc&);
#endif

// This helper breaks a UniqueKey down into its GraphicsPipelineDesc
// and a RenderPassDesc and checks that the reassembled pieces match the
// original.
bool ExtractKeyDescs(skgpu::graphite::Context*,
                     const skgpu::UniqueKey&,
                     skgpu::graphite::GraphicsPipelineDesc*,
                     skgpu::graphite::RenderPassDesc*);

}  // namespace UniqueKeyUtils

#endif  // UniqueKeyUtils_DEFINED
