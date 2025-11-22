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
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

using namespace skia_private;

namespace skgpu::graphite {

namespace {

struct LiftedExpression {
    // The node who's expression should be lifted.
    const ShaderNode* fNode;
    // The arguments to use as input to the lifted expression.
    ShaderSnippet::Args fArgs;
    // If true, capture the expression's resolved value in a varying.
    // This is false for expressions whose output is only used in other lifted expressions.
    bool fEmitVarying = true;
};

std::string get_uniform_header(int set, int bufferID) {
    std::string result;
    SkSL::String::appendf(
            &result, "layout (set=%d, binding=%d) uniform CombinedUniforms {\n", set, bufferID);
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
                              int* numUniforms,
                              int* numUnliftedUniforms,
                              bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        *numUniforms += uniforms.size();
        if (!((node->requiredFlags() & SnippetRequirementFlags::kLiftExpression) ||
              (node->requiredFlags() & SnippetRequirementFlags::kOmitExpression))) {
            *numUnliftedUniforms += uniforms.size();
        }

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
#if defined(SK_DEBUG)
            SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                                  node->keyIndex(), node->entry()->fName);
#endif
            result += get_uniforms(offsetter, uniforms, node->keyIndex(), wrotePaintColor);
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_uniforms(
                offsetter, child, numUniforms, numUnliftedUniforms, wrotePaintColor);
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
#if defined(SK_DEBUG)
                SkSL::String::appendf(&result, "    // deduplicated %s\n", u.name());
#endif
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

std::string get_node_ssbo_fields(const ShaderNode* node,
                                 int* numUniforms,
                                 int* numUnliftedUniforms,
                                 bool* wrotePaintColor) {
    std::string result;
    SkSpan<const Uniform> uniforms = node->entry()->fUniforms;

    if (!uniforms.empty()) {
        *numUniforms += uniforms.size();
        if (!((node->requiredFlags() & SnippetRequirementFlags::kLiftExpression) ||
              (node->requiredFlags() & SnippetRequirementFlags::kOmitExpression))) {
            *numUnliftedUniforms += uniforms.size();
        }

        if (node->entry()->fUniformStructName) {
            SkSL::String::appendf(&result, "%s node_%d;",
                                  node->entry()->fUniformStructName, node->keyIndex());
        } else {
#if defined(SK_DEBUG)
            SkSL::String::appendf(&result, "// %d - %s uniforms\n",
                                  node->keyIndex(), node->entry()->fName);
#endif
            result += get_ssbo_fields(uniforms, node->keyIndex(), wrotePaintColor);
        }
    }

    for (const ShaderNode* child : node->children()) {
        result += get_node_ssbo_fields(child, numUniforms, numUnliftedUniforms, wrotePaintColor);
    }
    return result;
}

std::string emit_intrinsic_constants(const ResourceBindingRequirements& bindingReqs) {
    std::string result;
    auto offsetter = UniformOffsetCalculator::ForTopLevel(bindingReqs.fUniformBufferLayout);

    if (bindingReqs.fUsePushConstantsForIntrinsicConstants) {
        SkASSERT(bindingReqs.fBackendApi == BackendApi::kVulkan ||
                 bindingReqs.fBackendApi == BackendApi::kDawn);
        result = SkSL::String::printf(
                "layout (%s, push_constant) uniform IntrinsicUniforms {\n",
                bindingReqs.fBackendApi == BackendApi::kVulkan ? "vulkan" : "webgpu");
    } else {
        std::string header;
        SkSL::String::appendf(&header,
                              "layout (set=%d, binding=%d) uniform IntrinsicUniforms {\n",
                              bindingReqs.fUniformsSetIdx,
                              bindingReqs.fIntrinsicBufferBinding);
        result = std::move(header);
    }
    result += get_uniforms(&offsetter, kIntrinsicUniforms, -1, /* wrotePaintColor= */ nullptr);
    result.append("};\n\n");
    SkASSERTF(bindingReqs.fUsePushConstantsForIntrinsicConstants ||
              result.find('[') == std::string::npos,
              "Arrays are not supported in intrinsic uniforms");
    return result;
}

std::string emit_combined_uniforms(int set,
                                   int bufferID,
                                   const Layout layout,
                                   SkSpan<const ShaderNode*> nodes,
                                   SkSpan<const Uniform> stepUniforms,
                                   int* numPaintUniforms,
                                   int* numUnliftedPaintUniforms,
                                   bool* wrotePaintColor) {
    auto offsetter = UniformOffsetCalculator::ForTopLevel(layout);

    std::string result = get_uniform_header(set, bufferID);
    for (const ShaderNode* n : nodes) {
        result += get_node_uniforms(
                &offsetter, n, numPaintUniforms, numUnliftedPaintUniforms, wrotePaintColor);
    }

    // Paint and RenderStep uniforms share a binding. When RenderSteps are processed in DrawList,
    // the paint uniforms are always processed before the render step ones, so the emitted uniforms
    // must respect that ordering.
    if (!stepUniforms.empty()) {
        result += get_uniforms(&offsetter, stepUniforms, -1, /* wrotePaintColor= */ nullptr);
    }

    result.append("};\n\n");

    if (*numPaintUniforms == 0 && stepUniforms.empty()) {
        // No uniforms were added
        return {};
    }

    return result;
}

std::string emit_combined_storage_buffer(int set,
                                         int bufferID,
                                         SkSpan<const ShaderNode*> nodes,
                                         SkSpan<const Uniform> stepUniforms,
                                         int* numPaintUniforms,
                                         int* numUnliftedPaintUniforms,
                                         bool* wrotePaintColor) {
    std::string fields;
    for (const ShaderNode* n : nodes) {
        fields += get_node_ssbo_fields(
                n, numPaintUniforms, numUnliftedPaintUniforms, wrotePaintColor);
    }

    if (!stepUniforms.empty()) {
        fields += get_ssbo_fields(stepUniforms, -1, /*wrotePaintColor=*/nullptr);
    }

    if (*numPaintUniforms == 0 && stepUniforms.empty()) {
        // No uniforms were added
        return {};
    }

    return SkSL::String::printf(
            "struct CombinedUniformData {\n"
            "    %s\n"
            "};\n\n"
            "layout (set=%d, binding=%d) readonly buffer CombinedUniforms {\n"
            "    CombinedUniformData combinedUniformData[];\n"
            "};\n",
            fields.c_str(),
            set,
            bufferID);
}

std::string emit_uniforms_from_storage_buffer(const char* indexVariableName,
                                              SkSpan<const Uniform> uniforms) {
    std::string result;

    for (const Uniform& u : uniforms) {
        SkSL::String::appendf(&result, "%s %s", SkSLTypeString(u.type()), u.name());
        if (u.count()) {
            SkSL::String::appendf(&result, "[%d]", u.count());
        }
        SkSL::String::appendf(&result,
                              " = combinedUniformData[%s].%s;\n",
                              indexVariableName,
                              u.name());
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
            memcpy(&desc, samplerData.data() + i, samplerDescLength * sizeof(uint32_t));
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
#if defined(SK_DEBUG)
        SkSL::String::appendf(&result, "// %d - %s samplers\n",
                              node->keyIndex(), node->entry()->fName);
#endif

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
                 snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kImageShaderClamp) ||
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

SkSLType sksl_type_for_lifted_expression(
        ShaderSnippet::LiftableExpressionType liftedExpressionType) {
    switch (liftedExpressionType) {
        case ShaderSnippet::LiftableExpressionType::kNone:
            return SkSLType::kVoid;
        case ShaderSnippet::LiftableExpressionType::kLocalCoords:
            return SkSLType::kFloat2;
        case ShaderSnippet::LiftableExpressionType::kPriorStageOutput:
            return SkSLType::kHalf4;
    }
    return SkSLType::kVoid;
}

std::string emit_varyings(const RenderStep* step,
                          const char* direction,
                          SkSpan<const LiftedExpression> liftedExpressions,
                          bool emitSsboIndexVarying,
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

    if (emitSsboIndexVarying) {
        appendVarying({RenderStep::ssboIndexVarying(), SkSLType::kUInt});
    }

    if (emitLocalCoordsVarying) {
        appendVarying({"localCoordsVar", SkSLType::kFloat2});
    }

    for (const LiftedExpression& expr : liftedExpressions) {
        if (expr.fEmitVarying) {
            const ShaderNode* node = expr.fNode;
            const std::string name = node->getExpressionVaryingName();
            appendVarying({name.c_str(),
                           sksl_type_for_lifted_expression(node->entry()->fLiftableExpressionType),
                           node->entry()->fLiftableExpressionInterpolation});
        }
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

std::string emit_advanced_blend_color_output(const char* outColor, const char* inColor) {
    /*
      When using hardware for advanced blend modes, we apply coverage for advanced blend modes by
      multiplying it into the src color before blending. This will "just work" given the following:

      The general SVG blend equation is defined in the spec as follows:

        Dca' = B(Sc, Dc) * Sa * Da + Y * Sca * (1-Da) + Z * Dca * (1-Sa)
        Da'  = X * Sa * Da + Y * Sa * (1-Da) + Z * Da * (1-Sa)

      (Note that Sca, Dca indicate RGB vectors that are premultiplied by alpha,
       and that B(Sc, Dc) is a mode-specific function that accepts non-multiplied
       RGB colors.)

      For every blend mode supported by this class, i.e. the "advanced" blend
      modes, X=Y=Z=1 and this equation reduces to the PDF blend equation.

      It can be shown that when X=Y=Z=1, these equations can modulate alpha for
      coverage.


      == Color ==

      We substitute Y=Z=1 and define a blend() function that calculates Dca' in
      terms of premultiplied alpha only:

        blend(Sca, Dca, Sa, Da) = {
                Dca : if Sa == 0,
                Sca : if Da == 0,
                B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa) : if Sa,Da != 0}

      And for coverage modulation, we use a post blend src-over model:

        Dca'' = f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

      (Where f is the fractional coverage.)

      Next we show that we can multiply coverage into the src color by proving the
      following relationship:

        blend(f*Sca, Dca, f*Sa, Da) == f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

      General case (f,Sa,Da != 0):

        f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
          = f * (B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa)) + (1-f) * Dca
            [Sa,Da != 0, definition of blend()]
          = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + f*Dca * (1-Sa) + Dca - f*Dca
          = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da + f*Dca - f*Dca * Sa + Dca - f*Dca
          = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da - f*Dca * Sa + Dca
          = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) - f*Dca * Sa + Dca
          = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)
          = B(f*Sca/f*Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)  [f!=0]
          = blend(f*Sca, Dca, f*Sa, Da)  [definition of blend()]

      Corner cases (Sa=0, Da=0, and f=0):

        Sa=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
                = f * Dca + (1-f) * Dca  [Sa=0, definition of blend()]
                = Dca
                = blend(0, Dca, 0, Da)  [definition of blend()]
                = blend(f*Sca, Dca, f*Sa, Da)  [Sa=0]

        Da=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
                = f * Sca + (1-f) * Dca  [Da=0, definition of blend()]
                = f * Sca  [Da=0]
                = blend(f*Sca, 0, f*Sa, 0)  [definition of blend()]
                = blend(f*Sca, Dca, f*Sa, Da)  [Da=0]

        f=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
               = Dca  [f=0]
               = blend(0, Dca, 0, Da)  [definition of blend()]
               = blend(f*Sca, Dca, f*Sa, Da)  [f=0]

      == Alpha ==

      We substitute X=Y=Z=1 and define a blend() function that calculates Da':

        blend(Sa, Da) = Sa * Da + Sa * (1-Da) + Da * (1-Sa)
                      = Sa * Da + Sa - Sa * Da + Da - Da * Sa
                      = Sa + Da - Sa * Da

      We use the same model for coverage modulation as we did with color:

        Da'' = f * blend(Sa, Da) + (1-f) * Da

      And show that show that we can multiply coverage into src alpha by proving the following
      relationship:

        blend(f*Sa, Da) == f * blend(Sa, Da) + (1-f) * Da

        f * blend(Sa, Da) + (1-f) * Da
          = f * (Sa + Da - Sa * Da) + (1-f) * Da
          = f*Sa + f*Da - f*Sa * Da + Da - f*Da
          = f*Sa - f*Sa * Da + Da
          = f*Sa + Da - f*Sa * Da
          = blend(f*Sa, Da)
    */
   return emit_color_output(BlendFormula::OutputType::kModulate_OutputType, outColor, inColor);
}

void collect_lifted_expressions(SkSpan<const ShaderNode*> nodes,
                                const ShaderSnippet::Args& args,
                                std::vector<LiftedExpression>& lifted) {
    for (const ShaderNode* node : nodes) {
        const bool emitVaryingInFS =
                static_cast<bool>(node->requiredFlags() & SnippetRequirementFlags::kLiftExpression);
        const bool emitExpressionInVS =
                emitVaryingInFS ||
                (node->requiredFlags() & SnippetRequirementFlags::kOmitExpression);
        SkASSERT(!emitExpressionInVS || node->entry()->fLiftableExpressionGenerator);

        ShaderSnippet::Args childArgs = args;
        if (emitExpressionInVS && node->entry()->fLiftableExpressionGenerator) {
            lifted.push_back({node, args, emitVaryingInFS});
            switch (node->entry()->fLiftableExpressionType) {
                case ShaderSnippet::LiftableExpressionType::kLocalCoords:
                    childArgs.fFragCoord = node->getExpressionVaryingName();
                    break;
                case ShaderSnippet::LiftableExpressionType::kPriorStageOutput:
                    childArgs.fPriorStageOutput = node->getExpressionVaryingName();
                    break;
                default:
                    SkUNREACHABLE;
            }
        }

        collect_lifted_expressions(node->children(), childArgs, lifted);
    }
}

std::vector<LiftedExpression> collect_lifted_expressions(SkSpan<const ShaderNode*> nodes) {
    std::vector<LiftedExpression> lifted;
    ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
    args.fFragCoord = "stepLocalCoords";  // Render Steps' stepLocalCoords
    collect_lifted_expressions(nodes, args, lifted);
    return lifted;
}

std::string dst_read_strategy_to_str(DstReadStrategy strategy) {
    switch (strategy) {
        case DstReadStrategy::kNoneRequired:
            return "NoneRequired";
        case DstReadStrategy::kTextureCopy:
            return "TextureCopy";
        case DstReadStrategy::kTextureSample:
            return "TextureSample";
        case DstReadStrategy::kReadFromInput:
            return "ReadFromInput";
        case DstReadStrategy::kFramebufferFetch:
            return "FramebufferFetch";
    }
    SkUNREACHABLE;
}

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}


constexpr skgpu::BlendEquation get_advanced_blend_equation(SkBlendMode mode) {
    SkASSERT(mode > SkBlendMode::kLastCoeffMode);

    constexpr int kEqOffset = ((int)skgpu::BlendEquation::kOverlay - (int)SkBlendMode::kOverlay);
    static_assert((int)skgpu::BlendEquation::kOverlay ==
                  (int)SkBlendMode::kOverlay + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kDarken ==
                  (int)SkBlendMode::kDarken + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kLighten ==
                  (int)SkBlendMode::kLighten + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kColorDodge ==
                  (int)SkBlendMode::kColorDodge + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kColorBurn ==
                  (int)SkBlendMode::kColorBurn + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kHardLight ==
                  (int)SkBlendMode::kHardLight + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kSoftLight ==
                  (int)SkBlendMode::kSoftLight + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kDifference ==
                  (int)SkBlendMode::kDifference + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kExclusion ==
                  (int)SkBlendMode::kExclusion + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kMultiply ==
                  (int)SkBlendMode::kMultiply + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kHSLHue ==
                  (int)SkBlendMode::kHue + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kHSLSaturation ==
                  (int)SkBlendMode::kSaturation + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kHSLColor ==
                  (int)SkBlendMode::kColor + kEqOffset);
    static_assert((int)skgpu::BlendEquation::kHSLLuminosity ==
                  (int)SkBlendMode::kLuminosity + kEqOffset);
    // There's an illegal BlendEquation that corresponds to no SkBlendMode, hence the extra +1.
    static_assert(skgpu::kBlendEquationCnt == (int)SkBlendMode::kLastMode + 1 + 1 + kEqOffset);

    return static_cast<skgpu::BlendEquation>((int)mode + kEqOffset);
}

constexpr skgpu::BlendInfo make_hardware_advanced_blendInfo(SkBlendMode advancedBlendMode) {
    BlendInfo blendInfo;
    blendInfo.fEquation = get_advanced_blend_equation(advancedBlendMode);
    return blendInfo;
}

static constexpr skgpu::BlendInfo gBlendTable[kSkBlendModeCount] = {
        /* Porter-Duff blend modes */
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
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC),

        /* BlendInfo for advanced blend modes */
        make_hardware_advanced_blendInfo(SkBlendMode::kOverlay),
        make_hardware_advanced_blendInfo(SkBlendMode::kDarken),
        make_hardware_advanced_blendInfo(SkBlendMode::kLighten),
        make_hardware_advanced_blendInfo(SkBlendMode::kColorDodge),
        make_hardware_advanced_blendInfo(SkBlendMode::kColorBurn),
        make_hardware_advanced_blendInfo(SkBlendMode::kHardLight),
        make_hardware_advanced_blendInfo(SkBlendMode::kSoftLight),
        make_hardware_advanced_blendInfo(SkBlendMode::kDifference),
        make_hardware_advanced_blendInfo(SkBlendMode::kExclusion),
        make_hardware_advanced_blendInfo(SkBlendMode::kMultiply),
        make_hardware_advanced_blendInfo(SkBlendMode::kHue),
        make_hardware_advanced_blendInfo(SkBlendMode::kSaturation),
        make_hardware_advanced_blendInfo(SkBlendMode::kColor),
        make_hardware_advanced_blendInfo(SkBlendMode::kLuminosity)
};

}  // anonymous namespace

struct ShaderInfo::SharedGeneratorData {
    SharedGeneratorData(const Caps* caps,
                        const ShaderCodeDictionary* dict,
                        SkArenaAlloc* alloc,
                        const RenderStep* step,
                        UniquePaintParamsID paintID,
                        const char* uniformSsboIndex)
            : fRootNodes(SkSpan<const ShaderNode*>())
            , fHasStepUniforms(step->numUniforms() > 0) {

        // Decompress Root Nodes & Determine Local Coords
        if (paintID.isValid()) {
            PaintParamsKey key = dict->lookup(paintID);
            SkASSERT(key.isValid());

            constexpr int kFixedVaryings = 2;
            const int availableVaryings =
                    caps->maxVaryings() - kFixedVaryings - step->varyings().size();

            fRootNodes = key.getRootNodes(caps, dict, alloc, availableVaryings);

            fNeedsLocalCoords = !fRootNodes.empty() &&
                               SkToBool(fRootNodes[0]->requiredFlags() &
                               SnippetRequirementFlags::kLocalCoords);
        } else {
            fNeedsLocalCoords = false;
        }

        // Lift Expressions & Check Uniforms
        bool vsHasLiftedPaintUniforms = false;
        fLiftedExpr = collect_lifted_expressions(fRootNodes);
        for (const auto& expr : fLiftedExpr) {
            if (!expr.fNode->entry()->fUniforms.empty()) {
                vsHasLiftedPaintUniforms = true;
                break;
            }
        }

        bool needsCombinedBufferVS = fHasStepUniforms || vsHasLiftedPaintUniforms;
        fUseUniformStorageBufferVS = caps->storageBufferSupport() && needsCombinedBufferVS;
        fUseUniformStorageBufferFS = caps->storageBufferSupport() && step->performsShading();
        bool useUniformStorageBuffer = fUseUniformStorageBufferVS || fUseUniformStorageBufferFS;

        // Emit Preamble
        int numPaintUniforms = 0;
        int numUnliftedPaintUniforms = 0;
        bool wrotePaintColor = false;
        SkSpan<const Uniform> allStepUniforms = step->uniforms();
        const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();

        if (useUniformStorageBuffer) {
            fSharedPreamble = emit_combined_storage_buffer(
                bindingReqs.fUniformsSetIdx,
                bindingReqs.fCombinedUniformBufferBinding,
                fRootNodes,
                allStepUniforms,
                &numPaintUniforms,
                &numUnliftedPaintUniforms,
                &wrotePaintColor);
        } else {
            fSharedPreamble = emit_combined_uniforms(
                bindingReqs.fUniformsSetIdx,
                bindingReqs.fCombinedUniformBufferBinding,
                bindingReqs.fUniformBufferLayout,
                fRootNodes,
                allStepUniforms,
                &numPaintUniforms,
                &numUnliftedPaintUniforms,
                &wrotePaintColor);
        }

        // Calculate Final Flags
        fHasPaintUniforms = numPaintUniforms > 0;
        fHasLiftedPaintUniforms = (numPaintUniforms - numUnliftedPaintUniforms) > 0;
        bool hasUnliftedPaintUniforms = numUnliftedPaintUniforms > 0;

        fHasSsboIndexVarying = useUniformStorageBuffer &&
                              (hasUnliftedPaintUniforms ||
                              (fHasStepUniforms && step->usesUniformsInFragmentSkSL()));

        // Append SSBO Index to preamble if required
        if (useUniformStorageBuffer && uniformSsboIndex) {
            SkSL::String::appendf(&fSharedPreamble, "uint %s;\n", uniformSsboIndex);
        }
    }

    // The decompressed shader tree
    SkSpan<const ShaderNode*> fRootNodes;

    // The expressions lifted from the shader tree
    // Changed from const& to value to allow ownership
    std::vector<LiftedExpression> fLiftedExpr;

    // The base SkSL preamble (uniforms, varyings) shared by both stages
    std::string fSharedPreamble;

    // Shared calculated properties
    bool fNeedsLocalCoords;
    bool fHasSsboIndexVarying;
    bool fHasStepUniforms;
    bool fHasPaintUniforms;
    bool fHasLiftedPaintUniforms;

    // Buffer usage flags needed by generators
    bool fUseUniformStorageBufferVS;
    bool fUseUniformStorageBufferFS;
};

std::unique_ptr<ShaderInfo> ShaderInfo::Make(const Caps* caps,
                                             const ShaderCodeDictionary* dict,
                                             const RuntimeEffectDictionary* rteDict,
                                             const RenderPassDesc& rpDesc,
                                             const RenderStep* step,
                                             UniquePaintParamsID paintID,
                                             skia_private::TArray<SamplerDesc>* outDescs) {
    // Determine if an SSBO index is needed at all (by either stage)
    bool needsSsboIndex =
        caps->storageBufferSupport() && (step->performsShading() || step->numUniforms() > 0);
    const char* uniformSsboIndex = needsSsboIndex ? "uniformSsboIndex" : nullptr;

    const bool hasFragShader = paintID.isValid() && step->performsShading();

    // Create the final ShaderInfo object.
    auto result = std::unique_ptr<ShaderInfo>(new ShaderInfo(dict, rteDict, uniformSsboIndex,
                                              hasFragShader ? rpDesc.fDstReadStrategy
                                                            : DstReadStrategy::kNoneRequired));

    // This arena holds all the ShaderNodes. It must live for the duration of 'Make' so the
    // rootNodes span is valid when passed to helpers.
    SkArenaAlloc shaderNodeAlloc{256};
    SharedGeneratorData sharedData(
            caps, dict, &shaderNodeAlloc, step, paintID, result->uniformSsboIndex());
    result->fHasCombinedUniforms = sharedData.fHasStepUniforms || sharedData.fHasPaintUniforms;

    SkString paintLabel = dict->idToString(caps, paintID);
    if (hasFragShader) {
        result->generateFragmentSkSL(caps,
                                     dict,
                                     paintLabel.c_str(),
                                     step,
                                     paintID,
                                     rpDesc.fColorAttachment.fFormat,
                                     rpDesc.fWriteSwizzle,
                                     outDescs,
                                     sharedData);
    } else {
        result->fBlendInfo.fWritesColor = false;
    }

    result->generateVertexSkSL(caps, step, sharedData);
    result->fVSLabel = step->name();
    if (sharedData.fNeedsLocalCoords) {
        result->fVSLabel += " (w/ local coords)";
    }

    result->fFSLabel = step->name();
    result->fFSLabel += " + ";
    result->fFSLabel += paintLabel.c_str();
    if (rpDesc.fWriteSwizzle != Swizzle::RGBA() ||
        result->fDstReadStrategy != DstReadStrategy::kNoneRequired) {
        result->fFSLabel += "(";
        result->fFSLabel += rpDesc.fWriteSwizzle.asString().c_str();
        if (result->fDstReadStrategy != DstReadStrategy::kNoneRequired) {
            result->fFSLabel += ", ";
            result->fFSLabel += dst_read_strategy_to_str(result->fDstReadStrategy);
        }
        result->fFSLabel += ")";
    }

    // KEEP IN SYNC with ContextUtils::GetPipelineLabel()
    result->fPipelineLabel = rpDesc.toPipelineLabel().c_str();
    result->fPipelineLabel += " + ";
    result->fPipelineLabel += step->name();
    result->fPipelineLabel += " + ";
    result->fPipelineLabel += paintLabel.c_str();

    return result;
}

ShaderInfo::ShaderInfo(const ShaderCodeDictionary* shaderCodeDictionary,
                       const RuntimeEffectDictionary* rteDict,
                       const char* uniformSsboIndex,
                       DstReadStrategy dstReadStrategy)
        : fShaderCodeDictionary(shaderCodeDictionary)
        , fRuntimeEffectDictionary(rteDict)
        , fUniformSsboIndex(uniformSsboIndex)
        , fDstReadStrategy(dstReadStrategy) {}

// The current, incomplete, model for shader construction is:
//   - Static code snippets (which can have an arbitrary signature) live in the Graphite
//     pre-compiled module, which is located at `src/sksl/sksl_graphite_frag.sksl`.
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
                                      const char* label,
                                      const RenderStep* step,
                                      UniquePaintParamsID paintID,
                                      TextureFormat targetFormat,
                                      Swizzle writeSwizzle,
                                      skia_private::TArray<SamplerDesc>* outDescs,
                                      const SharedGeneratorData& sharedData) {
#if defined(SK_DEBUG)
    // Validate the root node structure of the key.
    SkASSERT(sharedData.fRootNodes.size() == 2 || sharedData.fRootNodes.size() == 3);
    // First node produces the source color (all snippets return a half4), so we just require that
    // its signature takes no extra args or just local coords.
    const ShaderSnippet* srcSnippet = dict->getEntry(sharedData.fRootNodes[0]->codeSnippetId());
    SkASSERT(!srcSnippet->needsBlenderDstColor());
    // TODO(b/349997190): Once SkEmptyShader doesn't use the passthrough snippet, we can assert
    // that srcSnippet->needsPriorStageOutput() is false.
    SkASSERT(!srcSnippet->needsBlenderDstColor());
    // Second node is the final blender, so it must take both the src color and dst color, and not
    // any local coordinate.
    const ShaderSnippet* blendSnippet = dict->getEntry(sharedData.fRootNodes[1]->codeSnippetId());
    SkASSERT(blendSnippet->needsPriorStageOutput() && blendSnippet->needsBlenderDstColor());
    SkASSERT(!blendSnippet->needsLocalCoords());
    // Optional third node is the clip
    const ShaderSnippet* clipSnippet = sharedData.fRootNodes.size() > 2 ?
            dict->getEntry(sharedData.fRootNodes[2]->codeSnippetId()) : nullptr;
    SkASSERT(!clipSnippet ||
             (!clipSnippet->needsPriorStageOutput() && !clipSnippet->needsBlenderDstColor()));
#endif

    // Check for unexpected corruption / illegal instructions occurring in the wild.
    SkASSERTF_RELEASE(sharedData.fRootNodes.size() == 2 || sharedData.fRootNodes.size() == 3,
                      "root node size = %zu, label = %s", sharedData.fRootNodes.size(), label);

    // Extract the root nodes for clarity
    const ShaderNode* const srcColorRoot = sharedData.fRootNodes[0];
    const ShaderNode* const finalBlendRoot = sharedData.fRootNodes[1];
    const int32_t finalBlendRootSnippetId = finalBlendRoot->codeSnippetId();
    const ShaderNode* const clipRoot =
            sharedData.fRootNodes.size() > 2 ? sharedData.fRootNodes[2] : nullptr;

    // Determine the algorithm for final blending: direct HW blending, coverage-modified HW
    // blending (w/ or w/o dual-source blending) or via dst-read requirement.
    Coverage finalCoverage = step->coverage();
    if (finalCoverage == Coverage::kNone && SkToBool(clipRoot)) {
        finalCoverage = Coverage::kSingleChannel;
    }

    // Initialize the final blend mode to the final snippet's blend mode. It may be changed based
    // upon whether or not we can use hardware blending.
    std::optional<SkBlendMode> finalBlendMode;
    if (finalBlendRootSnippetId < kBuiltInCodeSnippetIDCount &&
        finalBlendRootSnippetId >= kFixedBlendIDOffset) {
        finalBlendMode = static_cast<SkBlendMode>(finalBlendRootSnippetId - kFixedBlendIDOffset);
    }
    if (finalBlendMode.has_value() &&
        CanUseHardwareBlending(caps, targetFormat, *finalBlendMode, finalCoverage)) {
        // If we can use hardware blending, update the dstReadStrategy to be kNoneRequired to ensure
        // that ShaderInfo properly informs PipelineInfo of the pipeline's dst read requirement.
        fDstReadStrategy = DstReadStrategy::kNoneRequired;
    } else {
        // If we cannot use hardware blending, then we must perform a dst read within the shader.
        // Therefore we should assert that a valid strategy to do so was passed in. Later operations
        // also expect the blend mode to be kSrc, so update that here.
        SkASSERT(fDstReadStrategy != DstReadStrategy::kNoneRequired);
        finalBlendMode = SkBlendMode::kSrc;
    }

    auto allReqFlags = srcColorRoot->requiredFlags() | finalBlendRoot->requiredFlags();
    if (clipRoot) {
        allReqFlags |= clipRoot->requiredFlags();
    }

    std::string fsPreamble;
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    fsPreamble += emit_intrinsic_constants(bindingReqs);
    fsPreamble += emit_varyings(step, "in",
                                sharedData.fLiftedExpr,
                                sharedData.fHasSsboIndexVarying,
                                sharedData.fNeedsLocalCoords);

    if (fDstReadStrategy == DstReadStrategy::kReadFromInput) {
        // If this shader reads the dst texture as an input attachment, assert that a valid set
        // index has been assigned within ResourceBindingRequirements.
        SkASSERT(bindingReqs.fInputAttachmentSetIdx != ResourceBindingRequirements::kUnassigned);
        // TODO: The following SkSL depends upon the fact that Vulkan is currently the only backend
        // that utilizes DstReadStrategy::kReadFromInput. Update accordingly if other backends add
        // support for this DstReadStrategy.
        SkSL::String::appendf(
                &fsPreamble,
                "layout (vulkan, input_attachment_index=%d, set=%d, binding=%d) "
                "subpassInput DstTextureInput;\n",
                /*input attachment idx within set=*/0,
                /*input attachment set idx=*/bindingReqs.fInputAttachmentSetIdx,
                /*binding=*/0);
    }

    bool useGradientBuffer = caps->gradientBufferSupport() &&
                              (allReqFlags & SnippetRequirementFlags::kGradientBuffer);
    if (useGradientBuffer) {
        SkSL::String::appendf(&fsPreamble,
                              "layout (set=%d, binding=%d) readonly buffer FSGradientBuffer {\n"
                              "    float %s[];\n"
                              "};\n",
                              bindingReqs.fUniformsSetIdx,
                              bindingReqs.fGradientBufferBinding,
                              ShaderInfo::kGradientBufferName);
        fHasGradientBuffer = true;
    }

    const bool useDstSampler = fDstReadStrategy == DstReadStrategy::kTextureCopy ||
                               fDstReadStrategy == DstReadStrategy::kTextureSample;
    {
        int binding = 0;
        fsPreamble += emit_textures_and_samplers(bindingReqs, sharedData.fRootNodes, &binding,
                                               outDescs);
        int paintTextureCount = binding;
        if (step->hasTextures()) {
            fsPreamble += step->texturesAndSamplersSkSL(bindingReqs, &binding);
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
            fsPreamble += EmitSamplerLayout(bindingReqs, &binding);
            fsPreamble += " sampler2D dstSampler;";
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
    emit_preambles(*this, sharedData.fRootNodes, /*treeLabel=*/"", &fsPreamble);

    std::string mainBody = "void main() {";

    if (sharedData.fHasSsboIndexVarying) {
        SkSL::String::appendf(&mainBody,
                              "%s = %s;\n",
                              this->uniformSsboIndex(),
                              RenderStep::ssboIndexVarying());
    }

    if (step->emitsPrimitiveColor()) {
        mainBody += "half4 primitiveColor;";
        mainBody += step->fragmentColorSkSL();
    } else {
        SkASSERT(!(sharedData.fRootNodes[0]->requiredFlags() &
                 SnippetRequirementFlags::kPrimitiveColor));
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

    // If not using hardware blending, we perform a dst read in the shader and must add SkSL
    // accordingly.
    if (fDstReadStrategy != DstReadStrategy::kNoneRequired) {
        // Get the current dst color into a local variable, it may be used later on for coverage
        // blending as well as the final blend.
        mainBody += "half4 dstColor;";
        if (useDstSampler) {
            // dstReadBounds is in frag coords and already includes the replay translation. The
            // reciprocol of the dstCopy dimensions are in ZW.
            mainBody += "dstColor = sample(dstSampler,"
                                          "dstReadBounds.zw*(sk_FragCoord.xy - dstReadBounds.xy));";
        } else if (fDstReadStrategy == DstReadStrategy::kReadFromInput) {
            // The dst texture should have been written to with the appropriate write swizzle, so we
            // do not need to worry about the read swizzle when accessing that value for blending.
            mainBody += "// Read color from input attachment\n";
            mainBody += "dstColor = subpassLoad(DstTextureInput);\n";
        } else {
            SkASSERT(fDstReadStrategy == DstReadStrategy::kFramebufferFetch);
            mainBody += "dstColor = sk_LastFragColor;";
        }

        args.fBlenderDstColor = "dstColor";
        args.fPriorStageOutput = finalBlendRoot->invokeAndAssign(*this, args, &mainBody);
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
        fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", args.fPriorStageOutput.c_str());
    } else {
        // Accumulate the output coverage. This will either modify the src color and secondary
        // outputs for dual-source blending, or be combined directly with the in-shader blended
        // final color if a dst-readback was required.

        if (sharedData.fUseUniformStorageBufferFS && sharedData.fHasStepUniforms) {
            mainBody +=
                    emit_uniforms_from_storage_buffer(this->uniformSsboIndex(), step->uniforms());
        }

        mainBody += "half4 outputCoverage = half4(1);";
        if (step->coverage() != Coverage::kNone) {
            mainBody += step->fragmentCoverageSkSL();
        }

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
        if (fDstReadStrategy != DstReadStrategy::kNoneRequired) {
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
            SkASSERT(finalBlendMode.has_value() && finalBlendMode.value() == SkBlendMode::kSrc);
            fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
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
            if (finalBlendMode > SkBlendMode::kLastCoeffMode) {
                SkASSERT(finalCoverage == Coverage::kSingleChannel);
                fBlendInfo = gBlendTable[static_cast<int>(*finalBlendMode)];
                mainBody += emit_advanced_blend_color_output("sk_FragColor", outColor);
            } else {
                // Porter-Duff blend modes can utilize BlendFormula.
                // TODO: Determine whether draw is opaque and pass that to GetBlendFormula.
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
    }
    mainBody += "}\n";

    SkASSERT(fFragmentSkSL.empty());
    fFragmentSkSL.reserve(
            sharedData.fSharedPreamble.size() + fsPreamble.size() + mainBody.size() +2);
    fFragmentSkSL  = sharedData.fSharedPreamble;
    fFragmentSkSL += "\n";
    fFragmentSkSL += fsPreamble;
    fFragmentSkSL += "\n";
    fFragmentSkSL += mainBody;
}

void ShaderInfo::generateVertexSkSL(const Caps* caps,
                                    const RenderStep* step,
                                    const SharedGeneratorData& sharedData) {
    std::string vsPreamble;
    // Fixed program header (intrinsics are always declared as an uniform interface block)
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    vsPreamble = emit_intrinsic_constants(bindingReqs);
    // Varyings needed by RenderStep and potentially lifted expressions
    vsPreamble += emit_varyings(step, "out",
                                sharedData.fLiftedExpr,
                                sharedData.fHasSsboIndexVarying,
                                sharedData.fNeedsLocalCoords);

    // Add vertex attributes
    if (step->numStaticAttributes() > 0 || step->numAppendAttributes() > 0) {
        int attr = 0;
        auto add_attrs = [&vsPreamble, &attr](SkSpan<const Attribute> attrs) {
            for (auto a : attrs) {
                SkSL::String::appendf(&vsPreamble, "    layout(location=%d) in ", attr++);
                vsPreamble.append(SkSLTypeString(a.gpuType()));
                SkSL::String::appendf(&vsPreamble, " %s;\n", a.name());
            }
        };
        if (step->numStaticAttributes() > 0) {
#if defined(SK_DEBUG)
            vsPreamble.append("// static attrs\n");
#endif
            add_attrs(step->staticAttributes());
        }
        if (step->numAppendAttributes() > 0) {
#if defined(SK_DEBUG)
            vsPreamble.append("// append attrs\n");
#endif
            add_attrs(step->appendAttributes());
        }
    }

    // Vertex shader function declaration
    std::string mainBody = "void main() {";
    // Create stepLocalCoords which render steps can write to.
    mainBody += "float2 stepLocalCoords = float2(0);";

    // We define the SSBO index variable immediately if the VS is using storage buffers. This covers
    // both the "Step Uniforms" case and the "Lifted Uniforms Only" case.
    if (sharedData.fUseUniformStorageBufferVS) {
        SkSL::String::appendf(&mainBody, "uint %s = %s;\n", this->uniformSsboIndex(),
                                  RenderStep::ssboIndexAttribute());
        if (sharedData.fHasStepUniforms) {
            mainBody +=
                    emit_uniforms_from_storage_buffer(this->uniformSsboIndex(), step->uniforms());
        }
    }

    // Inject RenderStep's main vertex logic
    mainBody += step->vertexSkSL();

    // Calculate sk_Position
    mainBody +=
            "sk_Position = float4(viewport.zw*devPosition.xy - sign(viewport.zw)*devPosition.ww,"
            "devPosition.zw);";

    // Assign local coords to varying if needed
    if (sharedData.fNeedsLocalCoords) {
        mainBody += "localCoordsVar = stepLocalCoords;";
    }

    // Generate lifted expressions
    if (!sharedData.fLiftedExpr.empty()) {
        for (const LiftedExpression& expr : sharedData.fLiftedExpr) {
            const ShaderNode* node = expr.fNode;
            // Determine the SkSL type string if not emitting directly to a varying
            const char* typeStr = expr.fEmitVarying
                                          ? ""
                                          : SkSLTypeString(sksl_type_for_lifted_expression(
                                                    node->entry()->fLiftableExpressionType));
            const std::string varName = node->getExpressionVaryingName();

            // Generate the expression code, potentially extracting uniforms from SSBO if needed
            std::string expression;
            expression = node->entry()->fLiftableExpressionGenerator(*this, node, expr.fArgs);

            // Assign the expression result to the varying or a temporary variable
            SkSL::String::appendf(
                    &mainBody, "%s %s = %s;", typeStr, varName.c_str(), expression.c_str());
        }
    }

    // Assign SSBO index to varying if needed
    if (sharedData.fHasSsboIndexVarying) {
        if (sharedData.fUseUniformStorageBufferVS) {
            // Use the local variable we already defined
            SkSL::String::appendf(&mainBody,
                                  "%s = %s;",
                                  RenderStep::ssboIndexVarying(),
                                  this->uniformSsboIndex());
        } else {
            // No local variable, read directly from attribute
            SkSL::String::appendf(&mainBody,
                                  "%s = %s;",
                                  RenderStep::ssboIndexVarying(),
                                  RenderStep::ssboIndexAttribute());
        }
    }

    mainBody += "}"; // End main()

    SkASSERT(fVertexSkSL.empty());
    fVertexSkSL.reserve(
            sharedData.fSharedPreamble.size() + vsPreamble.size() + mainBody.size() + 2);
    fVertexSkSL = sharedData.fSharedPreamble;
    fVertexSkSL += "\n";
    fVertexSkSL += vsPreamble;
    fVertexSkSL += "\n";
    fVertexSkSL += mainBody;
}

}  // namespace skgpu::graphite
