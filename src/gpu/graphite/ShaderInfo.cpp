/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ShaderInfo.h"

#include "src/gpu/BlendFormula.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

using namespace skia_private;

namespace skgpu::graphite {

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

std::string emit_intrinsic_uniforms(int bufferID, Layout layout) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(bufferID, "Intrinsic");
    result += get_uniforms(&offsetter, kIntrinsicUniforms, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");

    SkASSERTF(result.find('[') == std::string::npos,
              "Arrays are not supported in intrinsic uniforms");

    return result;
}

std::string emit_paint_params_uniforms(int bufferID,
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

std::string emit_render_step_uniforms(int bufferID,
                                      const Layout layout,
                                      SkSpan<const Uniform> uniforms) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(bufferID, "Step");
    result += get_uniforms(&offsetter, uniforms, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");

    return result;
}

std::string emit_paint_params_storage_buffer(int bufferID,
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

std::string emit_render_step_storage_buffer(int bufferID, SkSpan<const Uniform> uniforms) {
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

std::string emit_uniforms_from_storage_buffer(const char* bufferNamePrefix,
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

void append_sampler_descs(const SkSpan<const uint32_t> samplerData,
                          skia_private::TArray<SamplerDesc>& outDescs) {
    // Sampler data consists of variable-length SamplerDesc representations which can differ based
    // upon a sampler's immutability and format. For this reason, handle incrementing i in the loop.
    for (size_t i = 0; i < samplerData.size();) {
        // Create a default-initialized SamplerDesc (which only takes up one uint32). If we are
        // using a dynamic sampler, this will be directly inserted into outDescs. Otherwise, it will
        // be populated with actual immutable sampler data and then inserted.
        SamplerDesc desc{};
        size_t samplerDescLength = 1;
        SkASSERT(desc.asSpan().size() == samplerDescLength);

        // Isolate the ImmutableSamplerInfo portion of the SamplerDesc represented by samplerData.
        // If immutableSamplerInfo is non-zero, that means we are using an immutable sampler.
        uint32_t immutableSamplerInfo = samplerData[i] >> SamplerDesc::kImmutableSamplerInfoShift;
        if (immutableSamplerInfo != 0) {
            // Consult the first bit of immutableSamplerInfo which tells us whether the sampler uses
            // a known or external format. With this, update sampler description length.
            bool usesExternalFormat = immutableSamplerInfo & 0b1;
            samplerDescLength = usesExternalFormat ? SamplerDesc::kInt32sNeededExternalFormat
                                                   : SamplerDesc::kInt32sNeededKnownFormat;
            // Populate a SamplerDesc with samplerDescLength quantity of immutable sampler data
            memcpy(&desc, samplerData.begin() + i, samplerDescLength * sizeof(uint32_t));
        }
        outDescs.push_back(desc);
        i += samplerDescLength;
    }
}

std::string get_node_texture_samplers(const ResourceBindingRequirements& bindingReqs,
                                      const ShaderNode* node,
                                      int* binding,
                                      skia_private::TArray<SamplerDesc>* outDescs) {
    std::string result;
    SkSpan<const TextureAndSampler> samplers = node->entry()->fTexturesAndSamplers;

    if (!samplers.empty()) {
        SkSL::String::appendf(&result, "// %d - %s samplers\n",
                              node->keyIndex(), node->entry()->fName);

        // Determine whether we need to analyze & interpret a ShaderNode's data as immutable
        // SamplerDescs based upon whether:
        // 1) A backend passes in a non-nullptr outImmutableSamplers param (may be nullptr in
        //    backends or circumstances where we know immutable sampler data is never stored)
        // 2) Any data is stored on the ShaderNode
        // 3) Whether the ShaderNode snippet's ID matches that of any snippet ID that could store
        //    immutable sampler data.
        int32_t snippetId = node->codeSnippetId();
        if (outDescs) {
            // TODO(b/369846881): Refactor checking snippet ID to instead having a named
            // snippet requirement flag that we can check here to decrease fragility.
            if (!node->data().empty() &&
                (snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kImageShader) ||
                 snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kCubicImageShader) ||
                 snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kHWImageShader))) {
                append_sampler_descs(node->data(), *outDescs);
            } else {
                // Add default SamplerDescs for any dynamic samplers to outDescs.
                outDescs->push_back_n(samplers.size());
            }
        }

        for (const TextureAndSampler& t : samplers) {
            result += EmitSamplerLayout(bindingReqs, binding);
            SkSL::String::appendf(&result, " sampler2D %s_%d;\n", t.name(), node->keyIndex());
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_texture_samplers(bindingReqs, child, binding, outDescs);
    }
    return result;
}

std::string emit_textures_and_samplers(const ResourceBindingRequirements& bindingReqs,
                                       SkSpan<const ShaderNode*> nodes,
                                       int* binding,
                                       skia_private::TArray<SamplerDesc>* outDescs) {
    std::string result;
    for (const ShaderNode* n : nodes) {
        result += get_node_texture_samplers(bindingReqs, n, binding, outDescs);
    }
    return result;
}

std::string emit_varyings(const RenderStep* step,
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
        appendVarying({RenderStep::ssboIndicesVarying(), SkSLType::kUInt2});
    }

    if (emitLocalCoordsVarying) {
        appendVarying({"localCoordsVar", SkSLType::kFloat2});
    }

    for (auto v : step->varyings()) {
        appendVarying(v);
    }

    return result;
}

// Walk the node tree and generate all preambles, accumulating into 'preamble'.
void emit_preambles(const ShaderInfo& shaderInfo,
                    SkSpan<const ShaderNode*> nodes,
                    std::string treeLabel,
                    std::string* preamble) {
    for (int i = 0; i < SkTo<int>(nodes.size()); ++i) {
        const ShaderNode* node = nodes[i];
        std::string nodeLabel = std::to_string(i);
        std::string nextLabel = treeLabel.empty() ? nodeLabel : (treeLabel + "<-" + nodeLabel);

        if (node->numChildren() > 0) {
            emit_preambles(shaderInfo, node->children(), nextLabel, preamble);
        }

        std::string nodePreamble = node->entry()->fPreambleGenerator
                                           ? node->entry()->fPreambleGenerator(shaderInfo, node)
                                           : node->generateDefaultPreamble(shaderInfo);
        if (!nodePreamble.empty()) {
            SkSL::String::appendf(preamble,
                                  "// [%d]   %s: %s\n"
                                  "%s\n",
                                  node->keyIndex(),
                                  nextLabel.c_str(),
                                  node->entry()->fName,
                                  nodePreamble.c_str());
        }
    }
}

std::string emit_color_output(BlendFormula::OutputType outputType,
                              const char* outColor,
                              const char* inColor) {
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            return SkSL::String::printf("%s = half4(0.0);", outColor);

        case BlendFormula::kCoverage_OutputType:
            return SkSL::String::printf("%s = outputCoverage;", outColor);

        case BlendFormula::kModulate_OutputType:
            return SkSL::String::printf("%s = %s * outputCoverage;", outColor, inColor);

        case BlendFormula::kSAModulate_OutputType:
            return SkSL::String::printf("%s = %s.a * outputCoverage;", outColor, inColor);

        case BlendFormula::kISAModulate_OutputType:
            return SkSL::String::printf("%s = (1.0 - %s.a) * outputCoverage;", outColor, inColor);

        case BlendFormula::kISCModulate_OutputType:
            return SkSL::String::printf(
                    "%s = (half4(1.0) - %s) * outputCoverage;", outColor, inColor);

        default:
            SkUNREACHABLE;
    }
}

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
static constexpr skgpu::BlendInfo gBlendTable[kNumCoeffModes] = {
        /* clear */      make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
        /* src */        make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
        /* dst */        make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
        /* src-over */   make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
        /* dst-over */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
        /* src-in */     make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
        /* dst-in */     make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
        /* src-out */    make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
        /* dst-out */    make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
        /* src-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
        /* dst-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
        /* xor */        make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
        /* plus */       make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
        /* modulate */   make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC)
};

}  // anonymous namespace

std::unique_ptr<ShaderInfo> ShaderInfo::Make(const Caps* caps,
                                             const ShaderCodeDictionary* dict,
                                             const RuntimeEffectDictionary* rteDict,
                                             const RenderStep* step,
                                             UniquePaintParamsID paintID,
                                             bool useStorageBuffers,
                                             skgpu::Swizzle writeSwizzle,
                                             skia_private::TArray<SamplerDesc>* outDescs) {
    const char* shadingSsboIndex =
            useStorageBuffers && step->performsShading() ? "shadingSsboIndex" : nullptr;
    std::unique_ptr<ShaderInfo> result =
            std::unique_ptr<ShaderInfo>(new ShaderInfo(rteDict, shadingSsboIndex));

    // The fragment shader must be generated before the vertex shader, because we determine
    // properties of the entire program while generating the fragment shader.

    // If paintID is not valid this is a depth-only draw and there's no fragment shader to compile.
    if (paintID.isValid()) {
        result->generateFragmentSkSL(caps,
                                     dict,
                                     step,
                                     paintID,
                                     useStorageBuffers,
                                     writeSwizzle,
                                     outDescs);
    }

    result->generateVertexSkSL(caps,
                               step,
                               useStorageBuffers);

    return result;
}

ShaderInfo::ShaderInfo(const RuntimeEffectDictionary* rteDict, const char* ssboIndex)
        : fRuntimeEffectDictionary(rteDict), fSsboIndex(ssboIndex) {}

// The current, incomplete, model for shader construction is:
//   - Static code snippets (which can have an arbitrary signature) live in the Graphite
//     pre-compiled modules, which are located at `src/sksl/sksl_graphite_frag.sksl` and
//     `src/sksl/sksl_graphite_frag_es2.sksl`.
//   - Glue code is generated in a `main` method which calls these static code snippets.
//     The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   - The result of the final code snippet is then copied into "sk_FragColor".
//   Note: each entry's 'fStaticFunctionName' field is expected to match the name of a function
//   in the Graphite pre-compiled module, or be null if the preamble and expression generators are
//   overridden to not use a static function.
void ShaderInfo::generateFragmentSkSL(const Caps* caps,
                                      const ShaderCodeDictionary* dict,
                                      const RenderStep* step,
                                      UniquePaintParamsID paintID,
                                      bool useStorageBuffers,
                                      Swizzle writeSwizzle,
                                      skia_private::TArray<SamplerDesc>* outDescs) {
    PaintParamsKey key = dict->lookup(paintID);
    SkASSERT(key.isValid());  // invalid keys should have been caught by invalid paint ID earlier

    std::string label = key.toString(dict, /*includeData=*/false).c_str();
    fRootNodes = key.getRootNodes(dict, &fShaderNodeAlloc);

    // TODO(b/366220690): aggregateSnippetData() goes away entirely once the VulkanGraphicsPipeline
    // is updated to use the extracted SamplerDescs directly.
    for (const ShaderNode* root : fRootNodes) {
        this->aggregateSnippetData(root);
    }

#if defined(SK_DEBUG)
    // Validate the root node structure of the key.
    SkASSERT(fRootNodes.size() == 2 || fRootNodes.size() == 3);
    // First node produces the source color (all snippets return a half4), so we just require that
    // its signature takes no extra args or just local coords.
    const ShaderSnippet* srcSnippet = dict->getEntry(fRootNodes[0]->codeSnippetId());
    // TODO(b/349997190): Once SkEmptyShader doesn't use the passthrough snippet, we can assert
    // that srcSnippet->needsPriorStageOutput() is false.
    SkASSERT(!srcSnippet->needsBlenderDstColor());
    // Second node is the final blender, so it must take both the src color and dst color, and not
    // any local coordinate.
    const ShaderSnippet* blendSnippet = dict->getEntry(fRootNodes[1]->codeSnippetId());
    SkASSERT(blendSnippet->needsPriorStageOutput() && blendSnippet->needsBlenderDstColor());
    SkASSERT(!blendSnippet->needsLocalCoords());

    const ShaderSnippet* clipSnippet =
            fRootNodes.size() > 2 ? dict->getEntry(fRootNodes[2]->codeSnippetId()) : nullptr;
    SkASSERT(!clipSnippet ||
             (!clipSnippet->needsPriorStageOutput() && !clipSnippet->needsBlenderDstColor()));
#endif

    // The RenderStep should be performing shading since otherwise there's no need to generate a
    // fragment shader program at all.
    SkASSERT(step->performsShading());
    // TODO(b/372912880): Release assert debugging for illegal instruction occurring in the wild.
    SkASSERTF_RELEASE(step->performsShading(),
                      "render step: %s, label: %s",
                      step->name(),
                      label.c_str());

    // Extract the root nodes for clarity
    // TODO(b/372912880): Release assert debugging for illegal instruction occurring in the wild.
    SkASSERTF_RELEASE(fRootNodes.size() == 2 || fRootNodes.size() == 3,
                      "root node size = %zu, label = %s",
                      fRootNodes.size(),
                      label.c_str());
    const ShaderNode* const srcColorRoot = fRootNodes[0];
    const ShaderNode* const finalBlendRoot = fRootNodes[1];
    const ShaderNode* const clipRoot = fRootNodes.size() > 2 ? fRootNodes[2] : nullptr;

    // Determine the algorithm for final blending: direct HW blending, coverage-modified HW
    // blending (w/ or w/o dual-source blending) or via dst-read requirement.
    Coverage finalCoverage = step->coverage();
    if (finalCoverage == Coverage::kNone && SkToBool(clipRoot)) {
        finalCoverage = Coverage::kSingleChannel;
    }
    std::optional<SkBlendMode> finalBlendMode;
    if (finalBlendRoot->codeSnippetId() < kBuiltInCodeSnippetIDCount &&
        finalBlendRoot->codeSnippetId() >= kFixedBlendIDOffset) {
        finalBlendMode =
                static_cast<SkBlendMode>(finalBlendRoot->codeSnippetId() - kFixedBlendIDOffset);
        if (*finalBlendMode > SkBlendMode::kLastCoeffMode) {
            // TODO(b/239726010): When we support advanced blend modes in HW, these modes could
            // still be handled by fBlendInfo instead of SkSL
            finalBlendMode.reset();
        }
    }
    fDstReadRequirement = GetDstReadRequirement(caps, finalBlendMode, finalCoverage);
    // TODO(b/372912880): Release assert debugging for illegal instruction occurring in the wild.
    SkASSERTF_RELEASE(finalBlendMode.has_value() ||
                      fDstReadRequirement != DstReadRequirement::kNone,
                      "blend mode: %d, dst read: %d, coverage: %d, label = %s",
                      finalBlendMode.has_value() ? (int)*finalBlendMode : -1,
                      (int) fDstReadRequirement,
                      (int) finalCoverage,
                      label.c_str());

    const bool hasStepUniforms = step->numUniforms() > 0 && step->coverage() != Coverage::kNone;
    const bool useStepStorageBuffer = useStorageBuffers && hasStepUniforms;
    const bool useShadingStorageBuffer = useStorageBuffers && step->performsShading();

    auto allReqFlags = srcColorRoot->requiredFlags() | finalBlendRoot->requiredFlags();
    if (clipRoot) {
        allReqFlags |= clipRoot->requiredFlags();
    }
    const bool useGradientStorageBuffer = caps->gradientBufferSupport() &&
                                          (allReqFlags & SnippetRequirementFlags::kGradientBuffer);
    const bool useDstSampler = fDstReadRequirement == DstReadRequirement::kTextureCopy ||
                               fDstReadRequirement == DstReadRequirement::kTextureSample;

    const bool defineLocalCoordsVarying = this->needsLocalCoords();
    std::string preamble = emit_varyings(step,
                                         /*direction=*/"in",
                                         /*emitSsboIndicesVarying=*/useShadingStorageBuffer,
                                         defineLocalCoordsVarying);

    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    preamble += emit_intrinsic_uniforms(bindingReqs.fIntrinsicBufferBinding,
                                        bindingReqs.fUniformBufferLayout);
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            preamble += emit_render_step_storage_buffer(bindingReqs.fRenderStepBufferBinding,
                                                        step->uniforms());
        } else {
            preamble += emit_render_step_uniforms(bindingReqs.fRenderStepBufferBinding,
                                                  bindingReqs.fUniformBufferLayout,
                                                  step->uniforms());
        }
    }

    bool wrotePaintColor = false;
    if (useShadingStorageBuffer) {
        preamble += emit_paint_params_storage_buffer(bindingReqs.fPaintParamsBufferBinding,
                                                     fRootNodes,
                                                     &fHasPaintUniforms,
                                                     &wrotePaintColor);
        SkSL::String::appendf(&preamble, "uint %s;\n", this->ssboIndex());
    } else {
        preamble += emit_paint_params_uniforms(bindingReqs.fPaintParamsBufferBinding,
                                               bindingReqs.fUniformBufferLayout,
                                               fRootNodes,
                                               &fHasPaintUniforms,
                                               &wrotePaintColor);
    }

    if (useGradientStorageBuffer) {
        SkSL::String::appendf(&preamble,
                              "layout (binding=%d) readonly buffer FSGradientBuffer {\n"
                              "    float %s[];\n"
                              "};\n",
                              bindingReqs.fGradientBufferBinding,
                              ShaderInfo::kGradientBufferName);
        fHasGradientBuffer = true;
    }

    {
        int binding = 0;
        preamble += emit_textures_and_samplers(bindingReqs, fRootNodes, &binding, outDescs);
        int paintTextureCount = binding;
        if (step->hasTextures()) {
            preamble += step->texturesAndSamplersSkSL(bindingReqs, &binding);
            if (outDescs) {
                // Determine how many render step samplers were used by comparing the binding value
                // against paintTextureCount, taking into account the binding requirements. We
                // assume and do not anticipate the render steps to use immutable samplers.
                int renderStepSamplerCount = bindingReqs.fSeparateTextureAndSamplerBinding
                                                     ? (binding - paintTextureCount) / 2
                                                     : binding - paintTextureCount;
                // Add default SamplerDescs for all the dynamic samplers used by the render step so
                // the size of outDescs will be equivalent to the total number of samplers.
                outDescs->push_back_n(renderStepSamplerCount);
            }
        }
        if (useDstSampler) {
            preamble += EmitSamplerLayout(bindingReqs, &binding);
            preamble += " sampler2D dstSampler;";
            // Add default SamplerDesc for the intrinsic dstSampler to stay consistent with
            // `fNumFragmentTexturesAndSamplers`.
            if (outDescs) {
                outDescs->push_back({});
            }
        }

        // Record how many textures and samplers are used.
        fNumFragmentTexturesAndSamplers = binding;
    }

    // Emit preamble declarations and helper functions required for snippets. In the default case
    // this adds functions that bind a node's specific mangled uniforms to the snippet's
    // implementation in the SkSL modules.
    emit_preambles(*this, fRootNodes, /*treeLabel=*/"", &preamble);

    std::string mainBody = "void main() {";

    if (useShadingStorageBuffer) {
        SkSL::String::appendf(&mainBody,
                              "%s = %s.y;\n",
                              this->ssboIndex(),
                              RenderStep::ssboIndicesVarying());
    }

    if (step->emitsPrimitiveColor()) {
        mainBody += "half4 primitiveColor;";
        mainBody += step->fragmentColorSkSL();
    } else {
        SkASSERT(!(fRootNodes[0]->requiredFlags() & SnippetRequirementFlags::kPrimitiveColor));
    }

    // Using kDefaultArgs as the initial value means it will refer to undefined variables, but the
    // root nodes should--at most--be depending on the coordinate when "needsLocalCoords" is true.
    // If the PaintParamsKey violates that structure, this will produce SkSL compile errors.
    ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
    args.fFragCoord = "localCoordsVar";  // the varying added in emit_varyings()
    // TODO(b/349997190): The paint root node should not depend on any prior stage's output, but
    // it can happen with how SkEmptyShader is currently mapped to `sk_passthrough`. In this case
    // it requires that prior stage color to be transparent black. When SkEmptyShader can instead
    // cause the draw to be skipped, this can go away.
    args.fPriorStageOutput = "half4(0)";

    // Calculate the src color and stash its output variable in `args`
    args.fPriorStageOutput = srcColorRoot->invokeAndAssign(*this, args, &mainBody);

    if (fDstReadRequirement != DstReadRequirement::kNone) {
        // Get the current dst color into a local variable, it may be used later on for coverage
        // blending as well as the final blend.
        mainBody += "half4 dstColor;";
        if (useDstSampler) {
            // dstCopyBounds is in frag coords and already includes the replay translation. The
            // reciprocol of the dstCopy dimensions are in ZW.
            mainBody += "dstColor = sample(dstSampler,"
                                          "dstCopyBounds.zw*(sk_FragCoord.xy - dstCopyBounds.xy));";
        } else {
            SkASSERT(fDstReadRequirement == DstReadRequirement::kFramebufferFetch);
            mainBody += "dstColor = sk_LastFragColor;";
        }

        args.fBlenderDstColor = "dstColor";
        args.fPriorStageOutput = finalBlendRoot->invokeAndAssign(*this, args, &mainBody);
        finalBlendMode = SkBlendMode::kSrc;
    }

    if (writeSwizzle != Swizzle::RGBA()) {
        SkSL::String::appendf(&mainBody, "%s = %s.%s;", args.fPriorStageOutput.c_str(),
                                                        args.fPriorStageOutput.c_str(),
                                                        writeSwizzle.asString().c_str());
    }

    if (finalCoverage == Coverage::kNone) {
        // Either direct HW blending or a dst-read w/o any extra coverage. In both cases we just
        // need to assign directly to sk_FragCoord and update the HW blend info to finalBlendMode.
        SkASSERT(finalBlendMode.has_value());
        // TODO(b/372912880): Release assert debugging for illegal instruction occurring in the wild
        SkASSERTF_RELEASE(finalBlendMode.has_value(),
                          "blend mode: %d, dst read: %d, label = %s",
                          finalBlendMode.has_value() ? (int)*finalBlendMode : -1,
                          (int) fDstReadRequirement,
                          label.c_str());

        fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", args.fPriorStageOutput.c_str());
    } else {
        // Accumulate the output coverage. This will either modify the src color and secondary
        // outputs for dual-source blending, or be combined directly with the in-shader blended
        // final color if a dst-readback was required.
        if (useStepStorageBuffer) {
            SkSL::String::appendf(&mainBody,
                                  "uint stepSsboIndex = %s.x;\n",
                                  RenderStep::ssboIndicesVarying());
            mainBody +=
                    emit_uniforms_from_storage_buffer("step", "stepSsboIndex", step->uniforms());
        }

        mainBody += "half4 outputCoverage = half4(1);";
        mainBody += step->fragmentCoverageSkSL();

        if (clipRoot) {
            // The clip block node is invoked with device coords, not local coords like the main
            // shading root node. However sk_FragCoord includes any replay translation and we
            // need to recover the original device coordinate.
            mainBody += "float2 devCoord = sk_FragCoord.xy - viewport.xy;";
            args.fFragCoord = "devCoord";
            std::string clipBlockOutput = clipRoot->invokeAndAssign(*this, args, &mainBody);
            SkSL::String::appendf(&mainBody, "outputCoverage *= %s.a;", clipBlockOutput.c_str());
        }

        const char* outColor = args.fPriorStageOutput.c_str();
        if (fDstReadRequirement != DstReadRequirement::kNone) {
            // If this draw uses a non-coherent dst read, we want to keep the existing dst color (or
            // whatever has been previously drawn) when there's no coverage. This helps for batching
            // text draws that need to read from a dst copy for blends. However, this only helps the
            // case where the outer bounding boxes of each letter overlap and not two actual parts
            // of the text.
            if (useDstSampler) {
                // We don't think any shaders actually output negative coverage, but just as a
                // safety check for floating point precision errors, we compare with <= here. We
                // just check the RGB values of the coverage, since the alpha may not have been set
                // when using LCD. If we are using single-channel coverage, alpha will be equal to
                // RGB anyway.
                mainBody +=
                    "if (all(lessThanEqual(outputCoverage.rgb, half3(0)))) {"
                        "discard;"
                    "}";
            }

            // Use kSrc HW BlendInfo and do the coverage blend with dst in the shader.
            fBlendInfo = gBlendTable[static_cast<int>(SkBlendMode::kSrc)];
            SkSL::String::appendf(
                    &mainBody,
                    "sk_FragColor = %s * outputCoverage + dstColor * (1.0 - outputCoverage);",
                    outColor);
            if (finalCoverage == Coverage::kLCD) {
                SkSL::String::appendf(
                        &mainBody,
                        "half3 lerpRGB = mix(dstColor.aaa, %s.aaa, outputCoverage.rgb);"
                        "sk_FragColor.a = max(max(lerpRGB.r, lerpRGB.g), lerpRGB.b);",
                        outColor);
            }
        } else {
            // Adjust the shader output(s) to incorporate the coverage so that HW blending produces
            // the correct output.
            // TODO: Determine whether draw is opaque and pass that to GetBlendFormula.
            // TODO(b/372912880): Release assert debugging for illegal instruction
            SkASSERTF_RELEASE(finalBlendMode.has_value(),
                              "blend mode: %d, dst read: %d, coverage: %d, label = %s",
                              finalBlendMode.has_value() ? (int)*finalBlendMode : -1,
                              (int) fDstReadRequirement,
                              (int) finalCoverage,
                              label.c_str());
            BlendFormula coverageBlendFormula =
                    finalCoverage == Coverage::kLCD
                            ? skgpu::GetLCDBlendFormula(*finalBlendMode)
                            : skgpu::GetBlendFormula(
                                      /*isOpaque=*/false, /*hasCoverage=*/true, *finalBlendMode);
            fBlendInfo = {coverageBlendFormula.equation(),
                          coverageBlendFormula.srcCoeff(),
                          coverageBlendFormula.dstCoeff(),
                          SK_PMColor4fTRANSPARENT,
                          coverageBlendFormula.modifiesDst()};

            if (finalCoverage == Coverage::kLCD) {
                mainBody += "outputCoverage.a = max(max(outputCoverage.r, "
                                                       "outputCoverage.g), "
                                                   "outputCoverage.b);";
            }

            mainBody += emit_color_output(coverageBlendFormula.primaryOutput(),
                                          "sk_FragColor",
                                          outColor);
            if (coverageBlendFormula.hasSecondaryOutput()) {
                SkASSERT(caps->shaderCaps()->fDualSourceBlendingSupport);
                mainBody += emit_color_output(coverageBlendFormula.secondaryOutput(),
                                              "sk_SecondaryFragColor",
                                              outColor);
            }
        }
    }
    mainBody += "}\n";

    fFragmentSkSL = preamble + "\n" + mainBody;

    fFSLabel = writeSwizzle.asString().c_str();
    fFSLabel += " + ";
    fFSLabel = step->name();
    fFSLabel += " + ";
    fFSLabel += label;
}

void ShaderInfo::generateVertexSkSL(const Caps* caps,
                                    const RenderStep* step,
                                    bool useStorageBuffers) {
    const bool hasStepUniforms = step->numUniforms() > 0;
    const bool useStepStorageBuffer = useStorageBuffers && hasStepUniforms;
    const bool useShadingStorageBuffer = useStorageBuffers && step->performsShading();
    const bool defineLocalCoordsVarying = this->needsLocalCoords();

    // Fixed program header (intrinsics are always declared as an uniform interface block)
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    std::string sksl = emit_intrinsic_uniforms(bindingReqs.fIntrinsicBufferBinding,
                                               bindingReqs.fUniformBufferLayout);

    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
        int attr = 0;
        auto add_attrs = [&sksl, &attr](SkSpan<const Attribute> attrs) {
            for (auto a : attrs) {
                SkSL::String::appendf(&sksl, "    layout(location=%d) in ", attr++);
                sksl.append(SkSLTypeString(a.gpuType()));
                SkSL::String::appendf(&sksl, " %s;\n", a.name());
            }
        };
        if (step->numVertexAttributes() > 0) {
            sksl.append("// vertex attrs\n");
            add_attrs(step->vertexAttributes());
        }
        if (step->numInstanceAttributes() > 0) {
            sksl.append("// instance attrs\n");
            add_attrs(step->instanceAttributes());
        }
    }

    // Uniforms needed by RenderStep
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            sksl += emit_render_step_storage_buffer(bindingReqs.fRenderStepBufferBinding,
                                                    step->uniforms());
        } else {
            sksl += emit_render_step_uniforms(bindingReqs.fRenderStepBufferBinding,
                                              bindingReqs.fUniformBufferLayout,
                                              step->uniforms());
        }
    }

    // Varyings needed by RenderStep
    sksl += emit_varyings(step, "out", useShadingStorageBuffer, defineLocalCoordsVarying);

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
        sksl += emit_uniforms_from_storage_buffer("step", "stepSsboIndex", step->uniforms());
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

    fVertexSkSL = std::move(sksl);
    fVSLabel = step->name();
    if (defineLocalCoordsVarying) {
        fVSLabel += " (w/ local coords)";
    }
    fHasStepUniforms = hasStepUniforms;
}

bool ShaderInfo::needsLocalCoords() const {
    return !fRootNodes.empty() &&
           SkToBool(fRootNodes[0]->requiredFlags() & SnippetRequirementFlags::kLocalCoords);
}

void ShaderInfo::aggregateSnippetData(const ShaderNode* node) {
    if (!node) {
        return;
    }

    // Accumulate data of children first.
    for (const ShaderNode* child : node->children()) {
        this->aggregateSnippetData(child);
    }

    if (node->requiredFlags() & SnippetRequirementFlags::kStoresData && !node->data().empty()) {
        fData.push_back_n(node->data().size(), node->data().data());
    }
}

}  // namespace skgpu::graphite
