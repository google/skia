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

class SkColorInfo;
class SkM44;

namespace skgpu::graphite {

class DrawParams;
class GraphicsPipelineDesc;
class PaintParams;
class PipelineDataGatherer;
class Recorder;
class RenderStep;
class RuntimeEffectDictionary;
class UniquePaintParamsID;

struct ResourceBindingRequirements;

struct FragSkSLInfo {
    std::string fSkSL;
    BlendInfo fBlendInfo;
    bool fRequiresLocalCoords = false;
    int fNumTexturesAndSamplers = 0;
};

std::tuple<UniquePaintParamsID, const UniformDataBlock*, const TextureDataBlock*>
ExtractPaintData(Recorder*,
                 PipelineDataGatherer* gatherer,
                 PaintParamsKeyBuilder* builder,
                 const Layout layout,
                 const SkM44& local2Dev,
                 const PaintParams&,
                 const SkColorInfo& targetColorInfo);

std::tuple<const UniformDataBlock*, const TextureDataBlock*> ExtractRenderStepData(
        UniformDataCache* uniformDataCache,
        TextureDataCache* textureDataCache,
        PipelineDataGatherer* gatherer,
        const Layout layout,
        const RenderStep* step,
        const DrawParams& params);

std::string GetSkSLVS(const ResourceBindingRequirements&,
                      const RenderStep* step,
                      bool defineShadingSsboIndexVarying,
                      bool defineLocalCoordsVarying);

FragSkSLInfo GetSkSLFS(const ResourceBindingRequirements&,
                       const ShaderCodeDictionary*,
                       const RuntimeEffectDictionary*,
                       const RenderStep* renderStep,
                       UniquePaintParamsID paintID,
                       bool useStorageBuffers);

std::string EmitPaintParamsUniforms(int bufferID,
                                    const char* name,
                                    const Layout layout,
                                    const std::vector<PaintParamsKey::BlockReader>&);
std::string EmitRenderStepUniforms(int bufferID,
                                   const char* name,
                                   const Layout layout,
                                   SkSpan<const Uniform> uniforms);
std::string EmitPaintParamsStorageBuffer(int bufferID,
                                         const char* bufferTypePrefix,
                                         const char* bufferNamePrefix,
                                         const std::vector<PaintParamsKey::BlockReader>& readers);
std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName);
std::string EmitTexturesAndSamplers(const ResourceBindingRequirements&,
                                    const std::vector<PaintParamsKey::BlockReader>&,
                                    int* binding);
std::string EmitSamplerLayout(const ResourceBindingRequirements&, int* binding);
std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitShadingSsboIndexVarying,
                         bool emitLocalCoordsVarying);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
