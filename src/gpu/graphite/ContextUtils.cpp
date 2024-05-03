/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextUtils.h"

#include <string>
#include "src/core/SkBlenderBase.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

std::tuple<UniquePaintParamsID, const UniformDataBlock*, const TextureDataBlock*>
ExtractPaintData(Recorder* recorder,
                 PipelineDataGatherer* gatherer,
                 PaintParamsKeyBuilder* builder,
                 const Layout layout,
                 const SkM44& local2Dev,
                 const PaintParams& p,
                 sk_sp<TextureProxy> dstTexture,
                 SkIPoint dstOffset,
                 const SkColorInfo& targetColorInfo) {
    SkDEBUGCODE(builder->checkReset());

    gatherer->resetWithNewLayout(layout);

    KeyContext keyContext(
            recorder, local2Dev, targetColorInfo, p.color(), std::move(dstTexture), dstOffset);
    p.toKey(keyContext, builder, gatherer);

    UniquePaintParamsID paintID = recorder->priv().shaderCodeDictionary()->findOrCreate(builder);
    const UniformDataBlock* uniforms = nullptr;
    const TextureDataBlock* textures = nullptr;
    if (paintID.isValid()) {
        if (gatherer->hasUniforms()) {
            UniformDataCache* uniformDataCache = recorder->priv().uniformDataCache();
            uniforms = uniformDataCache->insert(gatherer->finishUniformDataBlock());
        }
        if (gatherer->hasTextures()) {
            TextureDataCache* textureDataCache = recorder->priv().textureDataCache();
            textures = textureDataCache->insert(gatherer->textureDataBlock());
        }
    }

    return { paintID, uniforms, textures };
}

std::tuple<const UniformDataBlock*, const TextureDataBlock*> ExtractRenderStepData(
        UniformDataCache* uniformDataCache,
        TextureDataCache* textureDataCache,
        PipelineDataGatherer* gatherer,
        const Layout layout,
        const RenderStep* step,
        const DrawParams& params) {
    gatherer->resetWithNewLayout(layout);
    step->writeUniformsAndTextures(params, gatherer);

    const UniformDataBlock* uniforms =
            gatherer->hasUniforms() ? uniformDataCache->insert(gatherer->finishUniformDataBlock())
                                    : nullptr;
    const TextureDataBlock* textures =
            gatherer->hasTextures() ? textureDataCache->insert(gatherer->textureDataBlock())
                                    : nullptr;

    return { uniforms, textures };
}

DstReadRequirement GetDstReadRequirement(const Caps* caps,
                                         std::optional<SkBlendMode> blendMode,
                                         Coverage coverage) {
    // If the blend mode is absent, this is assumed to be for a runtime blender, for which we always
    // do a dst read.
    if (!blendMode || *blendMode > SkBlendMode::kLastCoeffMode) {
        return caps->getDstReadRequirement();
    }

    const bool isLCD = coverage == Coverage::kLCD;
    const bool hasCoverage = coverage != Coverage::kNone;
    BlendFormula blendFormula = isLCD ? skgpu::GetLCDBlendFormula(*blendMode)
                                      : skgpu::GetBlendFormula(false, hasCoverage, *blendMode);
    if ((blendFormula.hasSecondaryOutput() && !caps->shaderCaps()->fDualSourceBlendingSupport) ||
        (coverage == Coverage::kLCD && blendMode != SkBlendMode::kSrcOver)) {
        return caps->getDstReadRequirement();
    }

    return DstReadRequirement::kNone;
}

namespace {

std::string get_uniform_header(int bufferID, const char* name) {
    std::string result;

    SkSL::String::appendf(&result, "layout (binding=%d) uniform %sUniforms {\n", bufferID, name);

    return result;
}

std::string get_uniforms(Layout layout,
                         SkSpan<const Uniform> uniforms,
                         int* offset,
                         int manglingSuffix,
                         bool* wrotePaintColor) {
    std::string result;
    UniformOffsetCalculator offsetter(layout, *offset);

    std::string uniformName;
    for (const Uniform& u : uniforms) {
        uniformName = u.name();

        if (u.isPaintColor() && wrotePaintColor) {
            if (*wrotePaintColor) {
                SkSL::String::appendf(&result, "    // deduplicated %s\n", u.name());
                continue;
            }

            *wrotePaintColor = true;
        } else {
            if (manglingSuffix >= 0) {
                uniformName.append("_");
                uniformName.append(std::to_string(manglingSuffix));
            }
        }

        SkSL::String::appendf(&result,
                              "    layout(offset=%d) %s %s",
                              offsetter.advanceOffset(u.type(), u.count()),
                              SkSLTypeString(u.type()),
                              uniformName.c_str());
        if (u.count()) {
            result.append("[");
            result.append(std::to_string(u.count()));
            result.append("]");
        }
        result.append(";\n");
    }

    *offset = offsetter.size();
    return result;
}

std::string get_node_uniforms(Layout layout,
                              const ShaderNode* node,
                              int* offset,
                              int* numUniforms,
                              bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                              node->keyIndex(), node->entry()->fName);
        result += get_uniforms(layout, uniforms, offset, node->keyIndex(), wrotePaintColor);
    }

    *numUniforms += uniforms.size();
    for (const ShaderNode* child : node->children()) {
        result += get_node_uniforms(layout, child, offset, numUniforms, wrotePaintColor);
    }
    return result;
}

std::string get_ssbo_fields(SkSpan<const Uniform> uniforms,
                            int manglingSuffix,
                            bool* wrotePaintColor) {
    std::string result;

    std::string uniformName;
    for (const Uniform& u : uniforms) {
        uniformName = u.name();

        if (u.isPaintColor() && wrotePaintColor) {
            if (*wrotePaintColor) {
                SkSL::String::appendf(&result, "    // deduplicated %s\n", u.name());
                continue;
            }

            *wrotePaintColor = true;
        } else {
            if (manglingSuffix >= 0) {
                uniformName.append("_");
                uniformName.append(std::to_string(manglingSuffix));
            }
        }

        SkSL::String::appendf(&result, "    %s %s", SkSLTypeString(u.type()), uniformName.c_str());
        if (u.count()) {
            SkSL::String::appendf(&result, "[%d]", u.count());
        }
        result.append(";\n");
    }

    return result;
}

std::string get_node_ssbo_fields(const ShaderNode* node, int* numUniforms, bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                              node->keyIndex(), node->entry()->fName);

        result += get_ssbo_fields(uniforms, node->keyIndex(), wrotePaintColor);
    }

    *numUniforms += uniforms.size();
    for (const ShaderNode* child : node->children()) {
        result += get_node_ssbo_fields(child, numUniforms, wrotePaintColor);
    }
    return result;
}

std::string get_node_texture_samplers(const ResourceBindingRequirements& bindingReqs,
                                      const ShaderNode* node,
                                      int* binding) {
    std::string result;
    SkSpan<const TextureAndSampler> samplers = node->entry()->fTexturesAndSamplers;

    if (!samplers.empty()) {
        SkSL::String::appendf(&result, "// %d - %s samplers\n",
                              node->keyIndex(), node->entry()->fName);

        for (const TextureAndSampler& t : samplers) {
            result += EmitSamplerLayout(bindingReqs, binding);
            SkSL::String::appendf(&result, " sampler2D %s_%d;\n",
                                  t.name(), node->keyIndex());
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_texture_samplers(bindingReqs, child, binding);
    }
    return result;
}

}  // anonymous namespace

std::string EmitPaintParamsUniforms(int bufferID,
                                    const Layout layout,
                                    SkSpan<const ShaderNode*> nodes,
                                    int* numUniforms,
                                    int* uniformsTotalBytes,
                                    bool* wrotePaintColor) {
    int offset = 0;

    std::string result = get_uniform_header(bufferID, "FS");
    for (const ShaderNode* n : nodes) {
        result += get_node_uniforms(layout, n, &offset, numUniforms, wrotePaintColor);
    }
    result.append("};\n\n");

    if (!*numUniforms) {
        // No uniforms were added
        return {};
    }

    if (uniformsTotalBytes) {
        *uniformsTotalBytes = offset;
    }

    return result;
}

std::string EmitRenderStepUniforms(int bufferID,
                                   const Layout layout,
                                   SkSpan<const Uniform> uniforms,
                                   int* renderStepUniformsTotalBytes) {
    int offset = 0;

    std::string result = get_uniform_header(bufferID, "Step");
    result += get_uniforms(layout, uniforms, &offset, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");

    if (renderStepUniformsTotalBytes) {
        *renderStepUniformsTotalBytes = offset;
    }

    return result;
}

std::string EmitPaintParamsStorageBuffer(
        int bufferID,
        SkSpan<const ShaderNode*> nodes,
        int* numUniforms,
        bool* wrotePaintColor) {

    std::string result;
    result += "struct FSUniformData {\n";
    for (const ShaderNode* n : nodes) {
        result += get_node_ssbo_fields(n, numUniforms, wrotePaintColor);
    }
    result += "};\n\n";

    if (!*numUniforms) {
        // No uniforms were added
        return {};
    }

    SkSL::String::appendf(&result,
                          "layout (binding=%d) readonly buffer FSUniforms {\n"
                          "    FSUniformData fsUniformData[];\n"
                          "};\n",
                          bufferID);
    return result;
}

std::string EmitRenderStepStorageBuffer(
        int bufferID,
        SkSpan<const Uniform> uniforms) {

    std::string result;
    result += "struct StepUniformData {\n" +
              get_ssbo_fields(uniforms, -1, /* wrotePaintColor= */ nullptr) +
              "};\n\n";

    SkSL::String::appendf(&result,
                          "layout (binding=%d) readonly buffer StepUniforms {\n"
                          "    StepUniformData stepUniformData[];\n"
                          "};\n",
                          bufferID);
    return result;
}

std::string EmitUniformsFromStorageBuffer(const char* bufferNamePrefix,
                                          const char* ssboIndex,
                                          SkSpan<const Uniform> uniforms) {
    std::string result;

    for (const Uniform& u : uniforms) {
        SkSL::String::appendf(&result, "%s %s", SkSLTypeString(u.type()), u.name());
        if (u.count()) {
            SkSL::String::appendf(&result, "[%d]", u.count());
        }
        SkSL::String::appendf(
                &result, " = %sUniformData[%s].%s;\n", bufferNamePrefix, ssboIndex, u.name());
    }

    return result;
}

std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName) {
    return SkSL::String::printf("%sUniformData[%s].%s", bufferNamePrefix, ssboIndex, uniformName);
}

std::string EmitTexturesAndSamplers(const ResourceBindingRequirements& bindingReqs,
                                    SkSpan<const ShaderNode*> nodes,
                                    int* binding) {
    std::string result;
    for (const ShaderNode* n : nodes) {
        result += get_node_texture_samplers(bindingReqs, n, binding);
    }
    return result;
}

std::string EmitSamplerLayout(const ResourceBindingRequirements& bindingReqs, int* binding) {
    std::string result;

    // If fDistinctIndexRanges is false, then texture and sampler indices may clash with other
    // resource indices. Graphite assumes that they will be placed in descriptor set (Vulkan) and
    // bind group (Dawn) index 1.
    const char* distinctIndexRange = bindingReqs.fDistinctIndexRanges ? "" : "set=1, ";

    if (bindingReqs.fSeparateTextureAndSamplerBinding) {
        int samplerIndex = (*binding)++;
        int textureIndex = (*binding)++;
        result = SkSL::String::printf("layout(webgpu, %ssampler=%d, texture=%d)",
                                      distinctIndexRange,
                                      samplerIndex,
                                      textureIndex);
    } else {
        int samplerIndex = (*binding)++;
        result = SkSL::String::printf("layout(%sbinding=%d)",
                                      distinctIndexRange,
                                      samplerIndex);
    }
    return result;
}

namespace {
std::string emit_attributes(SkSpan<const Attribute> vertexAttrs,
                            SkSpan<const Attribute> instanceAttrs) {
    std::string result;

    int attr = 0;
    auto add_attrs = [&](SkSpan<const Attribute> attrs) {
        for (auto a : attrs) {
            SkSL::String::appendf(&result, "    layout(location=%d) in ", attr++);
            result.append(SkSLTypeString(a.gpuType()));
            SkSL::String::appendf(&result, " %s;\n", a.name());
        }
    };

    if (!vertexAttrs.empty()) {
        result.append("// vertex attrs\n");
        add_attrs(vertexAttrs);
    }
    if (!instanceAttrs.empty()) {
        result.append("// instance attrs\n");
        add_attrs(instanceAttrs);
    }

    return result;
}
}  // anonymous namespace

std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitSsboIndicesVarying,
                         bool emitLocalCoordsVarying) {
    std::string result;
    int location = 0;

    if (emitSsboIndicesVarying) {
        SkSL::String::appendf(&result,
                              "    layout(location=%d) %s flat ushort2 %s;\n",
                              location++,
                              direction,
                              RenderStep::ssboIndicesVarying());
    }

    if (emitLocalCoordsVarying) {
        SkSL::String::appendf(&result, "    layout(location=%d) %s ", location++, direction);
        result.append(SkSLTypeString(SkSLType::kFloat2));
        SkSL::String::appendf(&result, " localCoordsVar;\n");
    }

    for (auto v : step->varyings()) {
        SkSL::String::appendf(&result, "    layout(location=%d) %s ", location++, direction);
        result.append(SkSLTypeString(v.fType));
        SkSL::String::appendf(&result, " %s;\n", v.fName);
    }

    return result;
}

VertSkSLInfo BuildVertexSkSL(const ResourceBindingRequirements& bindingReqs,
                            const RenderStep* step,
                            bool useStorageBuffers,
                            bool defineLocalCoordsVarying) {
    VertSkSLInfo result;

    const bool hasStepUniforms = step->numUniforms() > 0;
    const bool useStepStorageBuffer = useStorageBuffers && hasStepUniforms;
    const bool useShadingStorageBuffer = useStorageBuffers && step->performsShading();

    // TODO: To more completely support end-to-end rendering, this will need to be updated so that
    // the RenderStep shader snippet can produce a device coord, a local coord, and depth.
    // If the paint combination doesn't need the local coord it can be ignored, otherwise we need
    // a varying for it. The fragment function's output will need to be updated to have a color and
    // the depth, or when there's no combination, just the depth. Lastly, we also should add the
    // static/intrinsic uniform binding point so that we can handle normalizing the device position
    // produced by the RenderStep automatically.

    // Fixed program header
    std::string sksl =
        "layout (binding=0) uniform intrinsicUniforms {\n"
        "    layout(offset=0) float4 rtAdjust;\n"
        "};\n"
        "\n";

    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
        sksl += emit_attributes(step->vertexAttributes(), step->instanceAttributes());
    }

    // Uniforms needed by RenderStep
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    // TODO: replace hard-coded bufferID with the backend's renderstep uniform-buffer index.
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            sksl += EmitRenderStepStorageBuffer(/* bufferID= */ 1, step->uniforms());
        } else {
            sksl += EmitRenderStepUniforms(/* bufferID= */ 1,
                                           bindingReqs.fUniformBufferLayout,
                                           step->uniforms(),
                                           &result.fRenderStepUniformsTotalBytes);
        }
    }

    // Varyings needed by RenderStep
    sksl += EmitVaryings(step, "out", useShadingStorageBuffer, defineLocalCoordsVarying);

    // Vertex shader function declaration
    sksl += "void main() {";
    // Create stepLocalCoords which render steps can write to.
    sksl += "float2 stepLocalCoords = float2(0);";
    // Vertex shader body
    if (useStepStorageBuffer) {
        // Extract out render step uniforms from SSBO, declaring local variables with the expected
        // uniform names so that RenderStep SkSL is independent of storage choice.
        SkSL::String::appendf(
                &sksl, "uint stepSsboIndex = %s.x;\n", RenderStep::ssboIndicesAttribute());
        sksl += EmitUniformsFromStorageBuffer("step", "stepSsboIndex", step->uniforms());
    }

    sksl += step->vertexSkSL();
    sksl += "sk_Position = float4(devPosition.xy * rtAdjust.xy + devPosition.ww * rtAdjust.zw,"
            "devPosition.zw);";

    if (useShadingStorageBuffer) {
        // Assign SSBO index values to the SSBO index varying.
        SkSL::String::appendf(&sksl,
                              "%s = %s;",
                              RenderStep::ssboIndicesVarying(),
                              RenderStep::ssboIndicesAttribute());
    }

    if (defineLocalCoordsVarying) {
        // Assign Render Step's stepLocalCoords to the localCoordsVar varying.
        sksl += "localCoordsVar = stepLocalCoords;";
    }
    sksl += "}";

    result.fSkSL = std::move(sksl);
    result.fLabel = step->name();
    if (defineLocalCoordsVarying) {
        result.fLabel += " (w/ local coords)";
    }

    return result;
}

FragSkSLInfo BuildFragmentSkSL(const Caps* caps,
                               const ShaderCodeDictionary* dict,
                               const RuntimeEffectDictionary* rteDict,
                               const RenderStep* step,
                               UniquePaintParamsID paintID,
                               bool useStorageBuffers,
                               skgpu::Swizzle writeSwizzle) {
    FragSkSLInfo result;
    if (!paintID.isValid()) {
        // Depth-only draw so no fragment shader to compile
        return {};
    }

    const char* shadingSsboIndex =
            useStorageBuffers && step->performsShading() ? "shadingSsboIndex" : nullptr;
    ShaderInfo shaderInfo(paintID, dict, rteDict, shadingSsboIndex);

    result.fSkSL = shaderInfo.toSkSL(caps,
                                     step,
                                     useStorageBuffers,
                                     &result.fNumTexturesAndSamplers,
                                     &result.fNumPaintUniforms,
                                     &result.fRenderStepUniformsTotalBytes,
                                     &result.fPaintUniformsTotalBytes,
                                     writeSwizzle);

    // Extract blend info after integrating the RenderStep into the final fragment shader in case
    // that changes the HW blending choice to handle analytic coverage.
    result.fBlendInfo = shaderInfo.blendInfo();
    result.fRequiresLocalCoords = shaderInfo.needsLocalCoords();

    result.fLabel = writeSwizzle.asString().c_str();
    result.fLabel += " + ";
    result.fLabel = step->name();
    result.fLabel += " + ";
    result.fLabel += dict->idToString(paintID).c_str();

    return result;
}

std::string GetPipelineLabel(const ShaderCodeDictionary* dict,
                             const RenderPassDesc& renderPassDesc,
                             const RenderStep* renderStep,
                             UniquePaintParamsID paintID) {
    std::string label = renderPassDesc.toPipelineLabel().c_str(); // includes the write swizzle
    label += " + ";
    label += renderStep->name();
    label += " + ";
    label += dict->idToString(paintID).c_str(); // will be "(empty)" for depth-only draws
    return label;
}

std::string BuildComputeSkSL(const Caps* caps, const ComputeStep* step) {
    std::string sksl =
            SkSL::String::printf("layout(local_size_x=%u, local_size_y=%u, local_size_z=%u) in;\n",
                                 step->localDispatchSize().fWidth,
                                 step->localDispatchSize().fHeight,
                                 step->localDispatchSize().fDepth);

    const auto& bindingReqs = caps->resourceBindingRequirements();
    bool distinctRanges = bindingReqs.fDistinctIndexRanges;
    bool separateSampler = bindingReqs.fSeparateTextureAndSamplerBinding;

    int index = 0;
    int texIdx = 0;
    // NOTE: SkSL Metal codegen always assigns the same binding index to a texture and its sampler.
    // TODO: This could cause sampler indices to not be tightly packed if the sampler2D declaration
    // comes after 1 or more storage texture declarations (which don't have samplers). An optional
    // "layout(msl, sampler=T, texture=T)" syntax to count them separately (like we do for WGSL)
    // could come in handy here but it's not supported in MSL codegen yet.

    for (const ComputeStep::ResourceDesc& r : step->resources()) {
        using Type = ComputeStep::ResourceType;
        switch (r.fType) {
            case Type::kUniformBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) uniform ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kStorageBuffer:
            case Type::kIndirectBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) buffer ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kReadOnlyStorageBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) readonly buffer ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kWriteOnlyStorageTexture:
                SkSL::String::appendf(&sksl, "layout(binding=%d, rgba8) writeonly texture2D ",
                                      distinctRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kReadOnlyTexture:
                SkSL::String::appendf(&sksl, "layout(binding=%d, rgba8) readonly texture2D ",
                                      distinctRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kSampledTexture:
                if (distinctRanges) {
                    SkSL::String::appendf(&sksl, "layout(metal, binding=%d) ", texIdx++);
                } else if (separateSampler) {
                    SkSL::String::appendf(
                            &sksl, "layout(webgpu, sampler=%d, texture=%d) ", index, index + 1);
                    index += 2;
                } else {
                    SkSL::String::appendf(&sksl, "layout(binding=%d) ", index++);
                }
                sksl += "sampler2D ";
                sksl += r.fSkSL;
                break;
        }
        sksl += ";\n";
    }

    sksl += step->computeSkSL();
    return sksl;
}

} // namespace skgpu::graphite
