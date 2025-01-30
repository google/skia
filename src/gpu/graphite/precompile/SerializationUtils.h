/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_SerializationUtils_DEFINED
#define skgpu_graphite_precompile_SerializationUtils_DEFINED

#include "include/core/SkTypes.h"

class SkStream;
class SkWStream;

namespace skgpu::graphite {

class Caps;
class GraphicsPipelineDesc;
class RendererProvider;
struct RenderPassDesc;
class ShaderCodeDictionary;

// These are the top-level entry points to serialize Pipeline data for the Android-style
// Precompilation API
[[nodiscard]] bool SerializePipelineDesc(ShaderCodeDictionary*,
                                         SkWStream*,
                                         const GraphicsPipelineDesc&,
                                         const RenderPassDesc&);
[[nodiscard]] bool DeserializePipelineDesc(const Caps*,
                                           ShaderCodeDictionary*,
                                           SkStream*,
                                           GraphicsPipelineDesc*,
                                           RenderPassDesc*);

} // skgpu::graphite

#endif // skgpu_graphite_precompile_SerializationUtils_DEFINED
