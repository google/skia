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

std::string get_mangled_struct_reference(const ShaderInfo& shaderInfo,
                                         const ShaderNode* node) {
    SkASSERT(node->entry()->fUniformStructName);
    std::string result = "node_" + std::to_string(node->keyIndex()); // Field holding the struct
    if (shaderInfo.ssboIndex()) {
        result = EmitStorageBufferAccess("fs", shaderInfo.ssboIndex(), result.c_str());
    }
    return result;
}

std::string stitch_csv(SkSpan<const std::string> args) {
    std::string code = "";
    const char* separator = "";
    for (const std::string& arg : args) {
        code += separator;
        code += arg;
        separator = ", ";
    }

    return code;
}

static const ShaderSnippet::Args kDefaultArgs{"inColor", "destColor", "pos"};

// If 'args' is null, the generated list is assumed to be for parameter declarations. If it's non
// null, it is assumed to be the expressions to invoke the default signature.
void append_defaults(TArray<std::string>* list,
                     const ShaderNode* node,
                     const ShaderSnippet::Args* args) {
    // Use the node's aggregate required flags so that the provided dynamic variables propagate
    // to the child nodes that require them.
    if (node->requiredFlags() & SnippetRequirementFlags::kPriorStageOutput) {
        list->push_back(args ? args->fPriorStageOutput.c_str() : "half4 inColor");
    }
    if (node->requiredFlags() & SnippetRequirementFlags::kBlenderDstColor) {
        list->push_back(args ? args->fBlenderDstColor.c_str() : "half4 destColor");
    }
    if (node->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        list->push_back(args ? args->fFragCoord.c_str() : "float2 pos");
    }

    // Special variables and/or "global" scope variables that have to propagate
    // through the node tree.
    if (node->requiredFlags() & SnippetRequirementFlags::kPrimitiveColor) {
        list->push_back(args ? "primitiveColor" : "half4 primitiveColor");
    }
}

static const char* kGradientBufferName = "fsGradientBuffer";

void append_uniforms(TArray<std::string>* list,
                     const ShaderInfo& shaderInfo,
                     const ShaderNode* node,
                     SkSpan<const std::string> childOutputs) {
    const ShaderSnippet* entry = node->entry();

    if (entry->fUniformStructName) {
        // The node's uniforms are aggregated in a sub-struct within the global uniforms so we just
        // need to append a reference to the node's instance
        list->push_back(get_mangled_struct_reference(shaderInfo, node));
    } else {
        // The uniforms are in the global scope, so just pass in the ones bound to 'node'
        for (int i = 0; i < entry->fUniforms.size(); ++i) {
            list->push_back(get_mangled_uniform_name(shaderInfo,
                                                     entry->fUniforms[i],
                                                     node->keyIndex()));
        }
    }

    // Append samplers
    for (int i = 0; i < entry->fTexturesAndSamplers.size(); ++i) {
        list->push_back(get_mangled_sampler_name(entry->fTexturesAndSamplers[i], node->keyIndex()));
    }

    // Append gradient buffer.
    if (node->requiredFlags() & SnippetRequirementFlags::kGradientBuffer) {
        list->push_back(kGradientBufferName);
    }

    // Append child output names.
    if (!childOutputs.empty()) {
        list->push_back_n(childOutputs.size(), childOutputs.data());
    }
}

// If we have no children, the default expression just calls a built-in snippet with the signature:
//     half4 BuiltinFunctionName(/* required variable inputs (e.g. float2 pos) */,
//                               /* all uniforms as parameters (bound to node's values) */) { ... }
// If we do have children, we will have created a glue function in the preamble and that is called
// instead. Its signature looks like this:
//     half4 SnippetName_N(/* required variable inputs (e.g. float2 pos) */) { ... }
std::string invoke_node(const ShaderInfo& shaderInfo,
                        const ShaderNode* node,
                        const ShaderSnippet::Args& args) {
    std::string fnName;
    STArray<3, std::string> params; // 1-2 inputs and a uniform struct or texture

    if (node->numChildren() == 0 && node->entry()->fStaticFunctionName) {
        // We didn't generate a helper function in the preamble, so add uniforms to the parameter
        // list and call the static function directly.
        fnName = node->entry()->fStaticFunctionName;
        append_defaults(&params, node, &args);
        append_uniforms(&params, shaderInfo, node, /*childOutputs=*/{});
    } else {
        // Invoke the generated helper function added to the preamble, which will handle invoking
        // any children and appending their values to the rest of the static fn's arguments.
        fnName = get_mangled_name(node->entry()->fName, node->keyIndex());
        append_defaults(&params, node, &args);
    }

    return SkSL::String::printf("%s(%s)", fnName.c_str(), stitch_csv(params).c_str());
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 outColor%d`, where the %d is
// filled in with 'node->keyIndex()'.
std::string invoke_and_assign_node(const ShaderInfo& shaderInfo,
                                   const ShaderNode* node,
                                   const ShaderSnippet::Args& args,
                                   std::string* funcBody) {
    std::string expr = invoke_node(shaderInfo, node, args);
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

// Emit a declaration for a helper function that represents the ShaderNode (named using the node's
// mangled name). The dynamic parameters are declared to match kDefaultArgs. The returned string
// can either be followed by a "{ body }" to fully define it or a ";" for a forward declaration.
std::string emit_helper_declaration(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    const ShaderSnippet* entry = node->entry();
    std::string helperFnName = get_mangled_name(entry->fName, node->keyIndex());

    STArray<3, std::string> params;
    append_defaults(&params, node, /*args=*/nullptr); // null args emits declarations

    return SkSL::String::printf("half4 %s(%s)", helperFnName.c_str(), stitch_csv(params).c_str());
}

// If we have no children, we don't need to add anything into the preamble.
// If we have child entries, we create a function in the preamble with a signature of:
//     half4 SnippetName_N(/* required variable inputs (e.g. float2 pos) */) { ... }
// This function invokes each child in sequence, and then calls the built-in function, passing all
// uniforms and child outputs along:
//     half4 BuiltinFunctionName(/* required variable inputs (e.g. float2 pos) */,
//                               /* all uniforms as parameters */,
//                               /* all child output variable names as parameters */);
std::string generate_default_preamble(const ShaderInfo& shaderInfo,
                                      const ShaderNode* node) {
    if (node->numChildren() == 0) {
        // We don't need a helper function to wrap the snippet's static function
        return "";
    }

    std::string code = emit_helper_declaration(shaderInfo, node) + " {";

    // Invoke each child with unmodified input values and collect in a list of local variables
    STArray<2, std::string> childOutputVarNames;
    for (const ShaderNode* child : node->children()) {
        // Emit glue code into our helper function body (i.e. lifting the child execution up front
        // so their outputs can be passed to the static module function for the node's snippet).
        childOutputVarNames.push_back(
                invoke_and_assign_node(shaderInfo, child, kDefaultArgs, &code));
    }

    // Finally, invoke the snippet from the helper function, passing uniforms and child outputs.
    STArray<3, std::string> params;
    append_defaults(&params, node, &kDefaultArgs);
    append_uniforms(&params, shaderInfo, node, childOutputVarNames);

    SkSL::String::appendf(&code,
                              "return %s(%s);"
                          "}",
                          node->entry()->fStaticFunctionName,
                          stitch_csv(params).c_str());
    return code;
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
                               bool* hasPaintUniforms,
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
                                               step->uniforms());
        }
    }

    bool wrotePaintColor = false;
    if (useShadingStorageBuffer) {
        preamble += EmitPaintParamsStorageBuffer(bindingReqs.fPaintParamsBufferBinding,
                                                 fRootNodes,
                                                 hasPaintUniforms,
                                                 &wrotePaintColor);
        SkSL::String::appendf(&preamble, "uint %s;\n", this->ssboIndex());
    } else {
        preamble += EmitPaintParamsUniforms(bindingReqs.fPaintParamsBufferBinding,
                                            bindingReqs.fUniformBufferLayout,
                                            fRootNodes,
                                            hasPaintUniforms,
                                            &wrotePaintColor);
    }

    if (useGradientStorageBuffer) {
        SkSL::String::appendf(&preamble,
                              "layout (binding=%d) readonly buffer FSGradientBuffer {\n"
                              "    float %s[];\n"
                              "};\n",
                              bindingReqs.fGradientBufferBinding,
                              kGradientBufferName);
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

    // While looping through root nodes to emit shader code, skip the clip shader node if it's found
    // and keep it to apply later during coverage calculation.
    const ShaderNode* clipShaderNode = nullptr;

    // Using kDefaultArgs as the initial value means it will refer to undefined variables, but the
    // root nodes should--at most--be depending on the coordinate when "needsLocalCoords" is true.
    // If the PaintParamsKey violates that structure, this will produce SkSL compile errors.
    ShaderSnippet::Args args = kDefaultArgs;
    args.fFragCoord = "localCoordsVar"; // the varying added in EmitVaryings()
    // TODO(b/349997190): The paint root node should not depend on any prior stage's output, but
    // it can happen with how SkEmptyShader is currently mapped to `sk_passthrough`. In this case
    // it requires that prior stage color to be transparent black. When SkEmptyShader can instead
    // cause the draw to be skipped, this can go away.
    args.fPriorStageOutput = "half4(0)";

    // Emit shader main body code, invoking each root node's expression, forwarding the previous
    // node's output to the next.
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
            args.fPriorStageOutput = invoke_and_assign_node(*this, node, args, &mainBody);
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
            // The clip shader node is invoked with fragment coords, not local coords like the main
            // shading root node.
            // TODO: The actual clipShaderNode can go away once we can enforce that a PaintParamsKey
            // has only 1-2 roots and the 2nd root is always the clip node.
            args.fFragCoord = "sk_FragCoord.xy";
            std::string clipShaderOutput =
                    invoke_and_assign_node(*this, clipShaderNode->child(0), args, &mainBody);
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
                SkASSERT(caps->shaderCaps()->fDualSourceBlendingSupport);
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

// NOTE: The dst-read snippets have 0 children and could be described by a static module function
// except that for now they need to stash the read surfaceColor in a global variable. Instead of
// generating a mangled preamble helper function, these preambles just add a "static" function
// that can be called with the default expression generator. Since there should only ever be one
// dst-read snippet in a paint, the lack of mangling will detect if that property is violated.
std::string GenerateDstReadSamplePreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s(float4 coords, sampler2D dstSampler) {"
                "surfaceColor = sample(dstSampler, (sk_FragCoord.xy - coords.xy) * coords.zw);"
                "return surfaceColor;"
            "}",
            node->entry()->fStaticFunctionName);
}

std::string GenerateDstReadFetchPreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    return SkSL::String::printf(
            "half4 surfaceColor;"  // we save off the original dstRead color to combine w/ coverage
            "half4 %s() {"
                "surfaceColor = sk_LastFragColor;"
                "return surfaceColor;"
            "}",
            node->entry()->fStaticFunctionName);
}

//--------------------------------------------------------------------------------------------------

std::string GenerateClipShaderPreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    // No preamble is used for clip shaders. The child shader is called directly with sk_FragCoord.
    return "";
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumCoordinateManipulateChildren = 1;

// Create a helper function that manipulates the coordinates passed into a child. The specific
// manipulation is pre-determined by the code id (local matrix or clamp). This helper function meets
// the requirements for use with GenerateDefaultExpression, so there's no need to have a separate
// special GenerateLocalMatrixExpression.
// TODO: This is effectively GenerateComposePreamble except that 'node' is counting as the inner.
std::string GenerateCoordManipulationPreamble(const ShaderInfo& shaderInfo,
                                              const ShaderNode* node) {
    SkASSERT(node->numChildren() == kNumCoordinateManipulateChildren);

    std::string perspectiveStatement;

    ShaderSnippet::Args localArgs = kDefaultArgs;
    if (node->child(0)->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        std::string controlUni =
                get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());

        if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShader) {
            localArgs.fFragCoord = SkSL::String::printf("(%s * %s.xy01).xy",
                                                        controlUni.c_str(),
                                                        kDefaultArgs.fFragCoord.c_str());
        } else if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShaderPersp) {
            perspectiveStatement = SkSL::String::printf("float4 perspCoord = %s * %s.xy01;",
                                                        controlUni.c_str(),
                                                        kDefaultArgs.fFragCoord.c_str());
            localArgs.fFragCoord = "perspCoord.xy / perspCoord.w";
        } else {
            SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordClampShader);
            localArgs.fFragCoord = SkSL::String::printf("clamp(%s, %s.LT, %s.RB)",
                                                        kDefaultArgs.fFragCoord.c_str(),
                                                        controlUni.c_str(), controlUni.c_str());
        }
    } // else this is a no-op

    std::string decl = emit_helper_declaration(shaderInfo, node);
    std::string invokeChild = invoke_node(shaderInfo, node->child(0), localArgs);
    return SkSL::String::printf("%s { %s return %s; }",
                                decl.c_str(),
                                perspectiveStatement.c_str(),
                                invokeChild.c_str());
}

//--------------------------------------------------------------------------------------------------

// Compose N-1 children into the Nth child, must have at least two children. The ith child provides
// the value for the ith enabled ShaderSnippet::Arg.
std::string GenerateComposePreamble(const ShaderInfo& shaderInfo, const ShaderNode* node) {
    SkASSERT(node->numChildren() >= 2);

    const ShaderNode* outer = node->child(node->numChildren() - 1);

#if defined(SK_DEBUG)
    const int numOuterParameters =
            SkToBool((outer->requiredFlags() & SnippetRequirementFlags::kPriorStageOutput)) +
            SkToBool((outer->requiredFlags() & SnippetRequirementFlags::kBlenderDstColor)) +
            SkToBool((outer->requiredFlags() & SnippetRequirementFlags::kLocalCoords));
    SkASSERT(node->numChildren() == numOuterParameters + 1);
#endif

    ShaderSnippet::Args outerArgs = kDefaultArgs;
    int child = 0;
    if (outer->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        outerArgs.fFragCoord = invoke_node(shaderInfo, node->child(child++), kDefaultArgs);
    }
    if (outer->requiredFlags() & SnippetRequirementFlags::kPriorStageOutput) {
        outerArgs.fPriorStageOutput = invoke_node(shaderInfo, node->child(child++), kDefaultArgs);
    }
    if (outer->requiredFlags() & SnippetRequirementFlags::kBlenderDstColor) {
        outerArgs.fBlenderDstColor = invoke_node(shaderInfo, node->child(child++), kDefaultArgs);
    }

    std::string decl = emit_helper_declaration(shaderInfo, node);
    std::string invokeOuter = invoke_node(shaderInfo, outer, outerArgs);
    return SkSL::String::printf("%s { return %s; }", decl.c_str(), invokeOuter.c_str());
}

//--------------------------------------------------------------------------------------------------
class GraphitePipelineCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    GraphitePipelineCallbacks(const ShaderInfo& shaderInfo,
                              const ShaderNode* node,
                              std::string* preamble,
                              [[maybe_unused]] const SkRuntimeEffect* effect)
            : fShaderInfo(shaderInfo)
            , fNode(node)
            , fPreamble(preamble) {
        SkDEBUGCODE(fEffect = effect;)
    }

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
                    "%s { %s }",
                    emit_helper_declaration(fShaderInfo, fNode).c_str(),
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
        ShaderSnippet::Args args = kDefaultArgs;
        args.fFragCoord = coords;
        return invoke_node(fShaderInfo, fNode->child(index), args);
    }

    std::string sampleColorFilter(int index, std::string color) override {
        ShaderSnippet::Args args = kDefaultArgs;
        args.fPriorStageOutput = color;
        return invoke_node(fShaderInfo, fNode->child(index), args);
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        ShaderSnippet::Args args = kDefaultArgs;
        args.fPriorStageOutput = src;
        args.fBlenderDstColor = dst;
        return invoke_node(fShaderInfo, fNode->child(index), args);
    }

    std::string toLinearSrgb(std::string color) override {
        SkASSERT(SkRuntimeEffectPriv::UsesColorTransform(fEffect));
        // If we use color transforms (e.g. reference [to|from]LinearSrgb(), we dynamically add two
        // children to the runtime effect's node after all explicitly declared children. The
        // conversion *to* linear srgb is the second-to-last child node, and the conversion *from*
        // linear srgb is the last child node.)
        const ShaderNode* toLinearSrgbNode = fNode->child(fNode->numChildren() - 2);
        SkASSERT(toLinearSrgbNode->codeSnippetId() ==
                        (int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter);

        ShaderSnippet::Args args = kDefaultArgs;
        args.fPriorStageOutput = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string xformedColor = invoke_node(fShaderInfo, toLinearSrgbNode, args);
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }


    std::string fromLinearSrgb(std::string color) override {
        SkASSERT(SkRuntimeEffectPriv::UsesColorTransform(fEffect));
        // If we use color transforms (e.g. reference [to|from]LinearSrgb()), we dynamically add two
        // children to the runtime effect's node after all explicitly declared children. The
        // conversion *to* linear srgb is the second-to-last child node, and the conversion *from*
        // linear srgb is the last child node.
        const ShaderNode* fromLinearSrgbNode = fNode->child(fNode->numChildren() - 1);
        SkASSERT(fromLinearSrgbNode->codeSnippetId() ==
                        (int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter);

        ShaderSnippet::Args args = kDefaultArgs;
        args.fPriorStageOutput = SkSL::String::printf("(%s).rgb1", color.c_str());
        std::string xformedColor = invoke_node(fShaderInfo, fromLinearSrgbNode, args);
        return SkSL::String::printf("(%s).rgb", xformedColor.c_str());
    }

    std::string getMangledName(const char* name) override {
        return get_mangled_name(name, fNode->keyIndex());
    }

private:
    const ShaderInfo& fShaderInfo;
    const ShaderNode* fNode;
    std::string* fPreamble;
    SkDEBUGCODE(const SkRuntimeEffect* fEffect;)
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
    GraphitePipelineCallbacks callbacks{shaderInfo, node, &preamble, effect};
    SkSL::PipelineStage::ConvertProgram(program,
                                        kDefaultArgs.fFragCoord.c_str(),
                                        kDefaultArgs.fPriorStageOutput.c_str(),
                                        kDefaultArgs.fBlenderDstColor.c_str(),
                                        &callbacks);
    return preamble;
}

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

    const int numUniforms = uniforms.size();

    // Convert the SkRuntimeEffect::Uniform array into its Uniform equivalent.
    Uniform* uniformArray = fArena.makeInitializedArray<Uniform>(numUniforms, [&](int index) {
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

    // If the runtime effect references toLinearSrgb() or fromLinearSrgb(), we append two
    // color space transform children that are invoked when converting those "built-in" expressions.
    int numChildrenIncColorTransforms = SkTo<int>(effect->children().size()) +
                                        (SkRuntimeEffectPriv::UsesColorTransform(effect) ? 2 : 0);

    // TODO: We can have the custom runtime effect preamble generator define structs for its
    // uniforms if it has a lot of uniforms, and then calculate the required alignment here.
    return ShaderSnippet(name,
                         /*staticFn=*/nullptr,
                         snippetFlags,
                         this->convertUniforms(effect),
                         /*textures=*/{},
                         GenerateRuntimeShaderPreamble,
                         numChildrenIncColorTransforms);
}

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
     SkAutoSpinlock lock{fSpinLock};

    if (int stableKey = SkRuntimeEffectPriv::StableKey(*effect)) {
        SkASSERT(stableKey >= kSkiaKnownRuntimeEffectsStart &&
                 stableKey < kSkiaKnownRuntimeEffectsStart + kStableKeyCnt);

        int index = stableKey - kSkiaKnownRuntimeEffectsStart;

        if (!fKnownRuntimeEffectCodeSnippets[index].fPreambleGenerator) {
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

ShaderCodeDictionary::ShaderCodeDictionary(Layout layout)
        : fLayout(layout) {
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
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "localMatrix", SkSLType::kFloat4x4 } },
            /*textures=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShaderPersp] = {
            /*name=*/"LocalMatrixShaderPersp",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "localMatrix", SkSLType::kFloat4x4 } },
            /*textures=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            /*name=*/"ImageShader",
            /*staticFn=*/"sk_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "filterMode",            SkSLType::kInt } },
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
                           { "cubicCoeffs",           SkSLType::kHalf4x4 } },
            /*textures=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWImageShader] = {
            /*name=*/"HardwareImageShader",
            /*staticFn=*/"sk_hw_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 } },
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

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWYUVNoSwizzleImageShader] = {
            /*name=*/"HWYUVImageShader",
            /*staticFn=*/"sk_hw_yuv_no_swizzle_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",              SkSLType::kFloat2 },
                           { "invImgSizeUV",             SkSLType::kFloat2 }, // Relative to Y space
                           { "yuvToRGBMatrix",           SkSLType::kHalf3x3 },
                           { "yuvToRGBXlateAlphaParams", SkSLType::kHalf4 } },
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
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren
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
                           { "inHSL",     SkSLType::kInt },
                           { "clampRGB",  SkSLType::kInt } }
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
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPremulAlphaColorFilter] = {
            /*name=*/"PremulAlpha",
            /*staticFn=*/"sk_premul_alpha",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendShader] = {
            /*name=*/"BlendShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateComposePreamble,
            /*numChildren=*/3
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
            /*staticFn=*/"sk_color_space_transform",
            SnippetRequirementFlags::kPrimitiveColor,
            /*uniforms=*/{ { "csXformFlags",          SkSLType::kInt },
                           { "csXformSrcKind",        SkSLType::kInt },
                           { "csXformGamutTransform", SkSLType::kHalf3x3 },
                           { "csXformDstKind",        SkSLType::kInt },
                           { "csXformCoeffs",         SkSLType::kHalf4x4 } },
            /*textures=*/{}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadSample] = {
            /*name=*/"DstReadSample",
            /*staticFn=*/"$dst_read_sample", // "static" function injected by custom preamble
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{ {"dstOffsetAndInvWH", SkSLType::kFloat4} },
            /*textures=*/{ {"dstCopy"} },
            GenerateDstReadSamplePreamble,
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDstReadFetch] = {
            /*name=*/"DstReadFetch",
            /*staticFn=*/"$dst_read_fetch", // "static" function injected by custom preamble
            SnippetRequirementFlags::kSurfaceColor,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateDstReadFetchPreamble,
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kClipShader] = {
            /*name=*/"ClipShader",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateClipShaderPreamble,
            /*numChildren=*/1
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCircularRRectClip] = {
            /*name=*/"CircularRRectClip",
            /*staticFn=*/"sk_circular_rrect_clip",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "rect",           SkSLType::kFloat4 },
                           { "radiusPlusHalf", SkSLType::kHalf2 },
                           { "edgeSelect",     SkSLType::kHalf4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCompose] = {
            /*name=*/"Compose",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{},
            /*textures=*/{},
            GenerateComposePreamble,
            /*numChildren=*/2
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

    // Complete layout calculations for builtin snippets
    for (int i = 0; i < kBuiltInCodeSnippetIDCount; ++i) {
        ShaderSnippet& snippet = fBuiltInCodeSnippets[i];
        SkASSERT(snippet.fName); // Should not have missed a built-in

        if (snippet.fUniformStructName) {
            auto offsetCalculator = UniformOffsetCalculator::ForStruct(fLayout);
            for (int j = 0; j < snippet.fUniforms.size(); ++j) {
                SkASSERT(!snippet.fUniforms[j].isPaintColor()); // paint color shouldn't be embedded
                offsetCalculator.advanceOffset(snippet.fUniforms[j].type(),
                                               snippet.fUniforms[j].count());
            }
            snippet.fRequiredAlignment = offsetCalculator.requiredAlignment();
        }
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
