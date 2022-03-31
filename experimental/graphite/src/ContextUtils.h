/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextUtils_DEFINED
#define skgpu_ContextUtils_DEFINED

#include "experimental/graphite/src/PipelineDataCache.h"

class SkPaintParamsKeyBuilder;
class SkPipelineDataGatherer;
class SkUniquePaintParamsID;

namespace skgpu {

class DrawGeometry;
class PaintParams;
class Recorder;
class RenderStep;

std::tuple<SkUniquePaintParamsID, UniformDataCache::Index, TextureDataCache::Index>
ExtractPaintData(Recorder*,
                 SkPipelineDataGatherer* gatherer,
                 SkPaintParamsKeyBuilder* builder,
                 const PaintParams&);

UniformDataCache::Index ExtractRenderStepData(UniformDataCache* geometryUniformDataCache,
                                              SkPipelineDataGatherer* gatherer,
                                              const RenderStep* step,
                                              const DrawGeometry& geometry);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
