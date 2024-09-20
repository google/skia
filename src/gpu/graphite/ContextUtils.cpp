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
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

std::tuple<UniquePaintParamsID, UniformDataBlock, TextureDataBlock> ExtractPaintData(
        Recorder* recorder,
        PipelineDataGatherer* gatherer,
        PaintParamsKeyBuilder* builder,
        const Layout layout,
        const SkM44& local2Dev,
        const PaintParams& p,
        const Geometry& geometry,
        sk_sp<TextureProxy> dstTexture,
        SkIPoint dstOffset,
        const SkColorInfo& targetColorInfo) {
    SkDEBUGCODE(builder->checkReset());

    gatherer->resetWithNewLayout(layout);

    KeyContext keyContext(recorder,
                          local2Dev,
                          targetColorInfo,
                          geometry.isShape() || geometry.isEdgeAAQuad()
                                  ? KeyContext::OptimizeSampling::kYes
                                  : KeyContext::OptimizeSampling::kNo,
                          p.color(),
                          std::move(dstTexture),
                          dstOffset);
    p.toKey(keyContext, builder, gatherer);

    UniquePaintParamsID paintID = recorder->priv().shaderCodeDictionary()->findOrCreate(builder);
    UniformDataBlock uniforms;
    TextureDataBlock textures;
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

std::tuple<UniformDataBlock, TextureDataBlock> ExtractRenderStepData(
        UniformDataCache* uniformDataCache,
        TextureDataCache* textureDataCache,
        PipelineDataGatherer* gatherer,
        const Layout layout,
        const RenderStep* step,
        const DrawParams& params) {
    gatherer->resetWithNewLayout(layout);
    step->writeUniformsAndTextures(params, gatherer);

    UniformDataBlock uniforms =
            gatherer->hasUniforms() ? uniformDataCache->insert(gatherer->finishUniformDataBlock())
                                    : UniformDataBlock();
    TextureDataBlock textures =
            gatherer->hasTextures() ? textureDataCache->insert(gatherer->textureDataBlock())
                                    : TextureDataBlock();

    return { uniforms, textures };
}

DstReadRequirement GetDstReadRequirement(const Caps* caps,
                                         std::optional<SkBlendMode> blendMode,
                                         Coverage coverage) {
    // If the blend mode is absent, this is assumed to be for a runtime blender, for which we always
    // do a dst read.
    // If the blend mode is plus, always do in-shader blending since we may be drawing to an
    // unsaturated surface (e.g. F16) and we don't want to let the hardware clamp the color output
    // in that case. We could check the draw dst properties to only do in-shader blending with plus
    // when necessary, but we can't detect that during shader precompilation.
    if (!blendMode || *blendMode > SkBlendMode::kLastCoeffMode ||
        *blendMode == SkBlendMode::kPlus) {
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

std::string get_uniforms(UniformOffsetCalculator* offsetter,
                         SkSpan<const Uniform> uniforms,
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

        SkSL::String::appendf(&result,
                              "    layout(offset=%d) %s %s",
                              offsetter->advanceOffset(u.type(), u.count()),
                              SkSLTypeString(u.type()),
                              uniformName.c_str());
        if (u.count()) {
            result.append("[");
            result.append(std::to_string(u.count()));
            result.append("]");
        }
        result.append(";\n");
    }

    return result;
}

std::string get_node_uniforms(UniformOffsetCalculator* offsetter,
                              const ShaderNode* node,
                              bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        if (node->entry()->fUniformStructName) {
            auto substruct = UniformOffsetCalculator::ForStruct(offsetter->layout());
            for (const Uniform& u : uniforms) {
                substruct.advanceOffset(u.type(), u.count());
            }

            const int structOffset = offsetter->advanceStruct(substruct);
            SkSL::String::appendf(&result,
                                  "layout(offset=%d) %s node_%d;",
                                  structOffset,
                                  node->entry()->fUniformStructName,
                                  node->keyIndex());
        } else {
            SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                                node->keyIndex(), node->entry()->fName);
            result += get_uniforms(offsetter, uniforms, node->keyIndex(), wrotePaintColor);
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_uniforms(offsetter, child, wrotePaintColor);
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

std::string get_node_ssbo_fields(const ShaderNode* node, bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        if (node->entry()->fUniformStructName) {
            SkSL::String::appendf(&result, "%s node_%d;",
                                  node->entry()->fUniformStructName, node->keyIndex());
        } else {
            SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                                  node->keyIndex(), node->entry()->fName);

            result += get_ssbo_fields(uniforms, node->keyIndex(), wrotePaintColor);
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_ssbo_fields(child, wrotePaintColor);
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

static constexpr Uniform kIntrinsicUniforms[] = { {"viewport",      SkSLType::kFloat4},
                                                  {"dstCopyBounds", SkSLType::kFloat4} };

std::string emit_intrinsic_uniforms(int bufferID, Layout layout) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(bufferID, "Intrinsic");
    result += get_uniforms(&offsetter, kIntrinsicUniforms, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");

    SkASSERTF(result.find('[') == std::string::npos,
              "Arrays are not supported in intrinsic uniforms");

    return result;
}

}  // anonymous namespace

void CollectIntrinsicUniforms(const Caps* caps,
                              SkIRect viewport,
                              SkIRect dstCopyBounds,
                              UniformManager* uniforms) {
    SkDEBUGCODE(uniforms->setExpectedUniforms(kIntrinsicUniforms, /*isSubstruct=*/false);)

    // viewport
    {
        // The vertex shader needs to divide by the dimension and then multiply by 2, so do this
        // once on the CPU. This is because viewport normalization wants to range from -1 to 1, and
        // not 0 to 1. If any other user of the viewport uniform requires the true reciprocal or
        // original dimensions, this can be adjusted.
        SkASSERT(!viewport.isEmpty());
        float invTwoW = 2.f / viewport.width();
        float invTwoH = 2.f / viewport.height();

        // If the NDC Y axis points up (opposite normal skia convention and the underlying view
        // convention), upload the inverse height as a negative value. See BuildVertexSkSL
        // for how this is used.
        if (!caps->ndcYAxisPointsDown()) {
            invTwoH *= -1.f;
        }
        uniforms->write(SkV4{(float) viewport.left(), (float) viewport.top(), invTwoW, invTwoH});
    }

    // dstCopyBounds
    {
        // Unlike viewport, dstCopyBounds can be empty so check for 0 dimensions and set the
        // reciprocal to 0. It is also not doubled since its purpose is to normalize texture coords
        // to 0 to 1, and not -1 to 1.
        int width = dstCopyBounds.width();
        int height = dstCopyBounds.height();
        uniforms->write(SkV4{(float) dstCopyBounds.left(), (float) dstCopyBounds.top(),
                             width ? 1.f / width : 0.f, height ? 1.f / height : 0.f});
    }

    SkDEBUGCODE(uniforms->doneWithExpectedUniforms());
}

std::string EmitPaintParamsUniforms(int bufferID,
                                    const Layout layout,
                                    SkSpan<const ShaderNode*> nodes,
                                    bool* hasUniforms,
                                    bool* wrotePaintColor) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(bufferID, "FS");
    for (const ShaderNode* n : nodes) {
        result += get_node_uniforms(&offsetter, n, wrotePaintColor);
    }
    result.append("};\n\n");

    *hasUniforms = offsetter.size() > 0;
    if (!*hasUniforms) {
        // No uniforms were added
        return {};
    }

    return result;
}

std::string EmitRenderStepUniforms(int bufferID,
                                   const Layout layout,
                                   SkSpan<const Uniform> uniforms) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(bufferID, "Step");
    result += get_uniforms(&offsetter, uniforms, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");

    return result;
}

std::string EmitPaintParamsStorageBuffer(
        int bufferID,
        SkSpan<const ShaderNode*> nodes,
        bool* hasUniforms,
        bool* wrotePaintColor) {
    *hasUniforms = false;

    std::string fields;
    for (const ShaderNode* n : nodes) {
        fields += get_node_ssbo_fields(n, wrotePaintColor);
    }

    if (fields.empty()) {
        // No uniforms were added
        *hasUniforms = false;
        return {};
    }

    *hasUniforms = true;
    return SkSL::String::printf(
            "struct FSUniformData {\n"
            "%s\n"
            "};\n\n"
            "layout (binding=%d) readonly buffer FSUniforms {\n"
                "FSUniformData fsUniformData[];\n"
            "};\n",
            fields.c_str(),
            bufferID);
}

std::string EmitRenderStepStorageBuffer(
        int bufferID,
        SkSpan<const Uniform> uniforms) {
    SkASSERT(!uniforms.empty());
    std::string fields = get_ssbo_fields(uniforms, -1, /*wrotePaintColor=*/nullptr);
    return SkSL::String::printf(
            "struct StepUniformData {\n"
            "%s\n"
            "};\n\n"
            "layout (binding=%d) readonly buffer StepUniforms {\n"
            "    StepUniformData stepUniformData[];\n"
            "};\n",
            fields.c_str(),
            bufferID);
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

    auto appendVarying = [&](const Varying& v) {
        const char* interpolation;
        switch (v.interpolation()) {
            case Interpolation::kPerspective: interpolation = ""; break;
            case Interpolation::kLinear:      interpolation = "noperspective "; break;
            case Interpolation::kFlat:        interpolation = "flat "; break;
        }
        SkSL::String::appendf(&result, "layout(location=%d) %s %s%s %s;\n",
                              location++,
                              direction,
                              interpolation,
                              SkSLTypeString(v.gpuType()),
                              v.name());
    };

    if (emitSsboIndicesVarying) {
        appendVarying({RenderStep::ssboIndicesVarying(), SkSLType::kUShort2});
    }

    if (emitLocalCoordsVarying) {
        appendVarying({"localCoordsVar", SkSLType::kFloat2});
    }

    for (auto v : step->varyings()) {
        appendVarying(v);
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

    // Fixed program header (intrinsics are always declared as an uniform interface block)
    std::string sksl = emit_intrinsic_uniforms(bindingReqs.fIntrinsicBufferBinding,
                                               bindingReqs.fUniformBufferLayout);

    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
        sksl += emit_attributes(step->vertexAttributes(), step->instanceAttributes());
    }

    // Uniforms needed by RenderStep
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            sksl += EmitRenderStepStorageBuffer(bindingReqs.fRenderStepBufferBinding,
                                                step->uniforms());
        } else {
            sksl += EmitRenderStepUniforms(bindingReqs.fRenderStepBufferBinding,
                                           bindingReqs.fUniformBufferLayout,
                                           step->uniforms());
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

    // We want to map the rectangle of logical device pixels from (0,0) to (viewWidth, viewHeight)
    // to normalized device coordinates: (-1,-1) to (1,1) (actually -w to w since it's before
    // homogenous division).
    //
    // For efficiency, this assumes viewport.zw holds the reciprocol of twice the viewport width and
    // height. On some backends the NDC Y axis is flipped relative to the device and
    // viewport coords (i.e. it points up instead of down). In those cases, it's also assumed that
    // viewport.w holds a negative value. In that case the sign(viewport.zw) changes from
    // subtracting w to adding w.
    sksl += "sk_Position = float4(viewport.zw*devPosition.xy - sign(viewport.zw)*devPosition.ww,"
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
    result.fHasStepUniforms = hasStepUniforms;

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
                                     &result.fHasPaintUniforms,
                                     &result.fHasGradientBuffer,
                                     writeSwizzle);

    // Extract blend info after integrating the RenderStep into the final fragment shader in case
    // that changes the HW blending choice to handle analytic coverage.
    result.fBlendInfo = shaderInfo.blendInfo();
    result.fRequiresLocalCoords = shaderInfo.needsLocalCoords();
    result.fData = {shaderInfo.data()};
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
