/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextUtils_DEFINED
#define skgpu_graphite_ContextUtils_DEFINED

#include "src/gpu/graphite/PipelineDataCache.h"

class SkM44;
class SkPaintParamsKeyBuilder;
class SkPipelineDataGatherer;
class SkUniquePaintParamsID;

namespace skgpu::graphite {

class DrawParams;
class PaintParams;
class Recorder;
class RenderStep;

std::tuple<SkUniquePaintParamsID, UniformDataCache::Index, TextureDataCache::Index>
ExtractPaintData(Recorder*,
                 SkPipelineDataGatherer* gatherer,
                 SkPaintParamsKeyBuilder* builder,
                 const SkM44& dev2local,
                 const PaintParams&);

UniformDataCache::Index ExtractRenderStepData(UniformDataCache* geometryUniformDataCache,
                                              SkPipelineDataGatherer* gatherer,
                                              const RenderStep* step,
                                              const DrawParams& params);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
