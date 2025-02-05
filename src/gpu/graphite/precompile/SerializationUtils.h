/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_SerializationUtils_DEFINED
#define skgpu_graphite_precompile_SerializationUtils_DEFINED

#include "include/core/SkRefCnt.h"

class SkData;

namespace skgpu::graphite {

class Caps;
class GraphicsPipelineDesc;
struct RenderPassDesc;
class ShaderCodeDictionary;

// These are the top-level entry points to serialize Pipeline data for the Android-style
// Precompilation API
[[nodiscard]] sk_sp<SkData> PipelineDescToData(ShaderCodeDictionary*,
                                               const GraphicsPipelineDesc&,
                                               const RenderPassDesc&);

[[nodiscard]] bool DataToPipelineDesc(const Caps*,
                                      ShaderCodeDictionary*,
                                      const SkData*,
                                      GraphicsPipelineDesc* pipelineDesc,
                                      RenderPassDesc* renderPassDesc);

#if defined(GPU_TEST_UTILS)
void DumpPipelineDesc(const char* label,
                      ShaderCodeDictionary*,
                      const GraphicsPipelineDesc&,
                      const RenderPassDesc&);

[[nodiscard]] bool ComparePipelineDescs(const GraphicsPipelineDesc& a1, const RenderPassDesc& b1,
                                        const GraphicsPipelineDesc& a2, const RenderPassDesc& b2);
#endif

} // skgpu::graphite

#endif // skgpu_graphite_precompile_SerializationUtils_DEFINED
