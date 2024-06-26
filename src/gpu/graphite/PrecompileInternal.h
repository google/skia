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

// Create the Pipelines specified by 'options' by combining the shading portion w/ the specified
// 'drawTypes' and a stock set of RenderPass descriptors (e.g., kDepth+msaa, kDepthStencil+msaa)
void PrecompileCombinations(Context* context,
                            const PaintOptions& options,
                            const KeyContext& keyContext,
                            DrawTypeFlags drawTypes,
                            bool withPrimitiveBlender,
                            Coverage coverage);

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileInternal_DEFINED
