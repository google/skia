/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileInternal_DEFINED
#define skgpu_graphite_PrecompileInternal_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

namespace skgpu::graphite {

enum class Coverage;
class Context;
class KeyContext;
class PaintOptions;
struct RenderPassDesc;

// Create the Pipelines specified by the paint options by combining the shading portion w/ the
// specified 'drawTypes' and RenderPassDesc.
void PrecompileCombinations(const RendererProvider*,
                            ResourceProvider*,
                            const PaintOptions&,
                            const KeyContext&,
                            DrawTypeFlags,
                            bool withPrimitiveBlender,
                            Coverage,
                            const RenderPassDesc&);

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileInternal_DEFINED
