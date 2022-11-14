/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextUtils_DEFINED
#define skgpu_graphite_ContextUtils_DEFINED

#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineDataCache.h"

class SkM44;
class SkPipelineDataGatherer;
class SkRuntimeEffectDictionary;
class SkUniquePaintParamsID;

namespace skgpu::graphite {

class DrawParams;
class GraphicsPipelineDesc;
class PaintParams;
class Recorder;
class RenderStep;

std::tuple<SkUniquePaintParamsID, const SkUniformDataBlock*, const SkTextureDataBlock*>
ExtractPaintData(Recorder*,
                 SkPipelineDataGatherer* gatherer,
                 PaintParamsKeyBuilder* builder,
                 const SkM44& local2Dev,
                 const PaintParams&);

std::tuple<const SkUniformDataBlock*, const SkTextureDataBlock*>
ExtractRenderStepData(UniformDataCache* uniformDataCache,
                      TextureDataCache* textureDataCache,
                      SkPipelineDataGatherer* gatherer,
                      const RenderStep* step,
                      const DrawParams& params);

std::string GetSkSLVS(const RenderStep* step,
                      bool defineShadingSsboIndexVarying,
                      bool defineLocalCoordsVarying);

std::string GetSkSLFS(const ShaderCodeDictionary*,
                      const SkRuntimeEffectDictionary*,
                      const RenderStep* renderStep,
                      SkUniquePaintParamsID paintID,
                      bool useStorageBuffers,
                      BlendInfo* blendInfo,
                      bool* requiresLocalCoordsVarying);

std::string EmitPaintParamsUniforms(int bufferID,
                                    const char* name,
                                    const std::vector<PaintParamsKey::BlockReader>&);
std::string EmitRenderStepUniforms(int bufferID, const char* name,
                                   SkSpan<const SkUniform> uniforms);
std::string EmitPaintParamsStorageBuffer(int bufferID,
                                         const char* bufferTypePrefix,
                                         const char* bufferNamePrefix,
                                         const std::vector<PaintParamsKey::BlockReader>& readers);
std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName);
std::string EmitTexturesAndSamplers(const std::vector<PaintParamsKey::BlockReader>&,
                                    int* binding);
std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitShadingSsboIndexVarying,
                         bool emitLocalCoordsVarying);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
