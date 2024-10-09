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

UniquePaintParamsID ExtractPaintData(Recorder*,
                                     PipelineDataGatherer* gatherer,
                                     PaintParamsKeyBuilder* builder,
                                     const Layout layout,
                                     const SkM44& local2Dev,
                                     const PaintParams&,
                                     const Geometry& geometry,
                                     const SkColorInfo& targetColorInfo);

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

// Accepts a real or, by default, an invalid/nullptr pointer to a container of SamplerDescs.
// Backend implementations which may utilize static / immutable samplers should pass in a real
// pointer to indicate that shader node data must be analyzed to determine whether
// immutable samplers are used, and if so, ascertain SamplerDescs for them.
// TODO(b/366220690): Actually perform this analysis.

// If provided a valid container ptr, this function will delegate the addition of SamplerDescs for
// each sampler the nodes utilize (dynamic and immutable). This way, a SamplerDesc's index within
// the container can inform its binding order. Each SamplerDesc will be either:
// 1) a default-constructed SamplerDesc, indicating the use of a "regular" dynamic sampler which
//    requires no special handling OR
// 2) a real SamplerDesc describing an immutable sampler. Backend pipelines can then use the desc to
//    obtain a real immutable sampler pointer (which typically must be included in pipeline layouts)

// TODO(b/347072931): Streamline to return std::unique_ptr<ShaderInfo> instead.
FragSkSLInfo BuildFragmentSkSL(const Caps* caps,
                               const ShaderCodeDictionary*,
                               const RuntimeEffectDictionary*,
                               const RenderStep* renderStep,
                               UniquePaintParamsID paintID,
                               bool useStorageBuffers,
                               skgpu::Swizzle writeSwizzle,
                               skia_private::TArray<SamplerDesc>* outDescs = nullptr);

std::string GetPipelineLabel(const ShaderCodeDictionary*,
                             const RenderPassDesc& renderPassDesc,
                             const RenderStep* renderStep,
                             UniquePaintParamsID paintID);

std::string BuildComputeSkSL(const Caps*, const ComputeStep*);

std::string EmitIntrinsicUniforms(int bufferID, Layout layout);

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
                                    int* binding,
                                    skia_private::TArray<SamplerDesc>* outDescs);
std::string EmitSamplerLayout(const ResourceBindingRequirements&, int* binding);
std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitSsboIndicesVarying,
                         bool emitLocalCoordsVarying);

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextUtils_DEFINED
