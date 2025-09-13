/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ShaderCodeDictionary.h"

#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

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

std::string get_storage_buffer_access(const char* bufferNamePrefix,
                                      const char* ssboIndex,
                                      const char* uniformName) {
    return SkSL::String::printf("%sUniformData[%s].%s", bufferNamePrefix, ssboIndex, uniformName);
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
    if (shaderInfo.shadingSsboIndex()) {
        result = get_storage_buffer_access("fs", shaderInfo.shadingSsboIndex(), result.c_str());
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
    if (shaderInfo.shadingSsboIndex()) {
        result = get_storage_buffer_access("fs", shaderInfo.shadingSsboIndex(), result.c_str());
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
        list->push_back(ShaderInfo::kGradientBufferName);
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

// Emit a declaration for a helper function that represents the ShaderNode (named using the node's
// mangled name). The dynamic parameters are declared to match kDefaultArgs. The returned string
// can either be followed by a "{ body }" to fully define it or a ";" for a forward declaration.
std::string emit_helper_declaration(const ShaderNode* node) {
    const ShaderSnippet* entry = node->entry();
    std::string helperFnName = get_mangled_name(entry->fName, node->keyIndex());

    STArray<3, std::string> params;
    append_defaults(&params, node, /*args=*/nullptr); // null args emits declarations

    return SkSL::String::printf("half4 %s(%s)", helperFnName.c_str(), stitch_csv(params).c_str());
}

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
// ShaderSnippet

const ShaderSnippet::Args ShaderSnippet::kDefaultArgs = {"inColor", "destColor", "pos"};

//--------------------------------------------------------------------------------------------------
// ShaderNode

// If we have no children, we don't need to add anything into the preamble.
// If we have child entries, we create a function in the preamble with a signature of:
//     half4 SnippetName_N(/* required variable inputs (e.g. float2 pos) */) { ... }
// This function invokes each child in sequence, and then calls the built-in function, passing all
// uniforms and child outputs along:
//     half4 BuiltinFunctionName(/* required variable inputs (e.g. float2 pos) */,
//                               /* all uniforms as parameters */,
//                               /* all child output variable names as parameters */);
std::string ShaderNode::generateDefaultPreamble(const ShaderInfo& shaderInfo) const {
    if (this->numChildren() == 0) {
        // We don't need a helper function to wrap the snippet's static function
        return "";
    }

    std::string code = emit_helper_declaration(this) + " {";

    // Invoke each child with unmodified input values and collect in a list of local variables
    STArray<2, std::string> childOutputVarNames;
    for (const ShaderNode* child : this->children()) {
        // Emit glue code into our helper function body (i.e. lifting the child execution up front
        // so their outputs can be passed to the static module function for the node's snippet).
        childOutputVarNames.push_back(
                child->invokeAndAssign(shaderInfo, ShaderSnippet::kDefaultArgs, &code));
    }

    // Finally, invoke the snippet from the helper function, passing uniforms and child outputs.
    STArray<3, std::string> params;
    append_defaults(&params, this, &ShaderSnippet::kDefaultArgs);
    append_uniforms(&params, shaderInfo, this, childOutputVarNames);

    SkSL::String::appendf(&code,
                              "return %s(%s);"
                          "}",
                          this->entry()->fStaticFunctionName,
                          stitch_csv(params).c_str());
    return code;
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 outColor%d`, where the %d is
// filled in with 'node->keyIndex()'.
std::string ShaderNode::invokeAndAssign(const ShaderInfo& shaderInfo,
                                        const ShaderSnippet::Args& args,
                                        std::string* funcBody) const {
    std::string expr = invoke_node(shaderInfo, this, args);
    std::string outputVar = get_mangled_name("outColor", this->keyIndex());
#if defined(SK_DEBUG)
    SkSL::String::appendf(funcBody,
                          "// [%d] %s\n"
                          "half4 %s = %s;",
                          this->keyIndex(),
                          this->entry()->fName,
                          outputVar.c_str(),
                          expr.c_str());
#else
    SkSL::String::appendf(funcBody,
                          "half4 %s = %s;",
                          outputVar.c_str(),
                          expr.c_str());
#endif
    return outputVar;
}

// Return a name that should be used as a varying passing the result of an expression emitted by
// this node, if the expression is being lifted from the fragment shader to the vertex shader. The
// choice of name is arbitrary, but it must be used consistently.
std::string ShaderNode::getExpressionVaryingName() const {
    return get_mangled_name(this->entry()->fName, this->keyIndex()) + "_Var";
}

//--------------------------------------------------------------------------------------------------
// ShaderCodeDictionary

UniquePaintParamsID ShaderCodeDictionary::findOrCreate(PaintParamsKeyBuilder* builder) {
    AutoLockBuilderAsKey keyView{builder};

    return this->findOrCreate(*keyView);
}

UniquePaintParamsID ShaderCodeDictionary::findOrCreate(const PaintParamsKey& ppk) {
    if (!ppk.isValid()) {
        return UniquePaintParamsID::Invalid();
    }

    SkAutoSpinlock lock{fSpinLock};

    UniquePaintParamsID* existingEntry = fPaintKeyToID.find(ppk);
    if (existingEntry) {
        SkASSERT(fIDToPaintKey[(*existingEntry).asUInt()] == ppk);
        return *existingEntry;
    }

    // Detach from the builder and copy into the arena
    PaintParamsKey key = ppk.clone(&fArena);
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

    if (IsSkiaKnownRuntimeEffect(codeSnippetID)) {
        int knownRTECodeSnippetID = codeSnippetID - kSkiaKnownRuntimeEffectsStart;

        // TODO(b/238759147): if the snippet hasn't been initialized, get the SkRuntimeEffect and
        // initialize it here
        SkASSERT(fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID].fPreambleGenerator);
        return &fKnownRuntimeEffectCodeSnippets[knownRTECodeSnippetID];
    }

    if (IsViableUserDefinedKnownRuntimeEffect(codeSnippetID)) {
        int index = codeSnippetID - kUserDefinedKnownRuntimeEffectsStart;
        if (index >= fUserDefinedKnownCodeSnippets.size()) {
            return nullptr;
        }

        SkASSERT(fUserDefinedKnownCodeSnippets[index].fPreambleGenerator);
        return &fUserDefinedKnownCodeSnippets[index];
    }

    if (IsUserDefinedRuntimeEffect(codeSnippetID)) {
        int userDefinedCodeSnippetID = codeSnippetID - kUnknownRuntimeEffectIDStart;
        if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
            return &fUserDefinedCodeSnippets[userDefinedCodeSnippetID];
        }
    }

    return nullptr;
}

const SkRuntimeEffect* ShaderCodeDictionary::getUserDefinedKnownRuntimeEffect(
        int codeSnippetID) const {
    if (codeSnippetID < 0) {
        return nullptr;
    }

    if (IsViableUserDefinedKnownRuntimeEffect(codeSnippetID)) {
        int index = codeSnippetID - kUserDefinedKnownRuntimeEffectsStart;
        if (index >= fUserDefinedKnownRuntimeEffects.size()) {
            return nullptr;
        }

        SkASSERT(fUserDefinedKnownRuntimeEffects[index]);
        return fUserDefinedKnownRuntimeEffects[index].get();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
namespace {

std::string GenerateSolidColorExpression(const ShaderInfo& shaderInfo,
                                         const ShaderNode* node,
                                         const ShaderSnippet::Args& args) {
    std::string uniform =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
    return SkSL::String::printf("half4(%s)", uniform.c_str());
}

std::string GenerateSolidColorPreamble(const ShaderInfo& shaderInfo,
                                       const ShaderNode* node) {
    std::string code = emit_helper_declaration(node) + " {return ";

    if (node->requiredFlags() & SnippetRequirementFlags::kLiftExpression) {
        code += node->getExpressionVaryingName();
    } else if (node->requiredFlags() & SnippetRequirementFlags::kOmitExpression) {
        code += "half4(0)";
    } else {
        code += GenerateSolidColorExpression(shaderInfo, node, ShaderSnippet::kDefaultArgs);
    }

    return code + ";}";
}

//--------------------------------------------------------------------------------------------------

// Generate the expression that applies a non-perspective local matrix to coordinates.
std::string GenerateLocalMatrixExpression(const ShaderInfo& shaderInfo,
                                          const ShaderNode* node,
                                          const ShaderSnippet::Args& args) {
    // NOTE: upper2x2 is a float2x2 packed in column major order into a float4
    std::string upper2x2 =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
    std::string translation =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[1], node->keyIndex());
    return SkSL::String::printf("float2x2(%s.xy, %s.zw)*%s + %s",
                                upper2x2.c_str(),
                                upper2x2.c_str(),
                                args.fFragCoord.c_str(),
                                translation.c_str());
}

static constexpr int kNumCoordinateManipulateChildren = 1;

std::string GenerateCoordNormalizeExpression(const ShaderInfo& shaderInfo,
                                             const ShaderNode* node,
                                             const ShaderSnippet::Args& args) {
    std::string uniform =
            get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());
    return SkSL::String::printf("(%s * %s)",
                                uniform.c_str(),
                                args.fFragCoord.c_str());
}

// Create a helper function that manipulates the coordinates passed into a child. The specific
// manipulation is pre-determined by the code id (local matrix or clamp).
// TODO: This is effectively GenerateComposePreamble except that 'node' is counting as the inner.
std::string GenerateCoordManipulationPreamble(const ShaderInfo& shaderInfo,
                                              const ShaderNode* node) {
    SkASSERT(node->numChildren() == kNumCoordinateManipulateChildren);

    std::string perspectiveStatement;

    const ShaderSnippet::Args& defaultArgs = ShaderSnippet::kDefaultArgs;
    ShaderSnippet::Args localArgs = ShaderSnippet::kDefaultArgs;
    if (node->child(0)->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        std::string controlUni =
                get_mangled_uniform_name(shaderInfo, node->entry()->fUniforms[0], node->keyIndex());

        if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShader) {
            if (node->requiredFlags() & SnippetRequirementFlags::kLiftExpression) {
                localArgs.fFragCoord = node->getExpressionVaryingName();
            } else if (!(node->requiredFlags() & SnippetRequirementFlags::kOmitExpression)) {
                localArgs.fFragCoord = GenerateLocalMatrixExpression(shaderInfo, node, defaultArgs);
            }
        } else if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kLocalMatrixShaderPersp) {
            perspectiveStatement = SkSL::String::printf("float3 perspCoord = %s * %s.xy1;",
                                                        controlUni.c_str(),
                                                        defaultArgs.fFragCoord.c_str());
            localArgs.fFragCoord = "perspCoord.xy / perspCoord.z";
        } else if (node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordNormalizeShader) {
            if (node->requiredFlags() & SnippetRequirementFlags::kLiftExpression) {
                localArgs.fFragCoord = node->getExpressionVaryingName();
            } else if (!(node->requiredFlags() & SnippetRequirementFlags::kOmitExpression)) {
                localArgs.fFragCoord =
                        GenerateCoordNormalizeExpression(shaderInfo, node, defaultArgs);
            }
        } else {
            SkASSERT(node->codeSnippetId() == (int) BuiltInCodeSnippetID::kCoordClampShader);
            localArgs.fFragCoord = SkSL::String::printf("clamp(%s, %s.LT, %s.RB)",
                                                        defaultArgs.fFragCoord.c_str(),
                                                        controlUni.c_str(), controlUni.c_str());
        }
    } // else this is a no-op

    std::string decl = emit_helper_declaration(node);
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

    const ShaderSnippet::Args& defaultArgs = ShaderSnippet::kDefaultArgs;
    ShaderSnippet::Args outerArgs = ShaderSnippet::kDefaultArgs;
    int child = 0;
    if (outer->requiredFlags() & SnippetRequirementFlags::kLocalCoords) {
        outerArgs.fFragCoord = invoke_node(shaderInfo, node->child(child++), defaultArgs);
    }
    if (outer->requiredFlags() & SnippetRequirementFlags::kPriorStageOutput) {
        outerArgs.fPriorStageOutput = invoke_node(shaderInfo, node->child(child++), defaultArgs);
    }
    if (outer->requiredFlags() & SnippetRequirementFlags::kBlenderDstColor) {
        outerArgs.fBlenderDstColor = invoke_node(shaderInfo, node->child(child++), defaultArgs);
    }

    std::string decl = emit_helper_declaration(node);
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
        if (fShaderInfo.shadingSsboIndex()) {
            result =
                    get_storage_buffer_access("fs", fShaderInfo.shadingSsboIndex(), result.c_str());
        }
        return result;
    }

    void defineFunction(const char* decl, const char* body, bool isMain) override {
        if (isMain) {
            SkSL::String::appendf(
                    fPreamble,
                    "%s { %s }",
                    emit_helper_declaration(fNode).c_str(),
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
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
        args.fFragCoord = coords;
        return invoke_node(fShaderInfo, fNode->child(index), args);
    }

    std::string sampleColorFilter(int index, std::string color) override {
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
        args.fPriorStageOutput = color;
        return invoke_node(fShaderInfo, fNode->child(index), args);
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
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
                         (int)BuiltInCodeSnippetID::kColorSpaceXformColorFilter ||
                 toLinearSrgbNode->codeSnippetId() ==
                         (int)BuiltInCodeSnippetID::kColorSpaceXformPremul ||
                 toLinearSrgbNode->codeSnippetId() ==
                         (int)BuiltInCodeSnippetID::kColorSpaceXformSRGB);

        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
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
                         (int)BuiltInCodeSnippetID::kColorSpaceXformColorFilter ||
                 fromLinearSrgbNode->codeSnippetId() ==
                         (int)BuiltInCodeSnippetID::kColorSpaceXformPremul ||
                 fromLinearSrgbNode->codeSnippetId() ==
                         (int)BuiltInCodeSnippetID::kColorSpaceXformSRGB);

        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
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
    // Find this runtime effect in the shader-code or runtime-effect dictionary.
    SkASSERT(node->codeSnippetId() >= kBuiltInCodeSnippetIDCount);
    const SkRuntimeEffect* effect;

    if (IsSkiaKnownRuntimeEffect(node->codeSnippetId())) {
        effect = GetKnownRuntimeEffect(static_cast<StableKey>(node->codeSnippetId()));
    } else if (SkKnownRuntimeEffects::IsViableUserDefinedKnownRuntimeEffect(
                                                              node->codeSnippetId())) {
        effect = shaderInfo.shaderCodeDictionary()->getUserDefinedKnownRuntimeEffect(
                node->codeSnippetId());
    } else {
        SkASSERT(IsUserDefinedRuntimeEffect(node->codeSnippetId()));
        effect = shaderInfo.runtimeEffectDictionary()->find(node->codeSnippetId());
    }
    // This should always be true given the circumstances in which we call convertRuntimeEffect
    SkASSERT(effect);

    const SkSL::Program& program = SkRuntimeEffectPriv::Program(*effect);
    const ShaderSnippet::Args& args = ShaderSnippet::kDefaultArgs;
    std::string preamble;
    GraphitePipelineCallbacks callbacks{shaderInfo, node, &preamble, effect};
    SkSL::PipelineStage::ConvertProgram(program,
                                        args.fFragCoord.c_str(),
                                        args.fPriorStageOutput.c_str(),
                                        args.fBlenderDstColor.c_str(),
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
    if (IsSkiaKnownRuntimeEffect(snippetID)) {
        return true;
    }

    if (this->isUserDefinedKnownRuntimeEffect(snippetID)) {
        return true;
    }

    SkAutoSpinlock lock{fSpinLock};

    if (IsUserDefinedRuntimeEffect(snippetID)) {
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

static bool all_sample_usages_are_passthrough(const SkRuntimeEffect* effect) {
    for (size_t i = 0; i < effect->children().size(); ++i) {
        if (!SkRuntimeEffectPriv::ChildSampleUsage(effect, i).isPassThrough()) {
            return false;
        }
    }
    return true;
}

ShaderSnippet ShaderCodeDictionary::convertRuntimeEffect(const SkRuntimeEffect* effect,
                                                         const char* name) {
    SkEnumBitMask<SnippetRequirementFlags> snippetFlags = SnippetRequirementFlags::kNone;
    if (effect->allowShader()) {
        // TODO(b/412621191) Ideally we would have a way to tell exactly which children of a runtime
        // shader are sampled with modified coords, or whether coordinates are required at all. For
        // now we assume all runtime shaders need coordinates, and if any children are sampled with
        // modified coords, we assume they all are.
        snippetFlags |= SnippetRequirementFlags::kLocalCoords;
        if (all_sample_usages_are_passthrough(effect)) {
            snippetFlags |= SnippetRequirementFlags::kPassthroughLocalCoords;
        }
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
                         /*texturesAndSamplers=*/{},
                         GenerateRuntimeShaderPreamble,
                         numChildrenIncColorTransforms);
}

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
     SkAutoSpinlock lock{fSpinLock};

    if (int stableKey = SkRuntimeEffectPriv::StableKey(*effect)) {
        if (IsSkiaKnownRuntimeEffect(stableKey)) {
            int index = stableKey - kSkiaKnownRuntimeEffectsStart;

            if (!fKnownRuntimeEffectCodeSnippets[index].fPreambleGenerator) {
                const char* name = get_known_rte_name(static_cast<StableKey>(stableKey));
                fKnownRuntimeEffectCodeSnippets[index] = this->convertRuntimeEffect(effect, name);
            }

            return stableKey;
        } else if (IsViableUserDefinedKnownRuntimeEffect(stableKey)) {
            int index = stableKey - kUserDefinedKnownRuntimeEffectsStart;
            if (index >= fUserDefinedKnownCodeSnippets.size()) {
                return -1;
            }

            return stableKey;
        }

        return -1;
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
    static const char* kDefaultName = "RuntimeEffect";
    fUserDefinedCodeSnippets.push_back(this->convertRuntimeEffect(
                effect,
                SkRuntimeEffectPriv::HasName(*effect) ? SkRuntimeEffectPriv::GetName(*effect)
                                                      : kDefaultName));
    int newCodeSnippetID = kUnknownRuntimeEffectIDStart + fUserDefinedCodeSnippets.size() - 1;

    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
}

void ShaderCodeDictionary::registerUserDefinedKnownRuntimeEffects(
        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects) {
    // This is a formality to guard 'fRuntimeEffectMap'. This method should only be called by
    // the constructor.
    SkAutoSpinlock lock{fSpinLock};

    for (const sk_sp<SkRuntimeEffect>& u : userDefinedKnownRuntimeEffects) {
        if (!u) {
            continue;
        }

        if (fUserDefinedKnownCodeSnippets.size() >= kUserDefinedKnownRuntimeEffectsReservedCnt) {
            SKGPU_LOG_W("Too many user-defined known runtime effects. Only %d out of %zu "
                        "will be known.\n",
                        kUserDefinedKnownRuntimeEffectsReservedCnt,
                        userDefinedKnownRuntimeEffects.size());
            // too many user-defined known runtime effects
            return;
        }

        RuntimeEffectKey key;
        key.fHash = SkRuntimeEffectPriv::Hash(*u);
        key.fUniformSize = u->uniformSize();

        int32_t* existingCodeSnippetID = fRuntimeEffectMap.find(key);
        if (existingCodeSnippetID) {
            continue;           // This is a duplicate
        }

        static const char* kDefaultName = "UserDefinedKnownRuntimeEffect";
        fUserDefinedKnownCodeSnippets.push_back(this->convertRuntimeEffect(
                    u.get(),
                    SkRuntimeEffectPriv::HasName(*u) ? SkRuntimeEffectPriv::GetName(*u)
                                                     : kDefaultName));
        int stableID = kUserDefinedKnownRuntimeEffectsStart +
                       fUserDefinedKnownCodeSnippets.size() - 1;

        SkRuntimeEffectPriv::SetStableKey(u.get(), stableID);

        fUserDefinedKnownRuntimeEffects.push_back(u);

        // We register the key with the runtime effect map so that, if the user uses the same code
        // in a separate runtime effect (which they should *not* do), it will be discovered during
        // the unknown-runtime-effect processing and mapped back to the registered user-defined
        // known runtime effect.
        fRuntimeEffectMap.set(key, stableID);
    }

    SkASSERT(fUserDefinedKnownCodeSnippets.size() == fUserDefinedKnownRuntimeEffects.size());
}

bool ShaderCodeDictionary::isUserDefinedKnownRuntimeEffect(int candidate) const {
    if (!SkKnownRuntimeEffects::IsViableUserDefinedKnownRuntimeEffect(candidate)) {
        return false;
    }

    int index = candidate - kUserDefinedKnownRuntimeEffectsStart;
    if (index >= fUserDefinedKnownCodeSnippets.size()) {
        return false;
    }

    return true;
}

#if defined(GPU_TEST_UTILS)
int ShaderCodeDictionary::numUserDefinedRuntimeEffects() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUserDefinedCodeSnippets.size();
}

int ShaderCodeDictionary::numUserDefinedKnownRuntimeEffects() const {
    return fUserDefinedKnownCodeSnippets.size();
}
#endif

ShaderCodeDictionary::ShaderCodeDictionary(
                Layout layout,
                SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
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
            /*name=*/"Passthrough",
            /*staticFn=*/"sk_passthrough",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSolidColorShader] = {
            /*name=*/"SolidColor",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "color", SkSLType::kFloat4 } },
            /*texturesAndSamplers=*/{},
            GenerateSolidColorPreamble,
            /*numChildren=*/0,
            /*liftableExpression=*/GenerateSolidColorExpression,
            /*liftableExpressionType=*/ShaderSnippet::LiftableExpressionType::kPriorStageOutput,
            /*liftableExpressionInterpolation=*/Interpolation::kLinear
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
            /*texturesAndSamplers=*/{"colorAndOffsetSampler"}
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
            /*texturesAndSamplers=*/{"colorAndOffsetSampler"}
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
            /*texturesAndSamplers=*/{"colorAndOffsetSampler"}
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
            /*texturesAndSamplers=*/{"colorAndOffsetSampler"}
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
            /*name=*/"LocalMatrix",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "upper2x2",    SkSLType::kFloat4 },
                           { "translation", SkSLType::kFloat2 } },
            /*texturesAndSamplers=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren,
            /*liftableExpression=*/GenerateLocalMatrixExpression,
            /*liftableExpressionType=*/ShaderSnippet::LiftableExpressionType::kLocalCoords
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShaderPersp] = {
            /*name=*/"LocalMatrixShaderPersp",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "localMatrix", SkSLType::kFloat3x3 } },
            /*texturesAndSamplers=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            /*name=*/"Image",
            /*staticFn=*/"sk_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresSamplerDescData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "filterMode",            SkSLType::kInt } },
            /*texturesAndSamplers=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicImageShader] = {
            /*name=*/"CubicImage",
            /*staticFn=*/"sk_cubic_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresSamplerDescData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subset",                SkSLType::kFloat4 },
                           { "tilemodeX",             SkSLType::kInt },
                           { "tilemodeY",             SkSLType::kInt },
                           { "cubicCoeffs",           SkSLType::kHalf4x4 } },
            /*texturesAndSamplers=*/{"image"}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWImageShader] = {
            /*name=*/"HardwareImage",
            /*staticFn=*/"sk_hw_image_shader",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresSamplerDescData,
            /*uniforms=*/{},
            /*texturesAndSamplers=*/{"image"}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShaderClamp] = {
            /*name=*/"ImageShaderClamp",
            /*staticFn=*/"sk_image_shader_clamp",
            SnippetRequirementFlags::kLocalCoords | SnippetRequirementFlags::kStoresSamplerDescData,
            /*uniforms=*/{ { "invImgSize",            SkSLType::kFloat2 },
                           { "subsetInsetClamp",      SkSLType::kFloat4 } },
            /*texturesAndSamplers=*/{"image"}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kYUVImageShader] = {
            /*name=*/"YUVImage",
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
            /*texturesAndSamplers=*/ {{ "samplerY" },
                                      { "samplerU" },
                                      { "samplerV" },
                                      { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCubicYUVImageShader] = {
            /*name=*/"CubicYUVImage",
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
            /*texturesAndSamplers=*/ {{ "samplerY" },
                                      { "samplerU" },
                                      { "samplerV" },
                                      { "samplerA" }}
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWYUVImageShader] = {
            /*name=*/"HWYUVImage",
            /*staticFn=*/"sk_hw_yuv_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",           SkSLType::kFloat2 },
                           { "invImgSizeUV",          SkSLType::kFloat2 }, // Relative to Y's texels
                           { "subset",                SkSLType::kFloat4 },
                           { "linearFilterUVInset",   SkSLType::kFloat2 },
                           { "channelSelectY",        SkSLType::kHalf4 },
                           { "channelSelectU",        SkSLType::kHalf4 },
                           { "channelSelectV",        SkSLType::kHalf4 },
                           { "channelSelectA",        SkSLType::kHalf4 },
                           { "yuvToRGBMatrix",        SkSLType::kHalf3x3 },
                           { "yuvToRGBTranslate",     SkSLType::kHalf3 } },
            /*texturesAndSamplers=*/ {{ "samplerY" },
                                      { "samplerU" },
                                      { "samplerV" },
                                      { "samplerA" }}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHWYUVNoSwizzleImageShader] = {
            /*name=*/"HWYUVImageNoSwizzle",
            /*staticFn=*/"sk_hw_yuv_no_swizzle_image_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "invImgSizeY",              SkSLType::kFloat2 },
                           { "invImgSizeUV",             SkSLType::kFloat2 }, // Relative to Y space
                           { "subset",                   SkSLType::kFloat4 },
                           { "linearFilterUVInset",      SkSLType::kFloat2 },
                           { "yuvToRGBMatrix",           SkSLType::kHalf3x3 },
                           { "yuvToRGBXlateAlphaParams", SkSLType::kHalf4 } },
            /*texturesAndSamplers=*/ {{ "samplerY" },
                                      { "samplerU" },
                                      { "samplerV" },
                                      { "samplerA" }}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoordNormalizeShader] = {
            /*name=*/"CoordNormalize",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "invDimensions", SkSLType::kFloat2 } },
            /*texturesAndSamplers=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren,
            /*liftableExpression=*/GenerateCoordNormalizeExpression,
            /*liftableExpressionType=*/ShaderSnippet::LiftableExpressionType::kLocalCoords
    };

    // Like the local matrix shader, this is a no-op if the child doesn't need coords
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCoordClampShader] = {
            /*name=*/"CoordClamp",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kNone,
            /*uniforms=*/{ { "subset", SkSLType::kFloat4 } },
            /*texturesAndSamplers=*/{},
            GenerateCoordManipulationPreamble,
            /*numChildren=*/kNumCoordinateManipulateChildren
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kDitherShader] = {
            /*name=*/"Dither",
            /*staticFn=*/"sk_dither",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "range", SkSLType::kHalf } },
            /*texturesAndSamplers=*/{ { "ditherLUT" } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPerlinNoiseShader] = {
            /*name=*/"PerlinNoise",
            /*staticFn=*/"sk_perlin_noise_shader",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "baseFrequency", SkSLType::kFloat2 },
                           { "stitchData",    SkSLType::kFloat2 },
                           { "noiseType",     SkSLType::kInt },
                           { "numOctaves",    SkSLType::kInt },
                           { "stitching",     SkSLType::kInt } },
            /*texturesAndSamplers=*/{ { "permutationsSampler" },
                                      { "noiseSampler" } }
    };

    // SkColorFilter snippets
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kMatrixColorFilter] = {
            /*name=*/"MatrixColorFilter",
            /*staticFn=*/"sk_matrix_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{
                           { "colorMatrix",    SkSLType::kHalf4x4 },
                           { "colorTranslate", SkSLType::kHalf4 },
                           { "minMaxRGB",      SkSLType::kHalf2 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHSLMatrixColorFilter] = {
            /*name=*/"HSLMatrixColorFilter",
            /*staticFn=*/"sk_hsl_matrix_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "colorMatrix",    SkSLType::kHalf4x4 },
                           { "colorTranslate", SkSLType::kHalf4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kTableColorFilter] = {
            /*name=*/"TableColorFilter",
            /*staticFn=*/"sk_table_colorfilter",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{},
            /*texturesAndSamplers=*/{ {"table"} }};
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
            /*uniforms=*/{ { "gamut",        SkSLType::kHalf3x3 },
                           { "srcGABC",      SkSLType::kFloat4 },
                           { "srcDEF_args",  SkSLType::kFloat4 },
                           { "dstGABC",      SkSLType::kFloat4 },
                           { "dstDEF_args",  SkSLType::kFloat4 },
                           { "srcOOTF_args", SkSLType::kFloat4 },
                           { "dstOOTF_args", SkSLType::kFloat4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformPremul] = {
            /*name=*/"ColorSpaceTransformPremul",
            /*staticFn=*/"sk_color_space_transform_premul",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "args", SkSLType::kHalf2 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformSRGB] = {
            /*name=*/"ColorSpaceTransformSRGB",
            /*staticFn=*/"sk_color_space_transform_srgb",
            SnippetRequirementFlags::kPriorStageOutput,
            /*uniforms=*/{ { "gamut",       SkSLType::kHalf3x3 },
                           { "srcGABC",     SkSLType::kFloat4 },
                           { "srcDEF_args", SkSLType::kFloat4 },
                           { "dstGABC",     SkSLType::kFloat4 },
                           { "dstDEF_args", SkSLType::kFloat4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPrimitiveColor] = {
            /*name=*/"PrimitiveColor",
            /*staticFn=*/"sk_passthrough",
            SnippetRequirementFlags::kPrimitiveColor,
            /*uniforms=*/{}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAnalyticClip] = {
            /*name=*/"AnalyticClip",
            /*staticFn=*/"sk_analytic_clip",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "rect",           SkSLType::kFloat4 },
                           { "radiusPlusHalf", SkSLType::kFloat2 },
                           { "edgeSelect",     SkSLType::kHalf4 } }
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAnalyticAndAtlasClip] = {
            /*name=*/"AnalyticAndAtlasClip",
            /*staticFn=*/"sk_analytic_and_atlas_clip",
            SnippetRequirementFlags::kLocalCoords,
            /*uniforms=*/{ { "rect",           SkSLType::kFloat4 },
                           { "radiusPlusHalf", SkSLType::kFloat2 },
                           { "edgeSelect",     SkSLType::kHalf4 },
                           { "texCoordOffset", SkSLType::kFloat2 },
                           { "maskBounds",     SkSLType::kFloat4 },
                           { "invAtlasSize",   SkSLType::kFloat2 } },
            /*texturesAndSamplers=*/{"atlasSampler"}
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kCompose] = {
            /*name=*/"Compose",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kPassthroughLocalCoords,
            /*uniforms=*/{},
            /*texturesAndSamplers=*/{},
            GenerateComposePreamble,
            /*numChildren=*/2
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendCompose] = {
            /*name=*/"BlendCompose",
            /*staticFn=*/nullptr,
            SnippetRequirementFlags::kPassthroughLocalCoords,
            /*uniforms=*/{},
            /*texturesAndSamplers=*/{},
            GenerateComposePreamble,
            /*numChildren=*/3
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPorterDuffBlender] = {
            /*name=*/"PorterDuffBlender",
            /*staticFn=*/"sk_porter_duff_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "coeffs", SkSLType::kHalf4 } }
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kHSLCBlender] = {
            /*name=*/"HSLCBlender",
            /*staticFn=*/"sk_hslc_blend",
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kBlenderDstColor,
            /*uniforms=*/{ { "flipSat", SkSLType::kHalf2 } }
    };

    // Fixed-function blend mode snippets are all the same, their functionality is entirely defined
    // by their unique code snippet IDs.
    for (int i = 0; i <= (int) SkBlendMode::kLastMode; ++i) {
        int ffBlendModeID = kFixedBlendIDOffset + i;
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

    // Check for duplicate snippet names.
    SkDEBUGCODE(
        THashSet<std::string> snippetNames;
        for (const ShaderSnippet& snippet : fBuiltInCodeSnippets) {
            std::string name = snippet.fName;
            SkASSERT(!snippetNames.contains(name));
            snippetNames.add(name);
        }
    )

    this->registerUserDefinedKnownRuntimeEffects(userDefinedKnownRuntimeEffects);
}

// clang-format off

// Verify that the built-in code IDs for fixed function blending are consistent with SkBlendMode.
static_assert((int)SkBlendMode::kClear      == (int)BuiltInCodeSnippetID::kFixedBlend_Clear      - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSrc        == (int)BuiltInCodeSnippetID::kFixedBlend_Src        - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDst        == (int)BuiltInCodeSnippetID::kFixedBlend_Dst        - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSrcOver    == (int)BuiltInCodeSnippetID::kFixedBlend_SrcOver    - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDstOver    == (int)BuiltInCodeSnippetID::kFixedBlend_DstOver    - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSrcIn      == (int)BuiltInCodeSnippetID::kFixedBlend_SrcIn      - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDstIn      == (int)BuiltInCodeSnippetID::kFixedBlend_DstIn      - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSrcOut     == (int)BuiltInCodeSnippetID::kFixedBlend_SrcOut     - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDstOut     == (int)BuiltInCodeSnippetID::kFixedBlend_DstOut     - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSrcATop    == (int)BuiltInCodeSnippetID::kFixedBlend_SrcATop    - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDstATop    == (int)BuiltInCodeSnippetID::kFixedBlend_DstATop    - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kXor        == (int)BuiltInCodeSnippetID::kFixedBlend_Xor        - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kPlus       == (int)BuiltInCodeSnippetID::kFixedBlend_Plus       - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kModulate   == (int)BuiltInCodeSnippetID::kFixedBlend_Modulate   - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kScreen     == (int)BuiltInCodeSnippetID::kFixedBlend_Screen     - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kOverlay    == (int)BuiltInCodeSnippetID::kFixedBlend_Overlay    - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDarken     == (int)BuiltInCodeSnippetID::kFixedBlend_Darken     - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kColorDodge == (int)BuiltInCodeSnippetID::kFixedBlend_ColorDodge - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kColorBurn  == (int)BuiltInCodeSnippetID::kFixedBlend_ColorBurn  - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kHardLight  == (int)BuiltInCodeSnippetID::kFixedBlend_HardLight  - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSoftLight  == (int)BuiltInCodeSnippetID::kFixedBlend_SoftLight  - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kDifference == (int)BuiltInCodeSnippetID::kFixedBlend_Difference - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kExclusion  == (int)BuiltInCodeSnippetID::kFixedBlend_Exclusion  - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kMultiply   == (int)BuiltInCodeSnippetID::kFixedBlend_Multiply   - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kHue        == (int)BuiltInCodeSnippetID::kFixedBlend_Hue        - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kSaturation == (int)BuiltInCodeSnippetID::kFixedBlend_Saturation - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kColor      == (int)BuiltInCodeSnippetID::kFixedBlend_Color      - kFixedBlendIDOffset);
static_assert((int)SkBlendMode::kLuminosity == (int)BuiltInCodeSnippetID::kFixedBlend_Luminosity - kFixedBlendIDOffset);

static_assert(0 == static_cast<int>(SkTileMode::kClamp),  "ImageShader code depends on SkTileMode");
static_assert(1 == static_cast<int>(SkTileMode::kRepeat), "ImageShader code depends on SkTileMode");
static_assert(2 == static_cast<int>(SkTileMode::kMirror), "ImageShader code depends on SkTileMode");
static_assert(3 == static_cast<int>(SkTileMode::kDecal),  "ImageShader code depends on SkTileMode");

static_assert(0 == static_cast<int>(SkFilterMode::kNearest), "ImageShader code depends on SkFilterMode");
static_assert(1 == static_cast<int>(SkFilterMode::kLinear),  "ImageShader code depends on SkFilterMode");

// clang-format on

} // namespace skgpu::graphite
