/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ShaderCodeDictionary.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <new>

using namespace skia_private;
using namespace SkKnownRuntimeEffects;

namespace skgpu::graphite {

static_assert(static_cast<int>(BuiltInCodeSnippetID::kLast) < kSkiaBuiltInReservedCnt);

// The toLinearSrgb and fromLinearSrgb RuntimeEffect intrinsics need to be able to map to and
// from the dst color space and linearSRGB. These are the 10 uniforms needed to allow that.
// These boil down to two copies of the kColorSpaceTransformUniforms uniforms. The first set
// for mapping to LinearSRGB and the second set for mapping from LinearSRGB.
static constexpr Uniform kRuntimeEffectColorSpaceTransformUniforms[] = {
        // to LinearSRGB
        { "flags_toLinear",          SkSLType::kInt },
        { "srcKind_toLinear",        SkSLType::kInt },
        { "gamutTransform_toLinear", SkSLType::kHalf3x3 },
        { "dstKind_toLinear",        SkSLType::kInt },
        { "csXformCoeffs_toLinear",  SkSLType::kHalf4x4 },
        // from LinearSRGB
        { "flags_fromLinear",          SkSLType::kInt },
        { "srcKind_fromLinear",        SkSLType::kInt },
        { "gamutTransform_fromLinear", SkSLType::kHalf3x3 },
        { "dstKind_fromLinear",        SkSLType::kInt },
        { "csXformCoeffs_fromLinear",  SkSLType::kHalf4x4 },
};

namespace {

const char* get_known_rte_name(StableKey key) {
    switch (key) {
#define M(type) case StableKey::k##type : return "KnownRuntimeEffect_" #type;
#define M1(type)
#define M2(type, initializer) case StableKey::k##type : return "KnownRuntimeEffect_" #type;
        SK_ALL_STABLEKEYS(M, M1, M2)
#undef M2
#undef M1
#undef M
    }

    SkUNREACHABLE;
}

std::string get_mangled_name(const std::string& baseName, int manglingSuffix) {
    return baseName + "_" + std::to_string(manglingSuffix);
}

std::string get_mangled_uniform_name(const ShaderInfo& shaderInfo,
                                     const Uniform& uniform,
                                     int manglingSuffix) {
    std::string result;

    if (uniform.isPaintColor()) {
        // Due to deduplication there will only ever be one of these
        result = uniform.name();
    } else {
        result = uniform.name() + std::string("_") + std::to_string(manglingSuffix);
    }
    if (shaderInfo.ssboIndex()) {
        result = EmitStorageBufferAccess("fs", shaderInfo.ssboIndex(), result.c_str());
    }
    return result;
}

std::string get_mangled_sampler_name(const TextureAndSampler& tex, int manglingSuffix) {
    return tex.name() + std::string("_") + std::to_string(manglingSuffix);
}

std::string append_default_snippet_arguments(const ShaderInfo& shaderInfo,
                                             const ShaderNode* node,
                                             const ShaderSnippet::Args& args,
                                             SkSpan<const std::string> childOutputs) {
    std::string code = "(";

    const char* separator = "";

    const ShaderSnippet* entry = node->entry();

    // Append prior-stage output color.
    if (entry->needsPriorStageOutput()) {
        code += args.fPriorStageOutput;
        separator = ", ";
    }

    // Append blender destination color.
    if (entry->needsBlenderDstColor()) {
        code += separator;
        code += args.fBlenderDstColor;
        separator = ", ";
    }

    // Append fragment coordinates.
    if (entry->needsLocalCoords()) {
        code += separator;
        code += args.fFragCoord;
        separator = ", ";
    }

    // Append uniform names.
    for (int i = 0; i < entry->fUniforms.size(); ++i) {
        code += separator;
        separator = ", ";
        code += get_mangled_uniform_name(shaderInfo, entry->fUniforms[i], node->keyIndex());
    }

    // Append samplers.
    for (int i = 0; i < entry->fTexturesAndSamplers.size(); ++i) {
        code += separator;
        code += get_mangled_sampler_name(entry->fTexturesAndSamplers[i], node->keyIndex());
        separator = ", ";
    }

    // Append child output names.
    for (const std::string& childOutputVar : childOutputs) {
        code += separator;
        separator = ", ";
        code += childOutputVar;
    }
    code.push_back(')');

    return code;
}

// If we have no children, the default expression just calls a built-in snippet with the signature:
//     half4 BuiltinFunctionName(/* default snippet arguments */);
//
// If we do have children, we will have created a glue function in the preamble and that is called
// instead. Its signature looks like this:
//     half4 BuiltinFunctionName_N(half4 inColor, half4 destColor, float2 pos);

std::string generate_default_expression(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node,
                                        const ShaderSnippet::Args& args) {
    if (node->numChildren() == 0) {
        // We don't have any children; return an expression which invokes the snippet directly.
        return node->entry()->fStaticFunctionName +
               append_default_snippet_arguments(shaderInfo, node, args, /*childOutputs=*/{});
    } else {
        // Return an expression which invokes the helper function from the preamble.
        std::string helperFnName =
                get_mangled_name(node->entry()->fName, node->keyIndex());
        return SkSL::String::printf(
                "%s(%.*s, %.*s, %.*s)",
                helperFnName.c_str(),
                (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
                (int)args.fBlenderDstColor.size(),  args.fBlenderDstColor.data(),
                (int)args.fFragCoord.size(),        args.fFragCoord.data());
    }
}

// Returns an expression to invoke this entry.
std::string emit_expression_for_entry(const ShaderInfo& shaderInfo,
                                      const ShaderNode* node,
                                      ShaderSnippet::Args args) {
    if (node->entry()->fExpressionGenerator) {
        return node->entry()->fExpressionGenerator(shaderInfo, node, args);
    } else {
        return generate_default_expression(shaderInfo, node, args);
    }
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 outColor%d`, where the %d is
// filled in with 'node->keyIndex()'.
std::string emit_glue_code_for_entry(const ShaderInfo& shaderInfo,
                                     const ShaderNode* node,
                                     const ShaderSnippet::Args& args,
                                     std::string* funcBody) {
    std::string expr = emit_expression_for_entry(shaderInfo, node, args);
    std::string outputVar = get_mangled_name("outColor", node->keyIndex());
    SkSL::String::appendf(funcBody,
                          "// [%d] %s\n"
                          "half4 %s = %s;",
                          node->keyIndex(),
                          node->entry()->fName,
                          outputVar.c_str(),
                          expr.c_str());
    return outputVar;
}

std::string emit_helper_function(const ShaderInfo& shaderInfo,
                                 const ShaderNode* node) {
    // Create a helper function that invokes each of the children, then calls the entry's snippet
    // and passes all the child outputs along as arguments.
    const ShaderSnippet* entry = node->entry();
    std::string helperFnName = get_mangled_name(entry->fName, node->keyIndex());
    std::string helperFn = SkSL::String::printf(
            "half4 %s(half4 inColor, half4 destColor, float2 pos) {",
            helperFnName.c_str());
    TArray<std::string> childOutputVarNames;
    const ShaderSnippet::Args args = {"inColor", "destColor", "pos"};
    for (const ShaderNode* child : node->children()) {
        // Emit glue code into our helper function body (i.e. lifting the child execution up front
        // so their outputs can be passed to the static module function for the node's snippet).
        childOutputVarNames.push_back(emit_glue_code_for_entry(shaderInfo, child, args, &helperFn));
    }

    // Finally, invoke the snippet from the helper function, passing uniforms and child outputs.
    std::string snippetArgList = append_default_snippet_arguments(shaderInfo, node,
                                                                  args, childOutputVarNames);
    SkSL::String::appendf(&helperFn,
                              "return %s%s;"
                          "}",
                          entry->fStaticFunctionName, snippetArgList.c_str());
    return helperFn;
}

// If we have no children, we don't need to add anything into the preamble.
// If we have child entries, we create a function in the preamble with a signature of:
//     half4 BuiltinFunctionName_N(half4 inColor, half4 destColor, float2 pos) { ... }
// This function invokes each child in sequence, and then calls the built-in function, passing all
// uniforms and child outputs along:
//     half4 BuiltinFunctionName(/* all uniforms as parameters */,
//                               /* all child output variable names as parameters */);
std::string generate_default_preamble(const ShaderInfo& shaderInfo,
                                      const ShaderNode* node) {
    if (node->numChildren() > 0) {
        // Create a helper function which invokes all the child snippets.
        return emit_helper_function(shaderInfo, node);
    } else {
        // We don't need a helper function
        return "";
    }
}

// Walk the node tree and generate all preambles, accumulating into 'preamble'.
void emit_preambles(const ShaderInfo& shaderInfo,
                    SkSpan<const ShaderNode*> nodes,
                    std::string treeLabel,
                    std::string* preamble) {
    for (int i = 0; i < SkTo<int>(nodes.size()); ++i) {
        const ShaderNode* node = nodes[i];
        std::string nodeLabel = std::to_string(i);
        std::string nextLabel = treeLabel.empty() ? nodeLabel
                                                  : (treeLabel + "<-" + nodeLabel);

        if (node->numChildren() > 0) {
            emit_preambles(shaderInfo, node->children(), nextLabel, preamble);
        }

        std::string nodePreamble = node->entry()->fPreambleGenerator
                ? node->entry()->fPreambleGenerator(shaderInfo, node)
                : generate_default_preamble(shaderInfo, node);
        if (!nodePreamble.empty()) {
            SkSL::String::appendf(preamble,
                                "// [%d]   %s: %s\n"
                                "%s\n",
                                node->keyIndex(), nextLabel.c_str(), node->entry()->fName,
                                nodePreamble.c_str());
        }
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

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
// ShaderInfo

ShaderInfo::ShaderInfo(UniquePaintParamsID id,
                       const ShaderCodeDictionary* dict,
                       const RuntimeEffectDictionary* rteDict,
                       const char* ssboIndex)
        : fRuntimeEffectDictionary(rteDict)
        , fSsboIndex(ssboIndex)
        , fSnippetRequirementFlags(SnippetRequirementFlags::kNone) {
    PaintParamsKey key = dict->lookup(id);
    SkASSERT(key.isValid()); // invalid keys should have been caught by invalid paint ID earlier

    fRootNodes = key.getRootNodes(dict, &fShaderNodeAlloc);
    // Aggregate snippet requirements across root nodes and look for fixed-function blend IDs in
    // the root to initialize the HW blend info.
    SkDEBUGCODE(bool fixedFuncBlendFound = false;)
    for (const ShaderNode* root : fRootNodes) {
        // If a snippet within this node tree requires additional sampler data to be stored, append
        // it to fData.
        this->aggregateSnippetData(root);

        // TODO: This is brittle as it relies on PaintParams::toKey() putting the final fixed
        // function blend block at the root level. This can be improved with more structure to the
        // key creation.
        if (root->codeSnippetId() < kBuiltInCodeSnippetIDCount &&
            root->codeSnippetId() >= kFixedFunctionBlendModeIDOffset) {
            SkASSERT(root->numChildren() == 0);
            // This should occur at most once
            SkASSERT(!fixedFuncBlendFound);
            SkDEBUGCODE(fixedFuncBlendFound = true;)

            fBlendMode = static_cast<SkBlendMode>(root->codeSnippetId() -
                                                  kFixedFunctionBlendModeIDOffset);
            SkASSERT(static_cast<int>(fBlendMode) >= 0 &&
                     fBlendMode <= SkBlendMode::kLastCoeffMode);
            fBlendInfo = gBlendTable[static_cast<int>(fBlendMode)];
        } else {
            fSnippetRequirementFlags |= root->requiredFlags();
        }
    }
}

void ShaderInfo::aggregateSnippetData(const ShaderNode* node) {
    if (!node) {
        return;
    }

    // Accumulate data of children first.
    for (const ShaderNode* child : node->children()) {
        this->aggregateSnippetData(child);
    }

    if (node->requiredFlags() & SnippetRequirementFlags::kStoresData && node->data().size()) {
        fData.push_back_n(node->data().size(), node->data().data());
    }
}

void append_color_output(std::string* mainBody,
                         BlendFormula::OutputType outputType,
                         const char* outColor,
                         const char* inColor) {
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            SkSL::String::appendf(mainBody, "%s = half4(0.0);", outColor);
            break;
        case BlendFormula::kCoverage_OutputType:
            SkSL::String::appendf(mainBody, "%s = outputCoverage;", outColor);
            break;
        case BlendFormula::kModulate_OutputType:
            SkSL::String::appendf(mainBody, "%s = %s * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kSAModulate_OutputType:
            SkSL::String::appendf(mainBody, "%s = %s.a * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kISAModulate_OutputType:
            SkSL::String::appendf(
                    mainBody, "%s = (1.0 - %s.a) * outputCoverage;", outColor, inColor);
            break;
        case BlendFormula::kISCModulate_OutputType:
            SkSL::String::appendf(
                    mainBody, "%s = (half4(1.0) - %s) * outputCoverage;", outColor, inColor);
            break;
        default:
            SkUNREACHABLE;
            break;
    }
}

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
std::string ShaderInfo::toSkSL(const Caps* caps,
                               const RenderStep* step,
                               bool useStorageBuffers,
                               int* numTexturesAndSamplersUsed,
                               int* numPaintUniforms,
                               int* renderStepUniformTotalBytes,
                               int* paintUniformsTotalBytes,
                               bool* hasGradientBuffer,
                               Swizzle writeSwizzle) {
    // If we're doing analytic coverage, we must also be doing shading.
    SkASSERT(step->coverage() == Coverage::kNone || step->performsShading());
    const bool hasStepUniforms = step->numUniforms() > 0 && step->coverage() != Coverage::kNone;
    const bool useStepStorageBuffer = useStorageBuffers && hasStepUniforms;
    const bool useShadingStorageBuffer = useStorageBuffers && step->performsShading();
    const bool useGradientStorageBuffer = caps->gradientBufferSupport() &&
                                          (fSnippetRequirementFlags
                                                & SnippetRequirementFlags::kGradientBuffer);

    const bool defineLocalCoordsVarying = this->needsLocalCoords();
    std::string preamble = EmitVaryings(step,
                                        /*direction=*/"in",
                                        /*emitSsboIndicesVarying=*/useShadingStorageBuffer,
                                        defineLocalCoordsVarying);

    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    if (hasStepUniforms) {
        if (useStepStorageBuffer) {
            preamble += EmitRenderStepStorageBuffer(bindingReqs.fRenderStepBufferBinding,
                                                    step->uniforms());
        } else {
            preamble += EmitRenderStepUniforms(bindingReqs.fRenderStepBufferBinding,
                                               bindingReqs.fUniformBufferLayout,
                                               step->uniforms(),
                                               renderStepUniformTotalBytes);
        }
    }

    bool wrotePaintColor = false;
    if (useShadingStorageBuffer) {
        preamble += EmitPaintParamsStorageBuffer(bindingReqs.fPaintParamsBufferBinding,
                                                 fRootNodes,
                                                 numPaintUniforms,
                                                 &wrotePaintColor);
        SkSL::String::appendf(&preamble, "uint %s;\n", this->ssboIndex());
    } else {
        preamble += EmitPaintParamsUniforms(bindingReqs.fPaintParamsBufferBinding,
                                            bindingReqs.fUniformBufferLayout,
                                            fRootNodes,
                                            numPaintUniforms,
                                            paintUniformsTotalBytes,
                                            &wrotePaintColor);
    }

    if (useGradientStorageBuffer) {
        SkSL::String::appendf(&preamble,
                              "layout (binding=%d) readonly buffer FSGradientBuffer {\n"
                              "    float %s[];\n"
                              "};\n",
                              bindingReqs.fGradientBufferBinding,
                              caps->shaderCaps()->fFloatBufferArrayName);
        *hasGradientBuffer = true;
    }

    {
        int binding = 0;
        preamble += EmitTexturesAndSamplers(bindingReqs, fRootNodes, &binding);
        if (step->hasTextures()) {
            preamble += step->texturesAndSamplersSkSL(bindingReqs, &binding);
        }

        // Report back to the caller how many textures and samplers are used.
        if (numTexturesAndSamplersUsed) {
            *numTexturesAndSamplersUsed = binding;
        }
    }

    if (step->emitsPrimitiveColor()) {
        // TODO: Define this in the main body, and then pass it down into snippets like we do with
        // the local coordinates varying.
        preamble += "half4 primitiveColor;";
    }

    // Emit preamble declarations and helper functions required for snippets. In the default case
    // this adds functions that bind a node's specific mangled uniforms to the snippet's
    // implementation in the SkSL modules.
    emit_preambles(*this, fRootNodes, /*treeLabel=*/"", &preamble);

    std::string mainBody = "void main() {";
    // Set initial color. This will typically be optimized out by SkSL in favor of the paint
    // specifying a color with a solid color shader.
    mainBody += "half4 initialColor = half4(0);";

    if (useShadingStorageBuffer) {
        SkSL::String::appendf(&mainBody,
                              "%s = %s.y;\n",
                              this->ssboIndex(),
                              RenderStep::ssboIndicesVarying());
    }

    if (step->emitsPrimitiveColor()) {
        mainBody += step->fragmentColorSkSL();
    }

    // While looping through root nodes to emit shader code, skip the clip shader node if it's found
    // and keep it to apply later during coverage calculation.
    const ShaderNode* clipShaderNode = nullptr;

    // Emit shader main body code, invoking each root node's expression, forwarding the previous
    // node's output to the next.
    static constexpr char kUnusedDstColor[] = "half4(1)";
    static constexpr char kUnusedLocalCoords[] = "float2(0)";
    ShaderSnippet::Args args = {"initialColor",
                                kUnusedDstColor,
                                this->needsLocalCoords() ? "localCoordsVar" : kUnusedLocalCoords};
    for (const ShaderNode* node : fRootNodes) {
        if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kClipShader) {
            SkASSERT(!clipShaderNode);
            clipShaderNode = node;
            continue;
        }
        // This exclusion of the final Blend can be removed once we've resolved the final
        // blend parenting issue w/in the key
        if (node->codeSnippetId() >= kBuiltInCodeSnippetIDCount ||
            node->codeSnippetId() < kFixedFunctionBlendModeIDOffset) {
            args.fPriorStageOutput = emit_glue_code_for_entry(*this, node, args, &mainBody);
        }
    }

    if (writeSwizzle != Swizzle::RGBA()) {
        SkSL::String::appendf(&mainBody, "%s = %s.%s;", args.fPriorStageOutput.c_str(),
                                                        args.fPriorStageOutput.c_str(),
                                                        writeSwizzle.asString().c_str());
    }

    const char* outColor = args.fPriorStageOutput.c_str();
    const Coverage coverage = step->coverage();
    if (coverage != Coverage::kNone || clipShaderNode) {
        if (useStepStorageBuffer) {
            SkSL::String::appendf(&mainBody,
                                  "uint stepSsboIndex = %s.x;\n",
                                  RenderStep::ssboIndicesVarying());
            mainBody += EmitUniformsFromStorageBuffer("step", "stepSsboIndex", step->uniforms());
        }

        mainBody += "half4 outputCoverage = half4(1);";
        mainBody += step->fragmentCoverageSkSL();

        if (clipShaderNode) {
            std::string clipShaderOutput =
                    emit_glue_code_for_entry(*this, clipShaderNode, args, &mainBody);
            SkSL::String::appendf(&mainBody, "outputCoverage *= %s.a;", clipShaderOutput.c_str());
        }

        // TODO: Determine whether draw is opaque and pass that to GetBlendFormula.
        BlendFormula coverageBlendFormula =
                coverage == Coverage::kLCD
                        ? skgpu::GetLCDBlendFormula(fBlendMode)
                        : skgpu::GetBlendFormula(
                                  /*isOpaque=*/false, /*hasCoverage=*/true, fBlendMode);

        if (this->needsSurfaceColor()) {
            // If this draw uses a non-coherent dst read, we want to keep the existing dst color (or
            // whatever has been previously drawn) when there's no coverage. This helps for batching
            // text draws that need to read from a dst copy for blends. However, this only helps the
            // case where the outer bounding boxes of each letter overlap and not two actual parts
            // of the text.
            DstReadRequirement dstReadReq = caps->getDstReadRequirement();
            if (dstReadReq == DstReadRequirement::kTextureCopy ||
                dstReadReq == DstReadRequirement::kTextureSample) {
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

            // Use originally-specified BlendInfo and blend with dst manually.
            SkSL::String::appendf(
                    &mainBody,
                    "sk_FragColor = %s * outputCoverage + surfaceColor * (1.0 - outputCoverage);",
                    outColor);
            if (coverage == Coverage::kLCD) {
                SkSL::String::appendf(
                        &mainBody,
                        "half3 lerpRGB = mix(surfaceColor.aaa, %s.aaa, outputCoverage.rgb);"
                        "sk_FragColor.a = max(max(lerpRGB.r, lerpRGB.g), lerpRGB.b);",
                        outColor);
            }

        } else {
            fBlendInfo = {coverageBlendFormula.equation(),
                          coverageBlendFormula.srcCoeff(),
                          coverageBlendFormula.dstCoeff(),
                          SK_PMColor4fTRANSPARENT,
                          coverageBlendFormula.modifiesDst()};

            if (coverage == Coverage::kLCD) {
                mainBody += "outputCoverage.a = max(max(outputCoverage.r, "
                                                       "outputCoverage.g), "
                                                   "outputCoverage.b);";
            }
            append_color_output(
                    &mainBody, coverageBlendFormula.primaryOutput(), "sk_FragColor", outColor);
            if (coverageBlendFormula.hasSecondaryOutput()) {
                append_color_output(&mainBody,
                                    coverageBlendFormula.secondaryOutput(),
                                    "sk_SecondaryFragColor",
                                    outColor);
            }
        }

    } else {
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", outColor);
    }
    mainBody += "}\n";

    return preamble + "\n" + mainBody;
}

//--------------------------------------------------------------------------------------------------
// ShaderCodeDictionary

UniquePaintParamsID ShaderCodeDictionary::findOrCreate(PaintParamsKeyBuilder* builder) {
    AutoLockBuilderAsKey keyView{builder};
    if (!keyView->isValid()) {
        return UniquePaintParamsID::InvalidID();
    }

    SkAutoSpinlock lock{fSpinLock};

    UniquePaintParamsID* existingEntry = fPaintKeyToID.find(*keyView);
    if (existingEntry) {
        SkASSERT(fIDToPaintKey[(*existingEntry).asUInt()] == *keyView);
        return *existingEntry;
    }

    // Detach from the builder and copy into the arena
    PaintParamsKey key = keyView->clone(&fArena);
    UniquePaintParamsID newID{SkTo<uint32_t>(fIDToPaintKey.size())};

    fPaintKeyToID.set(key, newID);
    fIDToPaintKey.push_back(key);
    return newID;
}

PaintParamsKey ShaderCodeDictionary::lookup(UniquePaintParamsID codeID) const {
    if (!codeID.isValid()) {
        return PaintParamsKey::Invalid();
    }

    SkAutoSpinlock lock{fSpinLock};
    SkASSERT(codeID.asUInt() < SkTo<uint32_t>(fIDToPaintKey.size()));
    return fIDToPaintKey[codeID.asUInt()];
}

SkSpan<const Uniform> ShaderCodeDictionary::getUniforms(BuiltInCodeSnippetID id) const {
    return fBuiltInCodeSnippets[(int) id].fUniforms;
}

const ShaderSnippet* ShaderCodeDictionary::getEntry(int codeSnippetID) const {
    if (codeSnippetID < 0) {
        return nullptr;
    }

    if (codeSnippetID < kBuiltInCodeSnippetIDCount) {
        return &fBuiltInCodeSnippets[codeSnippetID];
    }

    SkAutoSpinlock lock{fSpinLock};

    if (codeSnippetID >= kSkiaKnownRuntimeEffectsStart &&
        codeSnippetID < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt) {
        int knownRTECodeSnippetID = codeSnippetID - kSkiaKnownRuntimeEffectsStart;

        // TODO(b/238759147): if the snippet hasn't been initialized, get the SkRuntimeEffect and
        // initialize it here
        SkASSERT(fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID].fPreambleGenerator);
        return &fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID];
    }

    // TODO(b/238759147): handle Android and chrome known runtime effects

    if (codeSnippetID >= kUnknownRuntimeEffectIDStart) {
        int userDefinedCodeSnippetID = codeSnippetID - kUnknownRuntimeEffectIDStart;
        if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
            return &fUserDefinedCodeSnippets[userDefinedCodeSnippetID];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
namespace {

std::string GenerateDstReadSampleExpression(const ShaderInfo& shaderInfo,
                                            const ShaderNode* node,
                                            const ShaderSnippet::Args& args) {
    const ShaderSnippet* entry = node->entry();
    std::string sampler =
            get_mangled_sampler_name(entry->fTexturesAndSamplers[0], node->keyIndex());
    std::string coords =
            get_mangled_uniform_name(shaderInfo, entry->fUniforms[0], node->keyIndex());
    std::string helperFnName = get_mangled_name(entry->fName, node->keyIndex());

    return SkSL::String::printf("%s(%s, %s)",
                                helperFnName.c_str(),
                                coords.c_str(),
                                sampler.c_str());
}

std::string GenerateDstReadSamplePreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());

    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s(float4 coords, sampler2D dstSampler) {"
                "surfaceColor = sample(dstSampler, (sk_FragCoord.xy - coords.xy) * coords.zw);"
                "return surfaceColor;"
            "}",
            helperFnName.c_str());
}

//--------------------------------------------------------------------------------------------------
std::string GenerateDstReadFetchExpression(const ShaderInfo& shaderInfo,
                                           const ShaderNode* node,
                                           const ShaderSnippet::Args& args) {
    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());

    return SkSL::String::printf("%s()", helperFnName.c_str());
}

std::string GenerateDstReadFetchPreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());

    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s() {"
                "surfaceColor = sk_LastFragColor;"
                "return surfaceColor;"
            "}",
            helperFnName.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumClipShaderChildren = 1;

std::string GenerateClipShaderExpression(const ShaderInfo& shaderInfo,
                                         const ShaderNode* node,
                                         const ShaderSnippet::Args& args) {
    SkASSERT(node->numChildren() == kNumClipShaderChildren);
    static constexpr char kUnusedSrcColor[] = "half4(1)";
    static constexpr char kUnusedDstColor[] = "half4(1)";
    return emit_expression_for_entry(
            shaderInfo, node->child(0), {kUnusedSrcColor, kUnusedDstColor, "sk_FragCoord.xy"});
}

std::string GenerateClipShaderPreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    // No preamble is used for clip shaders. The child shader is called directly with sk_FragCoord.
    return "";
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumLocalMatrixShaderChildren = 1;

// Create a helper function that multiplies coordinates by a local matrix, invokes the child
// entry with those updated coordinates, and returns the result. This helper function meets the
// requirements for use with GenerateDefaultExpression, so there's no need to have a separate
// special GenerateLocalMatrixExpression.
std::string GenerateLocalMatrixPreamble(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShader);
    SkASSERT(node->numChildren() == kNumLocalMatrixShaderChildren);

    // Get the child's evaluation expression.
    static constexpr char kUnusedDestColor[] = "half4(1)";
    std::string childExpr = emit_expression_for_entry(shaderInfo, node->child(0),
                                                      {"inColor", kUnusedDestColor, "coords"});
    std::string localMatrixUni =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());

    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());
    return SkSL::String::printf("half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                                    "coords = (%s * coords.xy01).xy;"
                                    "return %s;"
                                "}",
                                helperFnName.c_str(),
                                localMatrixUni.c_str(),
                                childExpr.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumCoordClampShaderChildren = 1;

// Create a helper function that clamps the local coords to the subset, invokes the child
// entry with those updated coordinates, and returns the result. This helper function meets the
// requirements for use with GenerateDefaultExpression, so there's no need to have a separate
// special GenerateCoordClampExpression.
// TODO: this has a lot of overlap with GenerateLocalMatrixPreamble
std::string GenerateCoordClampPreamble(const ShaderInfo& shaderInfo,
                                       const ShaderNode* node) {
    SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordClampShader);
    SkASSERT(node->numChildren() == kNumCoordClampShaderChildren);

    // Get the child's evaluation expression.
    static constexpr char kUnusedDestColor[] = "half4(1)";
    std::string childExpr = emit_expression_for_entry(shaderInfo, node->child(0),
                                                      {"inColor", kUnusedDestColor, "coords"});

    std::string subsetUni =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());

    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());
    return SkSL::String::printf("half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                                    "coords = clamp(coords, %s.LT, %s.RB);"
                                    "return %s;"
                                "}",
                                helperFnName.c_str(),
                                subsetUni.c_str(),
                                subsetUni.c_str(),
                                childExpr.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumBlendShaderChildren = 3;

std::string GenerateBlendShaderPreamble(const ShaderInfo& shaderInfo,
                                        const ShaderNode* node) {
    // Children are src, dst, and blender
    SkASSERT(node->numChildren() == 3);

    // Create a helper function that invokes the src and dst children, then calls the blend child
    // with the src and dst results.
    std::string helperFn = SkSL::String::printf(
            "half4 %s(half4 inColor, half4 destColor, float2 pos) {",
            get_mangled_name(node->entry()->fName, node->keyIndex()).c_str());

    // Get src and dst colors.
    const ShaderSnippet::Args args = {"inColor", "destColor", "pos"};
    std::string srcVar = emit_glue_code_for_entry(shaderInfo, node->child(0), args, &helperFn);
    std::string dstVar = emit_glue_code_for_entry(shaderInfo, node->child(1), args, &helperFn);

    // Do the blend.
    static constexpr char kUnusedLocalCoords[] = "float2(0)";

    std::string blendResultVar = emit_glue_code_for_entry(
            shaderInfo, node->child(2), {srcVar, dstVar, kUnusedLocalCoords}, &helperFn);

    SkSL::String::appendf(&helperFn,
                              "return %s;"
                          "}",
                          blendResultVar.c_str());
    return helperFn;
}

//--------------------------------------------------------------------------------------------------
class GraphitePipelineCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    GraphitePipelineCallbacks(const ShaderInfo& shaderInfo,
                              const ShaderNode* node,
                              std::string* preamble,
                              const SkRuntimeEffect* effect)
            : fShaderInfo(shaderInfo)
            , fNode(node)
            , fPreamble(preamble)
            , fEffect(effect) {}

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        std::string result = get_mangled_name(std::string(decl->var()->name()), fNode->keyIndex());
        if (fShaderInfo.ssboIndex()) {
            result = EmitStorageBufferAccess("fs", fShaderInfo.ssboIndex(), result.c_str());
        }
        return result;
    }

    void defineFunction(const char* decl, const char* body, bool isMain) override {
        if (isMain) {
            SkSL::String::appendf(
                 fPreamble,
                 "half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                     "%s"
                 "}",
                 get_mangled_name(fNode->entry()->fName, fNode->keyIndex()).c_str(),
                 body);
        } else {
            SkSL::String::appendf(fPreamble, "%s {%s}\n", decl, body);
        }
    }

    void declareFunction(const char* decl) override {
        *fPreamble += std::string(decl);
    }

    void defineStruct(const char* definition) override {
        *fPreamble += std::string(definition);
    }

    void declareGlobal(const char* declaration) override {
        *fPreamble += std::string(declaration);
    }

    std::string sampleShader(int index, std::string coords) override {
        return emit_expression_for_entry(fShaderInfo, fNode->child(index),
                                         {"inColor", "destColor", coords});
    }

    std::string sampleColorFilter(int index, std::string color) override {
        return emit_expression_for_entry(fShaderInfo, fNode->child(index),
                                         {color, "destColor", "coords"});
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        return emit_expression_for_entry(fShaderInfo, fNode->child(index),
                                         {src, dst, "coords"});
    }

    std::string toLinearSrgb(std::string color) override {
        if (!SkRuntimeEffectPriv::UsesColorTransform(fEffect)) {
            return color;
        }

        color = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string helper = get_mangled_name("toLinearSRGB", fNode->keyIndex());
        std::string xformedColor = SkSL::String::printf("%s(%s)",
                                    helper.c_str(),
                                    color.c_str());
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }


    std::string fromLinearSrgb(std::string color) override {
        if (!SkRuntimeEffectPriv::UsesColorTransform(fEffect)) {
            return color;
        }

        color = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string helper = get_mangled_name("fromLinearSRGB", fNode->keyIndex());
        std::string xformedColor = SkSL::String::printf("%s(%s)",
                                                        helper.c_str(),
                                                        color.c_str());
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }

    std::string getMangledName(const char* name) override {
        return get_mangled_name(name, fNode->keyIndex());
    }

private:
    const ShaderInfo& fShaderInfo;
    const ShaderNode* fNode;
    std::string* fPreamble;
    const SkRuntimeEffect* fEffect;
};

std::string GenerateRuntimeShaderPreamble(const ShaderInfo& shaderInfo,
                                          const ShaderNode* node) {
    // Find this runtime effect in the runtime-effect dictionary.
    SkASSERT(node->codeSnippetId() >= kBuiltInCodeSnippetIDCount);
    const SkRuntimeEffect* effect;
    if (node->codeSnippetId() < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt) {
        effect = GetKnownRuntimeEffect(static_cast<StableKey>(node->codeSnippetId()));
    } else {
        SkASSERT(node->codeSnippetId() >= kUnknownRuntimeEffectIDStart);
        effect = shaderInfo.runtimeEffectDictionary()->find(node->codeSnippetId());
    }
    SkASSERT(effect);

    const SkSL::Program& program = SkRuntimeEffectPriv::Program(*effect);

    std::string preamble;
    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
        SkSL::String::appendf(
                &preamble,
                "half4 %s(half4 inColor) {"
                    "return sk_color_space_transform(inColor, %s, %s, %s, %s, %s);"
                "}",
                get_mangled_name("toLinearSRGB", node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[0],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[1],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[2],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[3],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[4],
                                         node->keyIndex()).c_str());
        SkSL::String::appendf(
                &preamble,
                "half4 %s(half4 inColor) {"
                    "return sk_color_space_transform(inColor, %s, %s, %s, %s, %s);"
                "}",
                get_mangled_name("fromLinearSRGB", node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[5],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[6],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[7],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[8],
                                         node->keyIndex()).c_str(),
                get_mangled_uniform_name(shaderInfo, kRuntimeEffectColorSpaceTransformUniforms[9],
                                         node->keyIndex()).c_str());
    }

    GraphitePipelineCallbacks callbacks{shaderInfo, node, &preamble, effect};
    SkSL::PipelineStage::ConvertProgram(program, "coords", "inColor", "destColor", &callbacks);
    return preamble;
}

std::string GenerateRuntimeShaderExpression(const ShaderInfo& shaderInfo,
                                            const ShaderNode* node,
                                            const ShaderSnippet::Args& args) {
    return SkSL::String::printf(
            "%s(%.*s, %.*s, %.*s)",
            get_mangled_name(node->entry()->fName, node->keyIndex()).c_str(),
            (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
            (int)args.fBlenderDstColor.size(),  args.fBlenderDstColor.data(),
            (int)args.fFragCoord.size(),        args.fFragCoord.data());
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumComposeChildren = 2;

// Compose two children, assuming the first child is the innermost.
std::string GenerateNestedChildrenPreamble(const ShaderInfo& shaderInfo,
                                           const ShaderNode* node) {
    SkASSERT(node->numChildren() == kNumComposeChildren);

    // Evaluate inner child.
    static constexpr char kUnusedDestColor[] = "half4(1)";
    std::string innerColor = emit_expression_for_entry(shaderInfo, node->child(0),
                                                       {"inColor", kUnusedDestColor, "coords"});

    // Evaluate outer child.
    std::string outerColor = emit_expression_for_entry(shaderInfo, node->child(1),
                                                       {innerColor, kUnusedDestColor, "coords"});

    // Create a helper function that invokes the inner expression, then passes that result to the
    // outer expression, and returns the composed result.
    std::string helperFnName = get_mangled_name(node->entry()->fName, node->keyIndex());
    return SkSL::String::printf("half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                                    "return %s;"
                                "}",
                                helperFnName.c_str(),
                                outerColor.c_str());
}

//--------------------------------------------------------------------------------------------------

std::string GeneratePrimitiveColorExpression(const ShaderInfo&,
                                             const ShaderNode* node,
                                             const ShaderSnippet::Args&) {
    return "primitiveColor";
}

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

#if defined(SK_DEBUG)
bool ShaderCodeDictionary::isValidID(int snippetID) const {
    if (snippetID < 0) {
        return false;
    }

    if (snippetID < kBuiltInCodeSnippetIDCount) {
        return true;
    }
    if (snippetID >= kSkiaKnownRuntimeEffectsStart && snippetID < kSkiaKnownRuntimeEffectsEnd) {
        return snippetID < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt;
    }

    SkAutoSpinlock lock{fSpinLock};

    if (snippetID >= kUnknownRuntimeEffectIDStart) {
        int userDefinedCodeSnippetID = snippetID - kUnknownRuntimeEffectIDStart;
        return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
    }

    return false;
}

void ShaderCodeDictionary::dump(UniquePaintParamsID id) const {
    this->lookup(id).dump(this, id);
}
#endif

static SkSLType uniform_type_to_sksl_type(const SkRuntimeEffect::Uniform& u) {
    using Type = SkRuntimeEffect::Uniform::Type;
    if (u.flags & SkRuntimeEffect::Uniform::kHalfPrecision_Flag) {
        switch (u.type) {
            case Type::kFloat:    return SkSLType::kHalf;
            case Type::kFloat2:   return SkSLType::kHalf2;
            case Type::kFloat3:   return SkSLType::kHalf3;
            case Type::kFloat4:   return SkSLType::kHalf4;
            case Type::kFloat2x2: return SkSLType::kHalf2x2;
            case Type::kFloat3x3: return SkSLType::kHalf3x3;
            case Type::kFloat4x4: return SkSLType::kHalf4x4;
            // NOTE: shorts cannot be uniforms, so we shouldn't ever get here.
            // Defensively return the full precision integer type.
            case Type::kInt:      SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt;
            case Type::kInt2:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt2;
            case Type::kInt3:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt3;
            case Type::kInt4:     SkDEBUGFAIL("unsupported uniform type"); return SkSLType::kInt4;
        }
    } else {
        switch (u.type) {
            case Type::kFloat:    return SkSLType::kFloat;
            case Type::kFloat2:   return SkSLType::kFloat2;
            case Type::kFloat3:   return SkSLType::kFloat3;
            case Type::kFloat4:   return SkSLType::kFloat4;
            case Type::kFloat2x2: return SkSLType::kFloat2x2;
            case Type::kFloat3x3: return SkSLType::kFloat3x3;
            case Type::kFloat4x4: return SkSLType::kFloat4x4;
            case Type::kInt:      return SkSLType::kInt;
            case Type::kInt2:     return SkSLType::kInt2;
            case Type::kInt3:     return SkSLType::kInt3;
            case Type::kInt4:     return SkSLType::kInt4;
        }
    }
    SkUNREACHABLE;
}

const char* ShaderCodeDictionary::addTextToArena(std::string_view text) {
    char* textInArena = fArena.makeArrayDefault<char>(text.size() + 1);
    memcpy(textInArena, text.data(), text.size());
    textInArena[text.size()] = '\0';
    return textInArena;
}

SkSpan<const Uniform> ShaderCodeDictionary::convertUniforms(const SkRuntimeEffect* effect) {
    using rteUniform = SkRuntimeEffect::Uniform;
    SkSpan<const rteUniform> uniforms = effect->uniforms();

    int numBaseUniforms = uniforms.size();
    int xtraUniforms = 0;
    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
        xtraUniforms += std::size(kRuntimeEffectColorSpaceTransformUniforms);
    }

    // Convert the SkRuntimeEffect::Uniform array into its Uniform equivalent.
    int numUniforms = numBaseUniforms + xtraUniforms;
    Uniform* uniformArray = fArena.makeInitializedArray<Uniform>(numUniforms, [&](int index) {
        if (index >= numBaseUniforms) {
            return kRuntimeEffectColorSpaceTransformUniforms[index - numBaseUniforms];
        }

        const rteUniform* u;
        u = &uniforms[index];

        // The existing uniform names live in the passed-in SkRuntimeEffect and may eventually
        // disappear. Copy them into fArena. (It's safe to do this within makeInitializedArray; the
        // entire array is allocated in one big slab before any initialization calls are done.)
        const char* name = this->addTextToArena(u->name);

        // Add one Uniform to our array.
        SkSLType type = uniform_type_to_sksl_type(*u);
        return (u->flags & rteUniform::kArray_Flag) ? Uniform(name, type, u->count)
                                                    : Uniform(name, type);
    });

    return SkSpan<const Uniform>(uniformArray, numUniforms);
}

ShaderSnippet ShaderCodeDictionary::convertRuntimeEffect(const SkRuntimeEffect* effect,
                                                         const char* name) {
    SkEnumBitMask<SnippetRequirementFlags> snippetFlags = SnippetRequirementFlags::kNone;
    if (effect->allowShader()) {
        // SkRuntimeEffect::usesSampleCoords() can't be used to restrict this because it returns
        // false when the only use is to pass the coord unmodified to a child. When children can
        // refer to interpolated varyings directly in this case, we can refine the flags.
        snippetFlags |= SnippetRequirementFlags::kLocalCoords;
    } else if (effect->allowColorFilter()) {
        snippetFlags |= SnippetRequirementFlags::kPriorStageOutput;
    } else if (effect->allowBlender()) {
        snippetFlags |= SnippetRequirementFlags::kPriorStageOutput; // src
        snippetFlags |= SnippetRequirementFlags::kBlenderDstColor;  // dst
    }

    return ShaderSnippet(name,
                         /*staticFn=*/nullptr,
                         snippetFlags,
                         this->convertUniforms(effect),
                         /*textures=*/{},
                         GenerateRuntimeShaderExpression,
                         GenerateRuntimeShaderPreamble,
                         (int) effect->children().size());
}

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
     SkAutoSpinlock lock{fSpinLock};

    if (int stableKey = SkRuntimeEffectPriv::StableKey(*effect)) {
        SkASSERT(stableKey >= kSkiaKnownRuntimeEffectsStart &&
                 stableKey < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt);

        int index = stableKey - kSkiaKnownRuntimeEffectsStart;

        if (!fKnownRuntimeEffectCodeSnippets[index].fExpressionGenerator) {
            const char* name = get_known_rte_name(static_cast<StableKey>(stableKey));
            fKnownRuntimeEffectCodeSnippets[index] = this->convertRuntimeEffect(effect, name);
        }

        return stableKey;
    }

    // Use the combination of {SkSL program hash, uniform size} as our key.
    // In the unfortunate event of a hash collision, at least we'll have the right amount of
    // uniform data available.
    RuntimeEffectKey key;
    key.fHash = SkRuntimeEffectPriv::Hash(*effect);
    key.fUniformSize = effect->uniformSize();

    int32_t* existingCodeSnippetID = fRuntimeEffectMap.find(key);
    if (existingCodeSnippetID) {
        return *existingCodeSnippetID;
    }

    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(this->convertRuntimeEffect(effect, "RuntimeEffect"));
    int newCodeSnippetID = kUnknownRuntimeEffectIDStart + fUserDefinedCodeSnippets.size() - 1;

    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
}

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fIDToPaintKey.push_back(PaintParamsKey::Invalid());

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kError] = {
            /*name=*/"Error",
            /*staticFn=*/"sk_error",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPriorOutput] = {
            /*name=*/"PassthroughShader",
            /*staticFn=*/"sk_passthrough",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSolidColorShader] = {
            /*name=*/"SolidColor",
            /*staticFn=*/"sk_solid_shader",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "color", SkSLType::kFloat4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRGBPaintColor] = {
            /*name=*/"RGBPaintColor",
            /*staticFn=*/"sk_rgb_opaque",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ Uniform::PaintColor() }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAlphaOnlyPaintColor] = {
            /*name=*/"AlphaOnlyPaintColor",
            /*staticFn=*/"sk_alpha_only",
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ Uniform::PaintColor() }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader4] = {
            /*name=*/"LinearGradient4",
            /*staticFn=*/"sk_linear_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader8] = {
            /*name=*/"LinearGradient8",
            /*staticFn=*/"sk_linear_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShaderTexture] = {
            /*name=*/"LinearGradientTexture",
            /*staticFn=*/"sk_linear_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShaderBuffer] = {
            /*name=*/"LinearGradientBuffer",
            /*staticFn=*/"sk_linear_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader4] = {
            /*name=*/"RadialGradient4",
            /*staticFn=*/ "sk_radial_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader8] = {
            /*name=*/"RadialGradient8",
            /*staticFn=*/"sk_radial_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShaderTexture] = {
            /*name=*/"RadialGradientTexture",
            /*staticFn=*/"sk_radial_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShaderBuffer] = {
            /*name=*/"RadialGradientBuffer",
            /*staticFn=*/"sk_radial_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader4] = {
            /*name=*/"SweepGradient4",
            /*staticFn=*/"sk_sweep_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "bias",        SkSLType::kFloat },
                           { "scale",       SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader8] = {
            /*name=*/"SweepGradient8",
            /*staticFn=*/"sk_sweep_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "bias",        SkSLType::kFloat },
                           { "scale",       SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShaderTexture] = {
            /*name=*/"SweepGradientTexture",
            /*staticFn=*/"sk_sweep_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "bias",        SkSLType::kFloat },
                            { "scale",      SkSLType::kFloat },
                            { "numStops",   SkSLType::kInt },
                            { "tilemode",   SkSLType::kInt },
                            { "colorSpace", SkSLType::kInt },
                            { "doUnPremul", SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShaderBuffer] = {
            /*name=*/"SweepGradientBuffer",
            /*staticFn=*/"sk_sweep_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "bias",         SkSLType::kFloat },
                           { "scale",        SkSLType::kFloat },
                           { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader4] = {
            /*name=*/"ConicalGradient4",
            /*staticFn=*/"sk_conical_grad_4_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 4 },
                           { "offsets",     SkSLType::kFloat4 },
                           { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader8] = {
            /*name=*/"ConicalGradient8",
            /*staticFn=*/"sk_conical_grad_8_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "colors",      SkSLType::kFloat4, 8 },
                           { "offsets",     SkSLType::kFloat4, 2 },
                           { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShaderTexture] = {
            /*name=*/"ConicalGradientTexture",
            /*staticFn=*/"sk_conical_grad_tex_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "radius0",     SkSLType::kFloat },
                           { "dRadius",     SkSLType::kFloat },
                           { "a",           SkSLType::kFloat },
                           { "invA",        SkSLType::kFloat },
                           { "numStops",    SkSLType::kInt },
                           { "tilemode",    SkSLType::kInt },
                           { "colorSpace",  SkSLType::kInt },
                           { "doUnPremul",  SkSLType::kInt } },
            /*textures=*/{"colorAndOffsetSampler"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShaderBuffer] = {
            /*name=*/"ConicalGradientBuffer",
            /*staticFn=*/"sk_conical_grad_buf_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kGradientBuffer,
            /*uniforms=*/{ { "radius0",      SkSLType::kFloat },
                           { "dRadius",      SkSLType::kFloat },
                           { "a",            SkSLType::kFloat },
                           { "invA",         SkSLType::kFloat },
                           { "numStops",     SkSLType::kInt },
                           { "bufferOffset", SkSLType::kInt },
                           { "tilemode",     SkSLType::kInt },
                           { "colorSpace",   SkSLType::kInt },
                           { "doUnPremul",   SkSLType::kInt } }
    };

    // This snippet operates on local coords if the child requires local coords (hence why it does
    // not mask off the child's local coord requirement), but does nothing if the child does not
    // actually use coordinates.
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShader] = {
            /*name=*/"LocalMatrixShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "localMatrix", SkSLType::kFloat4x4 } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateLocalMatrixPreamble,
            /*numChildren=*/kNumLocalMatrixShaderChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            /*name=*/"ImageShader",
            /*staticFn=*/"sk_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "filterMode",            SkSLType::kInt },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicImageShader] = {
            /*name=*/"CubicImageShader",
            /*staticFn=*/"sk_cubic_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "cubicCoeffs",           SkSLType::kHalf4x4 },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWImageShader] = {
            /*name=*/"HardwareImageShader",
            /*staticFn=*/"sk_hw_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           // The next 5 uniforms are for the color space transformation
                           { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kYUVImageShader] = {
            /*name=*/"YUVImageShader",
            /*staticFn=*/"sk_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",         SkSLType::kFloat2 },
                           { "invImgSizeUV",        SkSLType::kFloat2 },  // Relative to Y's texels
                           { "subset",              SkSLType::kFloat4 },
                           { "linearFilterUVInset", SkSLType::kFloat2 },
                           { "tilemodeX",           SkSLType::kInt },
                           { "tilemodeY",           SkSLType::kInt },
                           { "filterModeY",         SkSLType::kInt },
                           { "filterModeUV",        SkSLType::kInt },
                           { "channelSelectY",      SkSLType::kHalf4 },
                           { "channelSelectU",      SkSLType::kHalf4 },
                           { "channelSelectV",      SkSLType::kHalf4 },
                           { "channelSelectA",      SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",      SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate",   SkSLType::kHalf3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicYUVImageShader] = {
            /*name=*/"CubicYUVImageShader",
            /*staticFn=*/"sk_cubic_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",       SkSLType::kFloat2 },
                           { "invImgSizeUV",      SkSLType::kFloat2 },  // Relative to Y's texels
                           { "subset",            SkSLType::kFloat4 },
                           { "tilemodeX",         SkSLType::kInt },
                           { "tilemodeY",         SkSLType::kInt },
                           { "cubicCoeffs",       SkSLType::kHalf4x4 },
                           { "channelSelectY",    SkSLType::kHalf4 },
                           { "channelSelectU",    SkSLType::kHalf4 },
                           { "channelSelectV",    SkSLType::kHalf4 },
                           { "channelSelectA",    SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",    SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate", SkSLType::kHalf3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWYUVImageShader] = {
            /*name=*/"HWYUVImageShader",
            /*staticFn=*/"sk_hw_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",           SkSLType::kFloat2 },
                           { "invImgSizeUV",          SkSLType::kFloat2 }, // Relative to Y's texels
                           { "channelSelectY",        SkSLType::kHalf4 },
                           { "channelSelectU",        SkSLType::kHalf4 },
                           { "channelSelectV",        SkSLType::kHalf4 },
                           { "channelSelectA",        SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",        SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate",     SkSLType::kHalf3 } },
            /*textures=*/ {{ "samplerY" },
                           { "samplerU" },
                           { "samplerV" },
                           { "samplerA" }}
    };

    // Like the local matrix shader, this is a no-op if the child doesn't need coords
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoordClampShader] = {
            /*name=*/"CoordClampShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "subset", SkSLType::kFloat4 } },
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateCoordClampPreamble,
            /*numChildren=*/kNumCoordClampShaderChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDitherShader] = {
            /*name=*/"Dither",
            /*staticFn=*/"sk_dither",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "range", SkSLType::kHalf } },
            /*textures=*/{ { "ditherLUT" } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPerlinNoiseShader] = {
            /*name=*/"PerlinNoiseShader",
            /*staticFn=*/"sk_perlin_noise_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "baseFrequency", SkSLType::kFloat2 },
                           { "stitchData",    SkSLType::kFloat2 },
                           { "noiseType",     SkSLType::kInt },
                           { "numOctaves",    SkSLType::kInt },
                           { "stitching",     SkSLType::kInt } },
            /*textures=*/{ { "permutationsSampler" },
                           { "noiseSampler" } }
    };

    // SkColorFilter snippets
    // TODO(b/349572157): investigate the implications of having separate hlsa and rgba matrix
    // colorfilters. It may be that having them separate will not contribute to an explosion.
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kMatrixColorFilter] = {
            /*name=*/"MatrixColorFilter",
            /*staticFn=*/"sk_matrix_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "matrix",    SkSLType::kFloat4x4 },
                           { "translate", SkSLType::kFloat4 },
                           { "inHSL",     SkSLType::kInt } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kTableColorFilter] = {
            /*name=*/"TableColorFilter",
            /*staticFn=*/"sk_table_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{},
            /*textures=*/{ {"table"} }};
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kGaussianColorFilter] = {
            /*name=*/"GaussianColorFilter",
            /*staticFn=*/"sk_gaussian_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter] = {
            /*name=*/"ColorSpaceTransform",
            /*staticFn=*/"sk_color_space_transform",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "flags",          SkSLType::kInt },
                           { "srcKind",        SkSLType::kInt },
                           { "gamutTransform", SkSLType::kHalf3x3 },
                           { "dstKind",        SkSLType::kInt },
                           { "csXformCoeffs",  SkSLType::kHalf4x4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendShader] = {
            /*name=*/"BlendShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateBlendShaderPreamble,
            /*numChildren=*/kNumBlendShaderChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoeffBlender] = {
            /*name=*/"CoeffBlender",
            /*staticFn=*/"sk_coeff_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "coeffs", SkSLType::kHalf4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendModeBlender] = {
            /*name=*/"BlendModeBlender",
            /*staticFn=*/"sk_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "blendMode", SkSLType::kInt } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPrimitiveColor] = {
            /*name=*/"PrimitiveColor",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GeneratePrimitiveColorExpression
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadSample] = {
            /*name=*/"DstReadSample",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{ {"dstOffsetAndInvWH", SkSLType::kFloat4} },
            /*textures=*/{ {"dstCopy"} },
            GenerateDstReadSampleExpression,
            GenerateDstReadSamplePreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadFetch] = {
            /*name=*/"DstReadFetch",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateDstReadFetchExpression,
            GenerateDstReadFetchPreamble,
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kClipShader] = {
            /*name=*/"ClipShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateClipShaderExpression,
            GenerateClipShaderPreamble,
            /*numChildren=*/kNumClipShaderChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCompose] = {
            /*name=*/"Compose",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            /*expressionGenerator=*/nullptr,
            GenerateNestedChildrenPreamble,
            /*numChildren=*/kNumComposeChildren
    };

    // Fixed-function blend mode snippets are all the same, their functionality is entirely defined
    // by their unique code snippet IDs.
    for (int i = 0; i <= (int) SkBlendMode::kLastCoeffMode; ++i) {
        int ffBlendModeID = kFixedFunctionBlendModeIDOffset + i;
        fBuiltInCodeSnippets[ffBlendModeID] = {
                /*name=*/SkBlendMode_Name(static_cast<SkBlendMode>(i)),
                /*staticFn=*/skgpu::BlendFuncName(static_cast<SkBlendMode>(i)),
                SnippetRequirementFlags::kPriorStageOutput |
                SnippetRequirementFlags::kBlenderDstColor,
                /*uniforms=*/{}
        };
    }
}

// clang-format off

// Verify that the built-in code IDs for fixed function blending are consistent with SkBlendMode.
static_assert((int)SkBlendMode::kClear    == (int)BuiltInCodeSnippetID::kFixedFunctionClearBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrc      == (int)BuiltInCodeSnippetID::kFixedFunctionSrcBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDst      == (int)BuiltInCodeSnippetID::kFixedFunctionDstBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcOver  == (int)BuiltInCodeSnippetID::kFixedFunctionSrcOverBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstOver  == (int)BuiltInCodeSnippetID::kFixedFunctionDstOverBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcIn    == (int)BuiltInCodeSnippetID::kFixedFunctionSrcInBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstIn    == (int)BuiltInCodeSnippetID::kFixedFunctionDstInBlendMode    - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcOut   == (int)BuiltInCodeSnippetID::kFixedFunctionSrcOutBlendMode   - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstOut   == (int)BuiltInCodeSnippetID::kFixedFunctionDstOutBlendMode   - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kSrcATop  == (int)BuiltInCodeSnippetID::kFixedFunctionSrcATopBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kDstATop  == (int)BuiltInCodeSnippetID::kFixedFunctionDstATopBlendMode  - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kXor      == (int)BuiltInCodeSnippetID::kFixedFunctionXorBlendMode      - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kPlus     == (int)BuiltInCodeSnippetID::kFixedFunctionPlusBlendMode     - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kModulate == (int)BuiltInCodeSnippetID::kFixedFunctionModulateBlendMode - kFixedFunctionBlendModeIDOffset);
static_assert((int)SkBlendMode::kScreen   == (int)BuiltInCodeSnippetID::kFixedFunctionScreenBlendMode   - kFixedFunctionBlendModeIDOffset);

// Verify enum constants match values expected by static module SkSL functions
static_assert(0 == static_cast<int>(skcms_TFType_Invalid),   "ColorSpaceTransform code depends on skcms_TFType");
static_assert(1 == static_cast<int>(skcms_TFType_sRGBish),   "ColorSpaceTransform code depends on skcms_TFType");
static_assert(2 == static_cast<int>(skcms_TFType_PQish),     "ColorSpaceTransform code depends on skcms_TFType");
static_assert(3 == static_cast<int>(skcms_TFType_HLGish),    "ColorSpaceTransform code depends on skcms_TFType");
static_assert(4 == static_cast<int>(skcms_TFType_HLGinvish), "ColorSpaceTransform code depends on skcms_TFType");

// TODO: We can meaningfully check these when we can use C++20 features.
// static_assert(0x1  == SkColorSpaceXformSteps::Flags{.unpremul = true}.mask(),        "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x2  == SkColorSpaceXformSteps::Flags{.linearize = true}.mask(),       "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x4  == SkColorSpaceXformSteps::Flags{.gamut_transform = true}.mask(), "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x8  == SkColorSpaceXformSteps::Flags{.encode = true}.mask(),          "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x10 == SkColorSpaceXformSteps::Flags{.premul = true}.mask(),          "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");

static_assert(0 == static_cast<int>(SkTileMode::kClamp),  "ImageShader code depends on SkTileMode");
static_assert(1 == static_cast<int>(SkTileMode::kRepeat), "ImageShader code depends on SkTileMode");
static_assert(2 == static_cast<int>(SkTileMode::kMirror), "ImageShader code depends on SkTileMode");
static_assert(3 == static_cast<int>(SkTileMode::kDecal),  "ImageShader code depends on SkTileMode");

static_assert(0 == static_cast<int>(SkFilterMode::kNearest), "ImageShader code depends on SkFilterMode");
static_assert(1 == static_cast<int>(SkFilterMode::kLinear),  "ImageShader code depends on SkFilterMode");

// clang-format on

} // namespace skgpu::graphite
