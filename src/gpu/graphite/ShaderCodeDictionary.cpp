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
#include "include/private/SkOpts_spi.h"
#include "include/private/SkSLString.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/ReadWriteSwizzle.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <new>

namespace {

std::string get_mangled_name(const std::string& baseName, int manglingSuffix) {
    return baseName + "_" + std::to_string(manglingSuffix);
}

} // anonymous namespace

namespace skgpu::graphite {

using DataPayloadField = PaintParamsKey::DataPayloadField;
using DataPayloadType = PaintParamsKey::DataPayloadType;

std::string ShaderSnippet::getMangledUniformName(const ShaderInfo& shaderInfo,
                                                 int uniformIdx,
                                                 int mangleId) const {
    std::string result;
    result = fUniforms[uniformIdx].name() + std::string("_") + std::to_string(mangleId);
    if (shaderInfo.ssboIndex()) {
        result = EmitStorageBufferAccess("fs", shaderInfo.ssboIndex(), result.c_str());
    }
    return result;
}

std::string ShaderSnippet::getMangledSamplerName(int samplerIdx, int mangleId) const {
    std::string result;
    result = fTexturesAndSamplers[samplerIdx].name() + std::string("_") + std::to_string(mangleId);
    return result;
}

// Returns an expression to invoke this entry.
static std::string emit_expression_for_entry(const ShaderInfo& shaderInfo,
                                             int entryIndex,
                                             ShaderSnippet::Args args) {
    const PaintParamsKey::BlockReader& reader = shaderInfo.blockReader(entryIndex);
    const ShaderSnippet* entry = reader.entry();

    return entry->fExpressionGenerator(shaderInfo, entryIndex, reader, args);
}

// Emit the glue code needed to invoke a single static helper isolated within its own scope.
// Glue code will assign the resulting color into a variable `half4 outColor%d`, where the %d is
// filled in with 'entryIndex'.
static std::string emit_glue_code_for_entry(const ShaderInfo& shaderInfo,
                                            int entryIndex,
                                            const ShaderSnippet::Args& args,
                                            std::string* funcBody) {
    const ShaderSnippet* entry = shaderInfo.blockReader(entryIndex).entry();

    std::string expr = emit_expression_for_entry(shaderInfo, entryIndex, args);
    std::string outputVar = get_mangled_name("outColor", entryIndex);
    SkSL::String::appendf(funcBody,
                          "// %s\n"
                          "half4 %s = %s;",
                          entry->fName,
                          outputVar.c_str(),
                          expr.c_str());
    return outputVar;
}

static void emit_preamble_for_entry(const ShaderInfo& shaderInfo,
                                    int* entryIndex,
                                    std::string* preamble) {
    const PaintParamsKey::BlockReader& reader = shaderInfo.blockReader(*entryIndex);

    [[maybe_unused]] int startingEntryIndex = *entryIndex;
    reader.entry()->fPreambleGenerator(shaderInfo, entryIndex, reader, preamble);

    // Preamble generators are responsible for increasing the entry index as entries are consumed.
    SkASSERT(*entryIndex > startingEntryIndex);
}

// The current, incomplete, model for shader construction is:
//   - Static code snippets (which can have an arbitrary signature) live in the Graphite
//     pre-compiled module, which is located at `src/sksl/sksl_graphite_frag.sksl`.
//   - Glue code is generated in a `main` method which calls these static code snippets.
//     The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   - The result of the final code snippet is then copied into "sk_FragColor".
//   Note: each entry's 'fStaticFunctionName' field is expected to match the name of a function
//   in the Graphite pre-compiled module.
std::string ShaderInfo::toSkSL(const ResourceBindingRequirements& bindingReqs,
                               const RenderStep* step,
                               const bool useStorageBuffers,
                               const bool defineLocalCoordsVarying,
                               int* numTexturesAndSamplersUsed) const {
    std::string preamble = EmitVaryings(step,
                                        /*direction=*/"in",
                                        /*emitShadingSsboIndexVarying=*/useStorageBuffers,
                                        defineLocalCoordsVarying);

    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    // TODO: replace hard-coded bufferIDs with the backend's step and paint uniform-buffer indices.
    // TODO: The use of these indices is Metal-specific. We should replace these functions with
    // API-independent ones.
    if (step->numUniforms() > 0) {
        preamble += EmitRenderStepUniforms(
                /*bufferID=*/1, "Step", bindingReqs.fUniformBufferLayout, step->uniforms());
    }
    if (this->ssboIndex()) {
        preamble += EmitPaintParamsStorageBuffer(/*bufferID=*/2, "FS", "fs", fBlockReaders);
    } else {
        preamble += EmitPaintParamsUniforms(
                /*bufferID=*/2,
                "FS",
                useStorageBuffers ? bindingReqs.fStorageBufferLayout
                                  : bindingReqs.fUniformBufferLayout,
                fBlockReaders);
    }

    {
        int binding = 0;
        preamble += EmitTexturesAndSamplers(bindingReqs, fBlockReaders, &binding);
        if (step->hasTextures()) {
            preamble += step->texturesAndSamplersSkSL(bindingReqs, &binding);
        }

        // Report back to the caller how many textures and samplers are used.
        if (numTexturesAndSamplersUsed) {
            *numTexturesAndSamplersUsed = binding;
        }
    }

    std::string mainBody = "void main() {";
    // Set initial color. This will typically be optimized out by SkSL in favor of the paint
    // specifying a color with a solid color shader.
    std::string lastOutputVar = "initialColor";
    mainBody += "half4 initialColor = half4(0);";

    if (step->emitsPrimitiveColor()) {
        mainBody += "half4 primitiveColor;";
        mainBody += step->fragmentColorSkSL();
    }

    for (int entryIndex = 0; entryIndex < (int)fBlockReaders.size();) {
        // Emit shader main body code. This never alters the preamble or increases the entry index.
        static constexpr char kUnusedDestColor[] = "half4(1)";
        static constexpr char kUnusedLocalCoordinates[] = "float2(0)";
        const std::string localCoordinates = this->needsLocalCoords() ? "localCoordsVar"
                                                                      : kUnusedLocalCoordinates;
        lastOutputVar = emit_glue_code_for_entry(*this, entryIndex, {lastOutputVar,
                                                 kUnusedDestColor, localCoordinates},
                                                 &mainBody);

        // Emit preamble code. This iterates over all the children as well, and increases the entry
        // index as we go.
        emit_preamble_for_entry(*this, &entryIndex, &preamble);
    }

    if (step->emitsCoverage()) {
        mainBody += "half4 outputCoverage;";
        mainBody += step->fragmentCoverageSkSL();
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s * outputCoverage;",
                              lastOutputVar.c_str());
    } else {
        SkSL::String::appendf(&mainBody, "sk_FragColor = %s;", lastOutputVar.c_str());
    }
    mainBody += "}\n";

    return preamble + "\n" + mainBody;
}

ShaderCodeDictionary::Entry* ShaderCodeDictionary::makeEntry(const PaintParamsKey& key,
                                                             const skgpu::BlendInfo& blendInfo) {
    uint8_t* newKeyData = fArena.makeArray<uint8_t>(key.sizeInBytes());
    memcpy(newKeyData, key.data(), key.sizeInBytes());

    SkSpan<const uint8_t> newKeyAsSpan = SkSpan(newKeyData, key.sizeInBytes());
    return fArena.make([&](void *ptr) { return new(ptr) Entry(newKeyAsSpan, blendInfo); });
}

size_t ShaderCodeDictionary::PaintParamsKeyPtr::Hash::operator()(PaintParamsKeyPtr p) const {
    return SkOpts::hash_fn(p.fKey->data(), p.fKey->sizeInBytes(), 0);
}

size_t ShaderCodeDictionary::RuntimeEffectKey::Hash::operator()(RuntimeEffectKey k) const {
    return SkOpts::hash_fn(&k, sizeof(k), 0);
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::findOrCreate(
        PaintParamsKeyBuilder* builder) {
    PaintParamsKey key = builder->lockAsKey();

    SkAutoSpinlock lock{fSpinLock};

    Entry** existingEntry = fHash.find(PaintParamsKeyPtr{&key});
    if (existingEntry) {
        SkASSERT(fEntryVector[(*existingEntry)->uniqueID().asUInt()] == *existingEntry);
        return *existingEntry;
    }

    Entry* newEntry = this->makeEntry(key, builder->blendInfo());
    newEntry->setUniqueID(fEntryVector.size());
    fHash.set(PaintParamsKeyPtr{&newEntry->paintParamsKey()}, newEntry);
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::lookup(
        UniquePaintParamsID codeID) const {

    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

SkSpan<const Uniform> ShaderCodeDictionary::getUniforms(BuiltInCodeSnippetID id) const {
    return fBuiltInCodeSnippets[(int) id].fUniforms;
}

SkSpan<const DataPayloadField> ShaderCodeDictionary::dataPayloadExpectations(
        int codeSnippetID) const {
    // All callers of this entry point should already have ensured that 'codeSnippetID' is valid
    return this->getEntry(codeSnippetID)->fDataPayloadExpectations;
}

const ShaderSnippet* ShaderCodeDictionary::getEntry(int codeSnippetID) const {
    if (codeSnippetID < 0) {
        return nullptr;
    }

    if (codeSnippetID < kBuiltInCodeSnippetIDCount) {
        return &fBuiltInCodeSnippets[codeSnippetID];
    }

    int userDefinedCodeSnippetID = codeSnippetID - kBuiltInCodeSnippetIDCount;
    if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
        return fUserDefinedCodeSnippets[userDefinedCodeSnippetID].get();
    }

    return nullptr;
}

void ShaderCodeDictionary::getShaderInfo(UniquePaintParamsID uniqueID,
                                         ShaderInfo* info) const {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(this, info);
    info->setBlendInfo(entry->blendInfo());
}

//--------------------------------------------------------------------------------------------------
namespace {

static std::string append_default_snippet_arguments(const ShaderInfo& shaderInfo,
                                                    const ShaderSnippet* entry,
                                                    int entryIndex,
                                                    const ShaderSnippet::Args& args,
                                                    SkSpan<const std::string> childOutputs) {
    std::string code = "(";

    const char* separator = "";

    // Append prior-stage output color.
    if (entry->needsPriorStageOutput()) {
        code += args.fPriorStageOutput;
        separator = ", ";
    }

    // Append destination color.
    if (entry->needsDestColor()) {
        code += separator;
        code += args.fDestColor;
        separator = ", ";
    }

    // Append fragment coordinates.
    if (entry->needsLocalCoords()) {
        code += separator;
        code += args.fFragCoord;
        separator = ", ";
    }

    // Append uniform names.
    for (size_t i = 0; i < entry->fUniforms.size(); ++i) {
        code += separator;
        separator = ", ";
        code += entry->getMangledUniformName(shaderInfo, i, entryIndex);
    }

    // Append samplers.
    for (size_t i = 0; i < entry->fTexturesAndSamplers.size(); ++i) {
        code += separator;
        code += entry->getMangledSamplerName(i, entryIndex);
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

static void emit_helper_function(const ShaderInfo& shaderInfo,
                                 int* entryIndex,
                                 std::string* preamble) {
    const PaintParamsKey::BlockReader& reader = shaderInfo.blockReader(*entryIndex);
    const ShaderSnippet* entry = reader.entry();

    const int numChildren = reader.numChildren();
    SkASSERT(numChildren == entry->fNumChildren);

    // Advance over the parent entry.
    int curEntryIndex = *entryIndex;
    *entryIndex += 1;

    // Create a helper function that invokes each of the children, then calls the entry's snippet
    // and passes all the child outputs along as arguments.
    std::string helperFnName = get_mangled_name(entry->fStaticFunctionName, curEntryIndex);
    std::string helperFn = SkSL::String::printf(
            "half4 %s(half4 inColor, half4 destColor, float2 pos) {",
            helperFnName.c_str());
    std::vector<std::string> childOutputVarNames;
    const ShaderSnippet::Args args = {"inColor", "destColor", "pos"};
    for (int j = 0; j < numChildren; ++j) {
        // Emit glue code into our helper function body.
        std::string childOutputVar = emit_glue_code_for_entry(shaderInfo, *entryIndex, args,
                                                              &helperFn);
        childOutputVarNames.push_back(std::move(childOutputVar));

        // If this entry itself requires a preamble, handle that here. This will advance the
        // entry index forward as required.
        emit_preamble_for_entry(shaderInfo, entryIndex, preamble);
    }

    // Finally, invoke the snippet from the helper function, passing uniforms and child outputs.
    std::string snippetArgList = append_default_snippet_arguments(shaderInfo, entry, curEntryIndex,
                                                                  args, childOutputVarNames);
    SkSL::String::appendf(&helperFn,
                              "return %s%s;"
                          "}",
                          entry->fStaticFunctionName, snippetArgList.c_str());

    // Add our new helper function to the bottom of the preamble.
    *preamble += helperFn;
}

// If we have no children, the default expression just calls a built-in snippet with the signature:
//     half4 BuiltinFunctionName(/* default snippet arguments */);
//
// If we do have children, we will have created a glue function in the preamble and that is called
// instead. Its signature looks like this:
//     half4 BuiltinFunctionName_N(half4 inColor, half4 destColor, float2 pos);

std::string GenerateDefaultExpression(const ShaderInfo& shaderInfo,
                                      int entryIndex,
                                      const PaintParamsKey::BlockReader& reader,
                                      const ShaderSnippet::Args& args) {
    const ShaderSnippet* entry = reader.entry();
    if (entry->fNumChildren == 0) {
        // We don't have any children; return an expression which invokes the snippet directly.
        return entry->fStaticFunctionName + append_default_snippet_arguments(shaderInfo,
                                                                             entry,
                                                                             entryIndex,
                                                                             args,
                                                                             /*childOutputs=*/{});
    } else {
        // Return an expression which invokes the helper function from the preamble.
        std::string helperFnName = get_mangled_name(entry->fStaticFunctionName, entryIndex);
        return SkSL::String::printf("%s(%.*s, %.*s, %.*s)",
                                  helperFnName.c_str(),
                                  (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
                                  (int)args.fDestColor.size(),        args.fDestColor.data(),
                                  (int)args.fFragCoord.size(),        args.fFragCoord.data());
    }
}

// If we have no children, we don't need to add anything into the preamble.
// If we have child entries, we create a function in the preamble with a signature of:
//     half4 BuiltinFunctionName_N(half4 inColor, half4 destColor, float2 pos) { ... }
// This function invokes each child in sequence, and then calls the built-in function, passing all
// uniforms and child outputs along:
//     half4 BuiltinFunctionName(/* all uniforms as parameters */,
//                               /* all child output variable names as parameters */);
void GenerateDefaultPreamble(const ShaderInfo& shaderInfo,
                             int* entryIndex,
                             const PaintParamsKey::BlockReader& reader,
                             std::string* preamble) {
    const ShaderSnippet* entry = reader.entry();

    if (entry->fNumChildren > 0) {
        // Create a helper function which invokes all the child snippets.
        emit_helper_function(shaderInfo, entryIndex, preamble);
    } else {
        // We don't need a helper function; just advance over this entry.
        SkASSERT(reader.numChildren() == 0);
        *entryIndex += 1;
    }
}

//--------------------------------------------------------------------------------------------------
static constexpr int kFourStopGradient = 4;
static constexpr int kEightStopGradient = 8;

static constexpr Uniform kLinearGradientUniforms4[7] = {
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};
static constexpr Uniform kLinearGradientUniforms8[7] = {
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr Uniform kRadialGradientUniforms4[7] = {
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};
static constexpr Uniform kRadialGradientUniforms8[7] = {
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr Uniform kSweepGradientUniforms4[8] = {
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};
static constexpr Uniform kSweepGradientUniforms8[8] = {
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr Uniform kConicalGradientUniforms4[9] = {
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};
static constexpr Uniform kConicalGradientUniforms8[9] = {
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "colorSpace",  SkSLType::kInt },
        { "doUnPremul",  SkSLType::kInt },
};

static constexpr char kLinearGradient4Name[] = "sk_linear_grad_4_shader";
static constexpr char kLinearGradient8Name[] = "sk_linear_grad_8_shader";
static constexpr char kRadialGradient4Name[] = "sk_radial_grad_4_shader";
static constexpr char kRadialGradient8Name[] = "sk_radial_grad_8_shader";
static constexpr char kSweepGradient4Name[] = "sk_sweep_grad_4_shader";
static constexpr char kSweepGradient8Name[] = "sk_sweep_grad_8_shader";
static constexpr char kConicalGradient4Name[] = "sk_conical_grad_4_shader";
static constexpr char kConicalGradient8Name[] = "sk_conical_grad_8_shader";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kSolidShaderUniforms[] = {
        { "color", SkSLType::kFloat4 }
};

static constexpr char kSolidShaderName[] = "sk_solid_shader";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kLocalMatrixShaderUniforms[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
};

static constexpr int kNumLocalMatrixShaderChildren = 1;

static constexpr char kLocalMatrixShaderName[] = "LocalMatrix";

void GenerateLocalMatrixPreamble(const ShaderInfo& shaderInfo,
                                 int* entryIndex,
                                 const PaintParamsKey::BlockReader& reader,
                                 std::string* preamble) {
    const ShaderSnippet* entry = reader.entry();
    SkASSERT(entry->fNumChildren == kNumLocalMatrixShaderChildren);

    // Advance over the parent entry.
    int curEntryIndex = *entryIndex;
    *entryIndex += 1;

    // Get the child's evaluation expression.
    static constexpr char kUnusedDestColor[] = "half4(1)";
    std::string childExpr = emit_expression_for_entry(shaderInfo, *entryIndex,
                                                      {"inColor", kUnusedDestColor, "coords"});
    // Emit preamble code for child.
    emit_preamble_for_entry(shaderInfo, entryIndex, preamble);

    std::string localMatrixUni = reader.entry()->getMangledUniformName(shaderInfo, 0,
                                                                       curEntryIndex);

    /**
     * Create a helper function that multiplies coordinates by a local matrix, invokes the child
     * entry with those updated coordinates, and returns the result. This helper function meets the
     * requirements for use with GenerateDefaultExpression, so there's no need to have a separate
     * special GenerateLocalMatrixExpression.
     */
    std::string helperFnName = get_mangled_name(entry->fStaticFunctionName, curEntryIndex);
    SkSL::String::appendf(preamble,
                          "half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                              "coords = (%s * coords.xy01).xy;"
                              "return %s;"
                          "}",
                          helperFnName.c_str(),
                          localMatrixUni.c_str(),
                          childExpr.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumXferFnCoeffs = 7;

static constexpr Uniform kImageShaderUniforms[] = {
        { "imgSize",               SkSLType::kFloat2 },
        { "subset",                SkSLType::kFloat4 },
        { "tilemodeX",             SkSLType::kInt },
        { "tilemodeY",             SkSLType::kInt },
        { "filterMode",            SkSLType::kInt },
        { "useCubic",              SkSLType::kInt },
        { "cubicCoeffs",           SkSLType::kFloat4x4 },
        { "readSwizzle",           SkSLType::kInt },
        // The next 6 uniforms are for the color space transformation
        { "csXformFlags",          SkSLType::kInt },
        { "csXformSrcKind",        SkSLType::kInt },
        { "csXformDstKind",        SkSLType::kInt },
        { "csXformSrcCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "csXformDstCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "csXformGamutTransform", SkSLType::kHalf3x3 },
};

static constexpr TextureAndSampler kISTexturesAndSamplers[] = {
        {"sampler"},
};

static_assert(0 == static_cast<int>(SkTileMode::kClamp),  "ImageShader code depends on SkTileMode");
static_assert(1 == static_cast<int>(SkTileMode::kRepeat), "ImageShader code depends on SkTileMode");
static_assert(2 == static_cast<int>(SkTileMode::kMirror), "ImageShader code depends on SkTileMode");
static_assert(3 == static_cast<int>(SkTileMode::kDecal),  "ImageShader code depends on SkTileMode");

static_assert(0 == static_cast<int>(SkFilterMode::kNearest),
              "ImageShader code depends on SkFilterMode");
static_assert(1 == static_cast<int>(SkFilterMode::kLinear),
              "ImageShader code depends on SkFilterMode");

static_assert(0 == static_cast<int>(ReadSwizzle::kRGBA),
              "ImageShader code depends on ReadSwizzle");
static_assert(1 == static_cast<int>(ReadSwizzle::kRGB1),
              "ImageShader code depends on ReadSwizzle");
static_assert(2 == static_cast<int>(ReadSwizzle::kRRRR),
              "ImageShader code depends on ReadSwizzle");
static_assert(3 == static_cast<int>(ReadSwizzle::kRRR1),
              "ImageShader code depends on ReadSwizzle");
static_assert(4 == static_cast<int>(ReadSwizzle::kBGRA),
              "ImageShader code depends on ReadSwizzle");

static constexpr char kImageShaderName[] = "sk_image_shader";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kPorterDuffBlendShaderUniforms[] = {
        { "blendConstants", SkSLType::kHalf4 },
};

static constexpr char kPorterDuffBlendShaderName[] = "porter_duff_blend_shader";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kBlendShaderUniforms[] = {
        { "blendMode", SkSLType::kInt },
};

static constexpr char kBlendShaderName[] = "sk_blend_shader";

static constexpr int kNumBlendShaderChildren = 2;

//--------------------------------------------------------------------------------------------------
static constexpr char kColorFilterShaderName[] = "ColorFilterShader";

static constexpr int kNumColorFilterShaderChildren = 2;

//--------------------------------------------------------------------------------------------------
static constexpr char kRuntimeShaderName[] = "RuntimeEffect";

class GraphitePipelineCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    GraphitePipelineCallbacks(const ShaderInfo& shaderInfo,
                              int entryIndex,
                              const std::vector<int>& childEntryIndices,
                              std::string* preamble)
            : fShaderInfo(shaderInfo)
            , fEntryIndex(entryIndex)
            , fChildEntryIndices(childEntryIndices)
            , fPreamble(preamble) {}

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        std::string result = get_mangled_name(std::string(decl->var()->name()), fEntryIndex);
        if (fShaderInfo.ssboIndex()) {
            result = EmitStorageBufferAccess("fs", fShaderInfo.ssboIndex(), result.c_str());
        }
        return result;
    }

    void defineFunction(const char* decl, const char* body, bool isMain) override {
        if (isMain) {
            SkSL::String::appendf(
                 fPreamble,
                 "half4 %s_%d(half4 inColor, half4 destColor, float2 coords) {"
                     "float2 pos = coords;"
                     "%s"
                 "}",
                 kRuntimeShaderName,
                 fEntryIndex,
                 body);
        } else {
            SkSL::String::appendf(fPreamble, "%s {%s}\n", decl, body);
        }
    }

    void declareFunction(const char* decl) override {
        *fPreamble += std::string(decl) + ";";
    }

    void defineStruct(const char* definition) override {
        *fPreamble += std::string(definition) + ";";
    }

    void declareGlobal(const char* declaration) override {
        *fPreamble += std::string(declaration) + ";";
    }

    std::string sampleShader(int index, std::string coords) override {
        SkASSERT(index >= 0 && index < (int)fChildEntryIndices.size());
        return emit_expression_for_entry(fShaderInfo, fChildEntryIndices[index],
                                         {"inColor", "destColor", coords});
    }

    std::string sampleColorFilter(int index, std::string color) override {
        SkASSERT(index >= 0 && index < (int)fChildEntryIndices.size());
        return emit_expression_for_entry(fShaderInfo, fChildEntryIndices[index],
                                         {color, "destColor", "coords"});
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        return emit_expression_for_entry(fShaderInfo, fChildEntryIndices[index],
                                         {src, dst, "coords"});
    }

    std::string toLinearSrgb(std::string color) override {
        // TODO(skia:13508): implement to-linear-SRGB child effect
        return color;
    }
    std::string fromLinearSrgb(std::string color) override {
        // TODO(skia:13508): implement from-linear-SRGB child effect
        return color;
    }

    std::string getMangledName(const char* name) override {
        return get_mangled_name(name, fEntryIndex);
    }

private:
    const ShaderInfo& fShaderInfo;
    int fEntryIndex;
    const std::vector<int>& fChildEntryIndices;
    std::string* fPreamble;
};

void GenerateRuntimeShaderPreamble(const ShaderInfo& shaderInfo,
                                   int* entryIndex,
                                   const PaintParamsKey::BlockReader& reader,
                                   std::string* preamble) {
    const ShaderSnippet* entry = reader.entry();

    // Advance over the parent entry.
    int curEntryIndex = *entryIndex;
    *entryIndex += 1;

    // Emit the preambles for all of our child effects (and advance the entry-index past them).
    // This computes the indices of our child effects, which we use when invoking them below.
    std::vector<int> childEntryIndices;
    childEntryIndices.reserve(entry->fNumChildren);
    for (int j = 0; j < entry->fNumChildren; ++j) {
        childEntryIndices.push_back(*entryIndex);
        emit_preamble_for_entry(shaderInfo, entryIndex, preamble);
    }

    // Find this runtime effect in the runtime-effect dictionary.
    const int codeSnippetId = reader.codeSnippetId();
    const SkRuntimeEffect* effect = shaderInfo.runtimeEffectDictionary()->find(codeSnippetId);
    SkASSERT(effect);
    const SkSL::Program& program = SkRuntimeEffectPriv::Program(*effect);

    GraphitePipelineCallbacks callbacks{shaderInfo, curEntryIndex, childEntryIndices, preamble};
    SkASSERT(std::string_view(entry->fName) == kRuntimeShaderName);  // the callbacks assume this
    SkSL::PipelineStage::ConvertProgram(program, "pos", "inColor", "destColor", &callbacks);
}

std::string GenerateRuntimeShaderExpression(const ShaderInfo& shaderInfo,
                                            int entryIndex,
                                            const PaintParamsKey::BlockReader& reader,
                                            const ShaderSnippet::Args& args) {
    const ShaderSnippet* entry = reader.entry();
    return SkSL::String::printf("%s_%d(%.*s, %.*s, %.*s)",
                                entry->fName,
                                entryIndex,
                                (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
                                (int)args.fDestColor.size(),        args.fDestColor.data(),
                                (int)args.fFragCoord.size(),        args.fFragCoord.data());
}

//--------------------------------------------------------------------------------------------------
// TODO: investigate the implications of having separate hlsa and rgba matrix colorfilters. It
// may be that having them separate will not contribute to combinatorial explosion.
static constexpr Uniform kMatrixColorFilterUniforms[] = {
        { "matrix",    SkSLType::kFloat4x4 },
        { "translate", SkSLType::kFloat4 },
        { "inHSL",     SkSLType::kInt },
};

static constexpr char kMatrixColorFilterName[] = "sk_matrix_colorfilter";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kBlendColorFilterUniforms[] = {
        { "blendMode", SkSLType::kInt },
        { "color",     SkSLType::kFloat4 }
};

static constexpr char kBlendColorFilterName[] = "sk_blend_colorfilter";

//--------------------------------------------------------------------------------------------------
static constexpr char kComposeColorFilterName[] = "ComposeColorFilter";

static constexpr int kNumComposeColorFilterChildren = 2;

void GenerateNestedChildrenPreamble(const ShaderInfo& shaderInfo,
                                    int* entryIndex,
                                    const PaintParamsKey::BlockReader& reader,
                                    std::string* preamble) {
    const ShaderSnippet* entry = reader.entry();
    SkASSERT(entry->fNumChildren == 2);

    // Advance over the parent entry.
    int curEntryIndex = *entryIndex;
    *entryIndex += 1;

    // Evaluate inner child.
    static constexpr char kUnusedDestColor[] = "half4(1)";
    std::string innerColor = emit_expression_for_entry(shaderInfo, *entryIndex, {"inColor",
                                                       kUnusedDestColor, "coords"});

    // Emit preamble code for inner child.
    emit_preamble_for_entry(shaderInfo, entryIndex, preamble);

    // Evaluate outer child.
    std::string outerColor = emit_expression_for_entry(shaderInfo, *entryIndex, {innerColor,
                                                       kUnusedDestColor, "coords"});

    // Emit preamble code for outer child.
    emit_preamble_for_entry(shaderInfo, entryIndex, preamble);

    // Create a helper function that invokes the inner expression, then passes that result to the
    // outer expression, and returns the composed result.
    std::string helperFnName = get_mangled_name(entry->fStaticFunctionName, curEntryIndex);
    SkSL::String::appendf(
            preamble,
            "half4 %s(half4 inColor, half4 destColor, float2 coords) {"
                "return %s;"
            "}",
            helperFnName.c_str(),
            outerColor.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr TextureAndSampler kTableColorFilterTexturesAndSamplers[] = {
        {"tableSampler"},
};

static constexpr char kTableColorFilterName[] = "sk_table_colorfilter";

//--------------------------------------------------------------------------------------------------
static constexpr char kGaussianColorFilterName[] = "sk_gaussian_colorfilter";

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kColorSpaceTransformUniforms[] = {
        { "flags",          SkSLType::kInt },
        { "srcKind",        SkSLType::kInt },
        { "dstKind",        SkSLType::kInt },
        { "srcCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "dstCoeffs",      SkSLType::kHalf, kNumXferFnCoeffs },
        { "gamutTransform", SkSLType::kHalf3x3 },
};

static_assert(0 == static_cast<int>(skcms_TFType_Invalid),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(1 == static_cast<int>(skcms_TFType_sRGBish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(2 == static_cast<int>(skcms_TFType_PQish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(3 == static_cast<int>(skcms_TFType_HLGish),
              "ColorSpaceTransform code depends on skcms_TFType");
static_assert(4 == static_cast<int>(skcms_TFType_HLGinvish),
              "ColorSpaceTransform code depends on skcms_TFType");

// TODO: We can meaningfully check these when we can use C++20 features.
// static_assert(0x1 == SkColorSpaceXformSteps::Flags{.unpremul = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x2 == SkColorSpaceXformSteps::Flags{.linearize = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x4 == SkColorSpaceXformSteps::Flags{.gamut_transform = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x8 == SkColorSpaceXformSteps::Flags{.encode = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");
// static_assert(0x10 == SkColorSpaceXformSteps::Flags{.premul = true}.mask(),
//               "ColorSpaceTransform code depends on SkColorSpaceXformSteps::Flags");

static constexpr char kColorSpaceTransformName[] = "sk_color_space_transform";

//--------------------------------------------------------------------------------------------------
static constexpr char kErrorName[] = "sk_error";

//--------------------------------------------------------------------------------------------------
static constexpr char kPassthroughShaderName[] = "sk_passthrough";

//--------------------------------------------------------------------------------------------------
static constexpr char kPassthroughBlenderName[] = "blend_src_over";

//--------------------------------------------------------------------------------------------------
static constexpr PaintParamsKey::DataPayloadField kFixedFunctionDataFields[] = {
    { "blendMode", PaintParamsKey::DataPayloadType::kByte, 1},
};

// This method generates the glue code for the case where the SkBlendMode-based blending is
// handled with fixed function blending.
std::string GenerateFixedFunctionBlenderExpression(const ShaderInfo&,
                                                   int entryIndex,
                                                   const PaintParamsKey::BlockReader& reader,
                                                   const ShaderSnippet::Args& args) {
    SkASSERT(reader.entry()->fUniforms.empty());
    SkASSERT(reader.numDataPayloadFields() == 1);

    // The actual blending is set up via the fixed function pipeline so we don't actually
    // need to access the blend mode in the glue code.
    return std::string(args.fPriorStageOutput);
}

//--------------------------------------------------------------------------------------------------
static constexpr Uniform kShaderBasedBlenderUniforms[] = {
        { "blendMode", SkSLType::kInt },
};

static constexpr char kBlendHelperName[] = "sk_blend";

// This method generates the glue code for the case where the SkBlendMode-based blending must occur
// in the shader (i.e., fixed function blending isn't possible).
// It exists as custom glue code so that we can deal with the dest reads. If that can be
// standardized (e.g., via a snippets requirement flag) this could be removed.
std::string GenerateShaderBasedBlenderExpression(const ShaderInfo& shaderInfo,
                                                 int entryIndex,
                                                 const PaintParamsKey::BlockReader& reader,
                                                 const ShaderSnippet::Args& args) {
    const bool usePrimitiveColorAsDst = reader.entry()->needsDestColor();

    SkASSERT(reader.entry()->fUniforms.size() == 1);
    SkASSERT(reader.numDataPayloadFields() == 0);

    std::string uniformName = reader.entry()->getMangledUniformName(shaderInfo, 0, entryIndex);

    // TODO: emit function to perform dest read into preamble, and replace half4(1) with that call
    // (The `args.destColor` variable might seem tempting here, but this is used for programmatic
    // src+dest blends within the shader, not for blends against the destination surface.)
    const char * destColor = usePrimitiveColorAsDst ? "primitiveColor" : "half4(1)";

    return SkSL::String::printf("%s(%s, %.*s, %s)",
                                reader.entry()->fStaticFunctionName,
                                uniformName.c_str(),
                                (int)args.fPriorStageOutput.size(), args.fPriorStageOutput.data(),
                                destColor);
}

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

bool ShaderCodeDictionary::isValidID(int snippetID) const {
    if (snippetID < 0) {
        return false;
    }

    if (snippetID < kBuiltInCodeSnippetIDCount) {
        return true;
    }

    int userDefinedCodeSnippetID = snippetID - kBuiltInCodeSnippetIDCount;
    return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
}

static constexpr int kNoChildren = 0;

int ShaderCodeDictionary::addUserDefinedSnippet(
        const char* name,
        SkSpan<const Uniform> uniforms,
        SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
        SkSpan<const TextureAndSampler> texturesAndSamplers,
        const char* functionName,
        ShaderSnippet::GenerateExpressionForSnippetFn expressionGenerator,
        ShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
        int numChildren,
        SkSpan<const PaintParamsKey::DataPayloadField> dataPayloadExpectations) {
    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(std::make_unique<ShaderSnippet>(name,
                                                                       uniforms,
                                                                       snippetRequirementFlags,
                                                                       texturesAndSamplers,
                                                                       functionName,
                                                                       expressionGenerator,
                                                                       preambleGenerator,
                                                                       numChildren,
                                                                       dataPayloadExpectations));

    return kBuiltInCodeSnippetIDCount + fUserDefinedCodeSnippets.size() - 1;
}

// TODO: this version needs to be removed
int ShaderCodeDictionary::addUserDefinedSnippet(
        const char* name,
        SkSpan<const DataPayloadField> dataPayloadExpectations) {
    return this->addUserDefinedSnippet("UserDefined",
                                       {},  // no uniforms
                                       SnippetRequirementFlags::kNone,
                                       {},  // no samplers
                                       name,
                                       GenerateDefaultExpression,
                                       GenerateDefaultPreamble,
                                       kNoChildren,
                                       dataPayloadExpectations);
}

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
            case Type::kInt:      return SkSLType::kShort;
            case Type::kInt2:     return SkSLType::kShort2;
            case Type::kInt3:     return SkSLType::kShort3;
            case Type::kInt4:     return SkSLType::kShort4;
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

    // Convert the SkRuntimeEffect::Uniform array into its Uniform equivalent.
    int numUniforms = uniforms.size();
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

int ShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
    // Use the combination of {SkSL program hash, uniform size} as our key.
    // In the unfortunate event of a hash collision, at least we'll have the right amount of
    // uniform data available.
    RuntimeEffectKey key;
    key.fHash = SkRuntimeEffectPriv::Hash(*effect);
    key.fUniformSize = effect->uniformSize();

    SkAutoSpinlock lock{fSpinLock};

    int32_t* existingCodeSnippetID = fRuntimeEffectMap.find(key);
    if (existingCodeSnippetID) {
        return *existingCodeSnippetID;
    }

    SkEnumBitMask<SnippetRequirementFlags> snippetFlags = SnippetRequirementFlags::kNone;
    if (effect->allowShader()) {
        snippetFlags |= SnippetRequirementFlags::kLocalCoords;
    }
    if (effect->allowBlender()) {
        snippetFlags |= SnippetRequirementFlags::kDestColor;
    }
    int newCodeSnippetID = this->addUserDefinedSnippet("RuntimeEffect",
                                                       this->convertUniforms(effect),
                                                       snippetFlags,
                                                       /*texturesAndSamplers=*/{},
                                                       kRuntimeShaderName,
                                                       GenerateRuntimeShaderExpression,
                                                       GenerateRuntimeShaderPreamble,
                                                       (int)effect->children().size(),
                                                       /*dataPayloadExpectations=*/{});
    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
}

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kError] = {
            "Error",
            { },     // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kErrorName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPassthroughShader] = {
            "PassthroughShader",
            { },     // no uniforms
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kPassthroughShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPassthroughBlender] = {
            "PassthroughBlender",
            {},      // no uniforms
            SnippetRequirementFlags::kPriorStageOutput | SnippetRequirementFlags::kDestColor,
            {},      // no samplers
            kPassthroughBlenderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            {}       // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSolidColorShader] = {
            "SolidColor",
            SkSpan(kSolidShaderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kSolidShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader4] = {
            "LinearGradient4",
            SkSpan(kLinearGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLinearGradientShader8] = {
            "LinearGradient8",
            SkSpan(kLinearGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient8Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader4] = {
            "RadialGradient4",
            SkSpan(kRadialGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient4Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kRadialGradientShader8] = {
            "RadialGradient8",
            SkSpan(kRadialGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient8Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader4] = {
            "SweepGradient4",
            SkSpan(kSweepGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient4Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kSweepGradientShader8] = {
            "SweepGradient8",
            SkSpan(kSweepGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient8Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader4] = {
            "ConicalGradient4",
            SkSpan(kConicalGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient4Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kConicalGradientShader8] = {
            "ConicalGradient8",
            SkSpan(kConicalGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient8Name,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kLocalMatrixShader] = {
            "LocalMatrixShader",
            SkSpan(kLocalMatrixShaderUniforms),
            (SnippetRequirementFlags::kPriorStageOutput |
             SnippetRequirementFlags::kLocalCoords),
            { },     // no samplers
            kLocalMatrixShaderName,
            GenerateDefaultExpression,
            GenerateLocalMatrixPreamble,
            kNumLocalMatrixShaderChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kImageShader] = {
            "ImageShader",
            SkSpan(kImageShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kISTexturesAndSamplers),
            kImageShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPorterDuffBlendShader] = {
            "PorterDuffBlendShader",
            SkSpan(kPorterDuffBlendShaderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kPorterDuffBlendShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNumBlendShaderChildren,
            { }      // no data payload
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendShader] = {
            "BlendShader",
            SkSpan(kBlendShaderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendShaderName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNumBlendShaderChildren,
            { }      // no data payload
    };

    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorFilterShader] = {
            "ColorFilterShader",
            {},      // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kColorFilterShaderName,
            GenerateDefaultExpression,
            GenerateNestedChildrenPreamble,
            kNumColorFilterShaderChildren,
            { }      // no data payload
    };

    // SkColorFilter snippets
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kMatrixColorFilter] = {
            "MatrixColorFilter",
            SkSpan(kMatrixColorFilterUniforms),
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kMatrixColorFilterName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kBlendColorFilter] = {
            "BlendColorFilter",
            SkSpan(kBlendColorFilterUniforms),
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kBlendColorFilterName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kComposeColorFilter] = {
            "ComposeColorFilter",
            { },     // no uniforms
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kComposeColorFilterName,
            GenerateDefaultExpression,
            GenerateNestedChildrenPreamble,
            kNumComposeColorFilterChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kTableColorFilter] = {
            "TableColorFilter",
            { },     // no uniforms
            SnippetRequirementFlags::kPriorStageOutput,
            SkSpan(kTableColorFilterTexturesAndSamplers),
            kTableColorFilterName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kGaussianColorFilter] = {
            "GaussianColorFilter",
            { },     // no uniforms
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kGaussianColorFilterName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformColorFilter] = {
            "ColorSpaceTransform",
            SkSpan(kColorSpaceTransformUniforms),
            SnippetRequirementFlags::kPriorStageOutput,
            { },     // no samplers
            kColorSpaceTransformName,
            GenerateDefaultExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kFixedFunctionBlender] = {
            "FixedFunctionBlender",
            { },     // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            "FF-blending",  // fixed function blending doesn't use static SkSL
            GenerateFixedFunctionBlenderExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            kFixedFunctionDataFields
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kShaderBasedBlender] = {
            "ShaderBasedBlender",
            SkSpan(kShaderBasedBlenderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
    fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kPrimitiveColorShaderBasedBlender] = {
            "PrimitiveColorShaderBasedBlender",
            SkSpan(kShaderBasedBlenderUniforms),
            SnippetRequirementFlags::kDestColor,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderExpression,
            GenerateDefaultPreamble,
            kNoChildren,
            { }      // no data payload
    };
}

} // namespace skgpu::graphite
