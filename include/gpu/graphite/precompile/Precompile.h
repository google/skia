/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_Precompile_DEFINED
#define skgpu_graphite_precompile_Precompile_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

namespace skgpu::graphite {

class Context;
class PaintOptions;

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
void Precompile(Context* context,
                const PaintOptions& paintOptions,
                DrawTypeFlags drawTypes = kMostCommon);

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_Precompile_DEFINED
