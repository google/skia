/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PublicPrecompile_DEFINED
#define skgpu_graphite_PublicPrecompile_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

// TODO: this header should be moved to include/gpu/graphite once the precompilation API
// is made public
namespace skgpu::graphite {

class Context;
class GraphicsPipelineDesc;
class PaintOptions;
struct RenderPassDesc;
class RuntimeEffectDictionary;

/**
 * Precompilation allows clients to create pipelines ahead of time based on what they expect
 * to draw. This can reduce performance hitches, due to inline compilation, during the actual
 * drawing. Graphite will always be able to perform an inline compilation if some SkPaint
 * combination was omitted from precompilation.
 *
 *   @param context        the Context to which the actual draws will be submitted
 *   @param paintOptions   captures a set of SkPaints that will be drawn
 *   @param drawTypes      communicates which primitives those paints will be drawn with
 */
void Precompile(Context*, const PaintOptions&, DrawTypeFlags = kMostCommon);

/*
 * TODO: Rather than passing in a pipelineDesc and renderPassDesc we need to add an
 * opaque serializable object that contains the same information.
 */
bool Precompile(Context*,
                RuntimeEffectDictionary* rteDict,
                const GraphicsPipelineDesc& pipelineDesc,
                const RenderPassDesc& renderPassDesc);

} // namespace skgpu::graphite

#endif // skgpu_graphite_PublicPrecompile_DEFINED
