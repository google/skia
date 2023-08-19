/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextUtils_DEFINED
#define skgpu_graphite_ContextUtils_DEFINED

#include "src/gpu/Blend.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineDataCache.h"

#include <optional>
#include <tuple>

class SkColorInfo;
class SkM44;
class SkPaint;

namespace skgpu {
class Swizzle;
}

namespace skgpu::graphite {

class ComputeStep;
enum class Coverage;
class DrawParams;
enum class DstReadRequirement;
class GraphicsPipelineDesc;
class PaintParams;
class PipelineDataGatherer;
class Recorder;
class RenderStep;
class RuntimeEffectDictionary;
class ShaderNode;
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
                 sk_sp<TextureProxy> dstTexture,
                 SkIPoint dstOffset,
                 const SkColorInfo& targetColorInfo);

std::tuple<const UniformDataBlock*, const TextureDataBlock*> ExtractRenderStepData(
        UniformDataCache* uniformDataCache,
        TextureDataCache* textureDataCache,
        PipelineDataGatherer* gatherer,
        const Layout layout,
        const RenderStep* step,
        const DrawParams& params);

DstReadRequirement GetDstReadRequirement(const Caps*, std::optional<SkBlendMode>, Coverage);

std::string BuildVertexSkSL(const ResourceBindingRequirements&,
                            const RenderStep* step,
                            bool defineShadingSsboIndexVarying,
                            bool defineLocalCoordsVarying);

FragSkSLInfo BuildFragmentSkSL(const Caps* caps,
                               const ShaderCodeDictionary*,
                               const RuntimeEffectDictionary*,
                               const RenderStep* renderStep,
                               UniquePaintParamsID paintID,
                               bool useStorageBuffers,
                               skgpu::Swizzle writeSwizzle);

std::string BuildComputeSkSL(const Caps*, const ComputeStep*);

std::string EmitPaintParamsUniforms(int bufferID,
                                    const char* name,
                                    const Layout layout,
                                    SkSpan<const ShaderNode*> nodes);
std::string EmitRenderStepUniforms(int bufferID,
                                   const char* name,
                                   const Layout layout,
                                   SkSpan<const Uniform> uniforms);
std::string EmitPaintParamsStorageBuffer(int bufferID,
                                         const char* bufferTypePrefix,
                                         const char* bufferNamePrefix,
                                         SkSpan<const ShaderNode*> nodes);
std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName);
std::string EmitTexturesAndSamplers(const ResourceBindingRequirements&,
                                    SkSpan<const ShaderNode*> nodes,
                                    int* binding);
std::string EmitSamplerLayout(const ResourceBindingRequirements&, int* binding);
std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitShadingSsboIndexVarying,
                         bool emitLocalCoordsVarying);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
