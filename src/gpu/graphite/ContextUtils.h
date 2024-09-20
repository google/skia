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
#include "src/gpu/graphite/PipelineData.h"

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
class Geometry;
class GraphicsPipelineDesc;
class PaintParams;
class PipelineDataGatherer;
class Recorder;
struct RenderPassDesc;
class RenderStep;
class RuntimeEffectDictionary;
class ShaderNode;
class UniformManager;
class UniquePaintParamsID;

struct ResourceBindingRequirements;

struct VertSkSLInfo {
    std::string fSkSL;
    std::string fLabel;

    bool fHasStepUniforms = false;
};

struct FragSkSLInfo {
    std::string fSkSL;
    std::string fLabel;

    // This represents the HW blending of the final program, and not the logical blending that was
    // defined on the SkPaint.
    BlendInfo fBlendInfo;
    DstReadRequirement fDstReadReq = DstReadRequirement::kNone;

    bool fRequiresLocalCoords = false;
    int  fNumTexturesAndSamplers = 0;
    bool fHasPaintUniforms = false;
    bool fHasGradientBuffer = false;
    // Note that fData is currently only used to store SamplerDesc information for shaders that have
    // the option of using immutable samplers. However, other snippets could leverage this field to
    // convey other information once data can be tied to snippetIDs (b/347072931).
    skia_private::TArray<uint32_t> fData = {};
};

std::tuple<UniquePaintParamsID, UniformDataBlock, TextureDataBlock> ExtractPaintData(
        Recorder*,
        PipelineDataGatherer* gatherer,
        PaintParamsKeyBuilder* builder,
        const Layout layout,
        const SkM44& local2Dev,
        const PaintParams&,
        const Geometry& geometry,
        sk_sp<TextureProxy> dstTexture,
        SkIPoint dstOffset,
        const SkColorInfo& targetColorInfo);

std::tuple<UniformDataBlock, TextureDataBlock> ExtractRenderStepData(
        UniformDataCache* uniformDataCache,
        TextureDataCache* textureDataCache,
        PipelineDataGatherer* gatherer,
        const Layout layout,
        const RenderStep* step,
        const DrawParams& params);

// `viewport` should hold the actual viewport set as backend state (defining the NDC -> pixel
// transform). The viewport's dimensions are used to define the SkDevice->NDC transform applied in
// the vertex shader, but this assumes that the (0,0) device coordinate maps to the corner of the
// top-left of the NDC cube. The viewport's origin is used in the fragment shader to reconstruct
// the logical fragment coordinate from the target's current frag coord (which are not relative to
// active viewport).
//
// It is assumed that `dstCopyBounds` is in the same coordinate space as the `viewport` (e.g.
// final backing target's pixel coords) and that its width and height match the dimensions of the
// texture to be sampled for dst reads.
void CollectIntrinsicUniforms(
        const Caps* caps,
        SkIRect viewport,
        SkIRect dstCopyBounds,
        UniformManager*);

DstReadRequirement GetDstReadRequirement(const Caps*, std::optional<SkBlendMode>, Coverage);

VertSkSLInfo BuildVertexSkSL(const ResourceBindingRequirements&,
                             const RenderStep* step,
                             bool defineShadingSsboIndexVarying,
                             bool defineLocalCoordsVarying);

// TODO(b/347072931): Refactor to return std::unique_ptr<ShaderInfo> instead such that snippet
// data can remain tied to its snippet ID.
FragSkSLInfo BuildFragmentSkSL(const Caps* caps,
                               const ShaderCodeDictionary*,
                               const RuntimeEffectDictionary*,
                               const RenderStep* renderStep,
                               UniquePaintParamsID paintID,
                               bool useStorageBuffers,
                               skgpu::Swizzle writeSwizzle);

std::string GetPipelineLabel(const ShaderCodeDictionary*,
                             const RenderPassDesc& renderPassDesc,
                             const RenderStep* renderStep,
                             UniquePaintParamsID paintID);

std::string BuildComputeSkSL(const Caps*, const ComputeStep*);

std::string EmitPaintParamsUniforms(int bufferID,
                                    const Layout layout,
                                    SkSpan<const ShaderNode*> nodes,
                                    bool* hasUniforms,
                                    bool* wrotePaintColor);
std::string EmitRenderStepUniforms(int bufferID,
                                   const Layout layout,
                                   SkSpan<const Uniform> uniforms);
std::string EmitPaintParamsStorageBuffer(int bufferID,
                                         SkSpan<const ShaderNode*> nodes,
                                         bool* hasUniforms,
                                         bool* wrotePaintColor);
std::string EmitRenderStepStorageBuffer(int bufferID,
                                        SkSpan<const Uniform> uniforms);
std::string EmitUniformsFromStorageBuffer(const char* bufferNamePrefix,
                                          const char* ssboIndex,
                                          SkSpan<const Uniform> uniforms);
std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName);
std::string EmitTexturesAndSamplers(const ResourceBindingRequirements&,
                                    SkSpan<const ShaderNode*> nodes,
                                    int* binding);
std::string EmitSamplerLayout(const ResourceBindingRequirements&, int* binding);
std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitSsboIndicesVarying,
                         bool emitLocalCoordsVarying);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
