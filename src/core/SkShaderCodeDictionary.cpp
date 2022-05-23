/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkShaderCodeDictionary.h"

#include "include/core/SkCombinationBuilder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"
#include "src/sksl/SkSLUtil.h"

#ifdef SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#endif

// We need to ensure that the user-defined snippet ID can't conflict with the SkBlendMode
// values (since they are used "raw" in the combination system).
static const int kMinUserDefinedSnippetID = std::max(kBuiltInCodeSnippetIDCount, kSkBlendModeCount);

namespace {

std::string get_mangled_local_var_name(const char* baseName, int manglingSuffix) {
    return std::string(baseName) + "_" + std::to_string(manglingSuffix);
}

void add_indent(std::string* result, int indent) {
    result->append(4*indent, ' ');
}

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
std::string generate_default_before_children_glue_code(int entryIndex,
                                                       const SkPaintParamsKey::BlockReader& reader,
                                                       const std::string& parentPreLocalName,
                                                       int indent) {
    std::string result;

    if (reader.entry()->needsLocalCoords()) {
        // Every snippet that requests local coordinates must have a preLocalMatrix as its first
        // uniform
        SkASSERT(reader.entry()->fUniforms.size() >= 1);
        SkASSERT(reader.entry()->fUniforms[0].type() == SkSLType::kFloat4x4);

        std::string localMatrixUniformName = reader.entry()->getMangledUniformName(0, entryIndex);

        std::string preLocalMatrixVarName = get_mangled_local_var_name("preLocal", entryIndex);

        add_indent(&result, indent);
        SkSL::String::appendf(&result,
                              "float4x4 %s = %s * %s;\n",
                              preLocalMatrixVarName.c_str(),
                              parentPreLocalName.c_str(),
                              localMatrixUniformName.c_str());
    }

    return result;
}
#endif

} // anonymous namespace


std::string SkShaderSnippet::getMangledUniformName(int uniformIndex, int mangleId) const {
    std::string result;
    result = fUniforms[uniformIndex].name() + std::string("_") + std::to_string(mangleId);
    return result;
}

// TODO: SkShaderInfo::toSkSL needs to work outside of both just graphite and metal. To do
// so we'll need to switch over to using SkSL's uniform capabilities.
#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

// TODO: switch this over to using SkSL's uniform system
namespace skgpu::graphite {
std::string GetMtlUniforms(int bufferID,
                           const char* name,
                           const std::vector<SkPaintParamsKey::BlockReader>&,
                           bool needsDev2Local);
std::string GetMtlTexturesAndSamplers(const std::vector<SkPaintParamsKey::BlockReader>&,
                                      int* binding);
} // namespace skgpu::graphite

// Emit the glue code needed to invoke a single static helper isolated w/in its own scope.
// The structure of this will be:
//
//     half4 outColor%d;
//     {
//         half4 child-outColor%d;  // for each child
//         {
//             /* emitted snippet sksl assigns to child-outColor%d */
//         }
//
//         /* emitted snippet sksl assigns to outColor%d - taking a vector of child var names */
//     }
// Where the %d is filled in with 'entryIndex'.
std::string SkShaderInfo::emitGlueCodeForEntry(int* entryIndex,
                                               const std::string& priorStageOutputName,
                                               const std::string& parentPreLocalName,
                                               std::string* result,
                                               int indent) const {
    const SkPaintParamsKey::BlockReader& reader = fBlockReaders[*entryIndex];
    int curEntryIndex = *entryIndex;

    std::string scopeOutputVar = get_mangled_local_var_name("outColor", curEntryIndex);

    add_indent(result, indent);
    SkSL::String::appendf(result,
                          "half4 %s; // output of %s\n",
                          scopeOutputVar.c_str(),
                          reader.entry()->fName);
    add_indent(result, indent);
    *result += "{\n";

    *result += generate_default_before_children_glue_code(curEntryIndex, reader,
                                                          parentPreLocalName, indent+1);

    // TODO: this could be returned by generate_default_before_children_glue_code
    std::string currentPreLocalName;
    if (reader.entry()->needsLocalCoords()) {
        currentPreLocalName = get_mangled_local_var_name("preLocal", curEntryIndex);
    } else {
        currentPreLocalName = parentPreLocalName;
    }

    // Although the children appear after the parent in the shader info they are emitted
    // before the parent
    std::vector<std::string> childOutputVarNames;
    for (int j = 0; j < reader.numChildren(); ++j) {
        *entryIndex += 1;
        std::string childOutputVar = this->emitGlueCodeForEntry(entryIndex,
                                                                priorStageOutputName,
                                                                currentPreLocalName,
                                                                result, indent+1);
        childOutputVarNames.push_back(childOutputVar);
    }

    *result += (reader.entry()->fGlueCodeGenerator)(scopeOutputVar, curEntryIndex, reader,
                                                    priorStageOutputName,
                                                    childOutputVarNames, indent+1);
    add_indent(result, indent);
    *result += "}\n";

    return scopeOutputVar;
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
std::string SkShaderInfo::toSkSL() const {
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    std::string result = skgpu::graphite::GetMtlUniforms(2, "FS", fBlockReaders,
                                                         this->needsLocalCoords());

    int binding = 0;
    result += skgpu::graphite::GetMtlTexturesAndSamplers(fBlockReaders, &binding);
    result += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
    result += "void main() {\n";

    if (this->needsLocalCoords()) {
        result += "const float4x4 initialPreLocal = float4x4(1);\n";
    }

    std::string parentPreLocal = "initialPreLocal";
    std::string lastOutputVar = "initialColor";

    // TODO: what is the correct initial color to feed in?
    add_indent(&result, 1);
    SkSL::String::appendf(&result, "    half4 %s = half4(0.0);", lastOutputVar.c_str());

    for (int entryIndex = 0; entryIndex < (int) fBlockReaders.size(); ++entryIndex) {
        lastOutputVar = this->emitGlueCodeForEntry(&entryIndex, lastOutputVar, parentPreLocal,
                                                   &result, 1);
    }

    SkSL::String::appendf(&result, "    sk_FragColor = %s;\n", lastOutputVar.c_str());
    result += "}\n";

    return result;
}
#endif

SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::makeEntry(
        const SkPaintParamsKey& key
#ifdef SK_GRAPHITE_ENABLED
        , const SkPipelineDataGatherer::BlendInfo& blendInfo
#endif
        ) {
    uint8_t* newKeyData = fArena.makeArray<uint8_t>(key.sizeInBytes());
    memcpy(newKeyData, key.data(), key.sizeInBytes());

    SkSpan<const uint8_t> newKeyAsSpan = SkMakeSpan(newKeyData, key.sizeInBytes());
#ifdef SK_GRAPHITE_ENABLED
    return fArena.make([&](void *ptr) { return new(ptr) Entry(newKeyAsSpan, blendInfo); });
#else
    return fArena.make([&](void *ptr) { return new(ptr) Entry(newKeyAsSpan); });
#endif
}

size_t SkShaderCodeDictionary::Hash::operator()(const SkPaintParamsKey* key) const {
    return SkOpts::hash_fn(key->data(), key->sizeInBytes(), 0);
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::findOrCreate(
        const SkPaintParamsKey& key
#ifdef SK_GRAPHITE_ENABLED
        , const SkPipelineDataGatherer::BlendInfo& blendInfo
#endif
        ) {
    SkAutoSpinlock lock{fSpinLock};

    auto iter = fHash.find(&key);
    if (iter != fHash.end()) {
        SkASSERT(fEntryVector[iter->second->uniqueID().asUInt()] == iter->second);
        return iter->second;
    }

#ifdef SK_GRAPHITE_ENABLED
    Entry* newEntry = this->makeEntry(key, blendInfo);
#else
    Entry* newEntry = this->makeEntry(key);
#endif
    newEntry->setUniqueID(fEntryVector.size());
    fHash.insert(std::make_pair(&newEntry->paintParamsKey(), newEntry));
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::lookup(
        SkUniquePaintParamsID codeID) const {

    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

SkSpan<const SkUniform> SkShaderCodeDictionary::getUniforms(SkBuiltInCodeSnippetID id) const {
    return fBuiltInCodeSnippets[(int) id].fUniforms;
}

SkSpan<const SkPaintParamsKey::DataPayloadField> SkShaderCodeDictionary::dataPayloadExpectations(
        int codeSnippetID) const {
    // All callers of this entry point should already have ensured that 'codeSnippetID' is valid
    return this->getEntry(codeSnippetID)->fDataPayloadExpectations;
}

const SkShaderSnippet* SkShaderCodeDictionary::getEntry(int codeSnippetID) const {
    if (codeSnippetID < 0) {
        return nullptr;
    }

    if (codeSnippetID < kBuiltInCodeSnippetIDCount) {
        return &fBuiltInCodeSnippets[codeSnippetID];
    }

    if (codeSnippetID < kMinUserDefinedSnippetID) {
        return nullptr;
    }

    int userDefinedCodeSnippetID = codeSnippetID - kMinUserDefinedSnippetID;
    if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
        return fUserDefinedCodeSnippets[userDefinedCodeSnippetID].get();
    }

    return nullptr;
}

const SkShaderSnippet* SkShaderCodeDictionary::getEntry(SkBlenderID id) const {
    return this->getEntry(id.asUInt());
}

void SkShaderCodeDictionary::getShaderInfo(SkUniquePaintParamsID uniqueID, SkShaderInfo* info) {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(this, info);

#ifdef SK_GRAPHITE_ENABLED
    info->setBlendInfo(entry->blendInfo());
#endif
}

//--------------------------------------------------------------------------------------------------
namespace {

using DataPayloadField = SkPaintParamsKey::DataPayloadField;

// The default glue code just calls a helper function with the signature:
//    half4 fStaticFunctionName(/* all uniforms as parameters */,
//                              /* all child output variable names as parameters */);
// and stores the result in a variable named "resultName".
std::string GenerateDefaultGlueCode(const std::string& resultName,
                                    int entryIndex,
                                    const SkPaintParamsKey::BlockReader& reader,
                                    const std::string& priorStageOutputName,
                                    const std::vector<std::string>& childOutputVarNames,
                                    int indent) {
    const SkShaderSnippet* entry = reader.entry();

    SkASSERT((int)childOutputVarNames.size() == entry->numExpectedChildren());

    if (entry->needsLocalCoords()) {
        // Every snippet that requests local coordinates must have a localMatrix as its first
        // uniform
        SkASSERT(reader.entry()->fUniforms.size() >= 1);
        SkASSERT(reader.entry()->fUniforms[0].type() == SkSLType::kFloat4x4);
    }

    std::string result;

    add_indent(&result, indent);
    SkSL::String::appendf(&result,
                          "%s = %s(",
                          resultName.c_str(),
                          entry->fStaticFunctionName);
    for (size_t i = 0; i < entry->fUniforms.size(); ++i) {
        if (i == 0 && reader.entry()->needsLocalCoords()) {
            std::string preLocalMatrixVarName = get_mangled_local_var_name("preLocal",
                                                                           entryIndex);
            result += preLocalMatrixVarName;
            result += " * dev2LocalUni";
        } else {
            result += entry->getMangledUniformName(i, entryIndex);
        }
        if (i+1 < entry->fUniforms.size() + childOutputVarNames.size()) {
            result += ", ";
        }
    }
    for (size_t i = 0; i < childOutputVarNames.size(); ++i) {
        result += childOutputVarNames[i].c_str();
        if (i+1 < childOutputVarNames.size()) {
            result += ", ";
        }
    }
    result += ");\n";

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kFourStopGradient = 4;
static constexpr int kEightStopGradient = 8;

static constexpr int kNumLinearGradientUniforms = 9;
static constexpr SkUniform kLinearGradientUniforms4[kNumLinearGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
        { "padding1",    SkSLType::kFloat }, // TODO: add automatic uniform padding
        { "padding2",    SkSLType::kFloat },
        { "padding3",    SkSLType::kFloat },
};
static constexpr SkUniform kLinearGradientUniforms8[kNumLinearGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
        { "padding1",    SkSLType::kFloat }, // TODO: add automatic uniform padding
        { "padding2",    SkSLType::kFloat },
        { "padding3",    SkSLType::kFloat },
};

static constexpr int kNumRadialGradientUniforms = 6;
static constexpr SkUniform kRadialGradientUniforms4[kNumRadialGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};
static constexpr SkUniform kRadialGradientUniforms8[kNumRadialGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};

static constexpr int kNumSweepGradientUniforms = 10;
static constexpr SkUniform kSweepGradientUniforms4[kNumSweepGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "padding1",    SkSLType::kFloat }, // TODO: add automatic uniform padding
        { "padding2",    SkSLType::kFloat },
        { "padding3",    SkSLType::kFloat },
};
static constexpr SkUniform kSweepGradientUniforms8[kNumSweepGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "padding1",    SkSLType::kFloat }, // TODO: add automatic uniform padding
        { "padding2",    SkSLType::kFloat },
        { "padding3",    SkSLType::kFloat },
};

static constexpr int kNumConicalGradientUniforms = 9;
static constexpr SkUniform kConicalGradientUniforms4[kNumConicalGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "padding",     SkSLType::kFloat }, // TODO: add automatic uniform padding
};
static constexpr SkUniform kConicalGradientUniforms8[kNumConicalGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
        { "padding",     SkSLType::kFloat }, // TODO: add automatic uniform padding
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
static constexpr int kNumSolidShaderUniforms = 1;
static constexpr SkUniform kSolidShaderUniforms[kNumSolidShaderUniforms] = {
        { "color", SkSLType::kFloat4 }
};

static constexpr char kSolidShaderName[] = "sk_solid_shader";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumLocalMatrixShaderUniforms = 1;
static constexpr SkUniform kLocalMatrixShaderUniforms[kNumLocalMatrixShaderUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
};

static constexpr int kNumLocalMatrixShaderChildren = 1;

static constexpr char kLocalMatrixShaderName[] = "sk_local_matrix_shader";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumImageShaderUniforms = 6;
static constexpr SkUniform kImageShaderUniforms[kNumImageShaderUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "subset",      SkSLType::kFloat4 },
        { "tilemodeX",   SkSLType::kInt },
        { "tilemodeY",   SkSLType::kInt },
        { "imgWidth",    SkSLType::kInt },
        { "imgHeight",   SkSLType::kInt },
};

static constexpr int kNumImageShaderTexturesAndSamplers = 1;
static constexpr SkTextureAndSampler kISTexturesAndSamplers[kNumImageShaderTexturesAndSamplers] = {
        {"sampler"},
};

static_assert(0 == static_cast<int>(SkTileMode::kClamp),  "ImageShader code depends on SkTileMode");
static_assert(1 == static_cast<int>(SkTileMode::kRepeat), "ImageShader code depends on SkTileMode");
static_assert(2 == static_cast<int>(SkTileMode::kMirror), "ImageShader code depends on SkTileMode");
static_assert(3 == static_cast<int>(SkTileMode::kDecal),  "ImageShader code depends on SkTileMode");

static constexpr char kImageShaderName[] = "sk_compute_coords";

// This is _not_ what we want to do.
// Ideally the "compute_coords" code snippet could just take texture and
// sampler references and do everything. That is going to take more time to figure out though so,
// for the sake of expediency, we're generating custom code to do the sampling.
std::string GenerateImageShaderGlueCode(const std::string& resultName,
                                        int entryIndex,
                                        const SkPaintParamsKey::BlockReader& reader,
                                        const std::string& priorStageOutputName,
                                        const std::vector<std::string>& childNames,
                                        int indent) {
    SkASSERT(childNames.empty());

    std::string samplerVarName = std::string("sampler_") + std::to_string(entryIndex) + "_0";
    std::string preLocalMatrixVarName = get_mangled_local_var_name("preLocal", entryIndex);

    // Uniform slot 0 is being used for the localMatrix but is handled in
    // generate_default_before_children_glue_code.
    std::string subsetName = reader.entry()->getMangledUniformName(1, entryIndex);
    std::string tmXName = reader.entry()->getMangledUniformName(2, entryIndex);
    std::string tmYName = reader.entry()->getMangledUniformName(3, entryIndex);
    std::string imgWidthName = reader.entry()->getMangledUniformName(4, entryIndex);
    std::string imgHeightName = reader.entry()->getMangledUniformName(5, entryIndex);

    std::string result;

    add_indent(&result, indent);
    SkSL::String::appendf(&result,
                          "float2 coords = %s(%s * dev2LocalUni, %s, %s, %s, %s, %s);",
                          reader.entry()->fStaticFunctionName,
                          preLocalMatrixVarName.c_str(),
                          subsetName.c_str(),
                          tmXName.c_str(),
                          tmYName.c_str(),
                          imgWidthName.c_str(),
                          imgHeightName.c_str());

    add_indent(&result, indent);
    SkSL::String::appendf(&result,
                          "%s = sample(%s, coords);\n",
                          resultName.c_str(),
                          samplerVarName.c_str());

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumBlendShaderUniforms = 4;
static constexpr SkUniform kBlendShaderUniforms[kNumBlendShaderUniforms] = {
        { "blendMode", SkSLType::kInt },
        { "padding1",  SkSLType::kInt }, // TODO: add automatic uniform padding
        { "padding2",  SkSLType::kInt },
        { "padding3",  SkSLType::kInt },
};

static constexpr int kNumBlendShaderChildren = 2;

static constexpr char kBlendShaderName[] = "sk_blend_shader";

//--------------------------------------------------------------------------------------------------
static constexpr char kErrorName[] = "sk_error";

//--------------------------------------------------------------------------------------------------
// This method generates the glue code for the case where the SkBlendMode-based blending is
// handled with fixed function blending.
std::string GenerateFixedFunctionBlenderGlueCode(const std::string& resultName,
                                                 int entryIndex,
                                                 const SkPaintParamsKey::BlockReader& reader,
                                                 const std::string& priorStageOutputName,
                                                 const std::vector<std::string>& childNames,
                                                 int indent) {
    SkASSERT(childNames.empty());
    SkASSERT(reader.entry()->fUniforms.empty());
    SkASSERT(reader.numDataPayloadFields() == 0);

    // The actual blending is set up via the fixed function pipeline so we don't actually
    // need to access the blend mode in the glue code.

    std::string result;
    add_indent(&result, indent);
    result += "// Fixed-function blending\n";
    add_indent(&result, indent);
    SkSL::String::appendf(&result, "%s = %s;", resultName.c_str(), priorStageOutputName.c_str());

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kNumShaderBasedBlenderUniforms = 4;
static constexpr SkUniform kShaderBasedBlenderUniforms[kNumShaderBasedBlenderUniforms] = {
        { "blendMode", SkSLType::kInt },
        { "padding1",  SkSLType::kInt }, // TODO: add automatic uniform padding
        { "padding2",  SkSLType::kInt },
        { "padding3",  SkSLType::kInt },
};

static constexpr char kBlendHelperName[] = "sk_blend";

// This method generates the glue code for the case where the SkBlendMode-based blending must occur
// in the shader (i.e., fixed function blending isn't possible).
// It exists as custom glue code so that we can deal with the dest reads. If that can be
// standardized (e.g., via a snippets requirement flag) this could be removed.
std::string GenerateShaderBasedBlenderGlueCode(const std::string& resultName,
                                               int entryIndex,
                                               const SkPaintParamsKey::BlockReader& reader,
                                               const std::string& priorStageOutputName,
                                               const std::vector<std::string>& childNames,
                                               int indent) {
    SkASSERT(childNames.empty());
    SkASSERT(reader.entry()->fUniforms.size() == 4); // actual blend uniform + 3 padding int
    SkASSERT(reader.numDataPayloadFields() == 0);

    std::string uniformName = reader.entry()->getMangledUniformName(0, entryIndex);

    std::string result;

    add_indent(&result, indent);
    result += "// Shader-based blending\n";

    // TODO: emit code to perform dest read here
    add_indent(&result, indent);
    result += "half4 dummyDst = half4(1.0, 1.0, 1.0, 1.0);\n";

    add_indent(&result, indent);
    SkSL::String::appendf(&result, "%s = %s(%s, %s, dummyDst);",
                          resultName.c_str(),
                          reader.entry()->fStaticFunctionName,
                          uniformName.c_str(),
                          priorStageOutputName.c_str());

    return result;
}

//--------------------------------------------------------------------------------------------------

} // anonymous namespace

bool SkShaderCodeDictionary::isValidID(int snippetID) const {
    if (snippetID < 0) {
        return false;
    }

    if (snippetID < kBuiltInCodeSnippetIDCount) {
        return true;
    }

    if (snippetID < kMinUserDefinedSnippetID) {
        return false;
    }

    int userDefinedCodeSnippetID = snippetID - kMinUserDefinedSnippetID;
    return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
}

static constexpr int kNoChildren = 0;

// TODO: this version needs to be removed
int SkShaderCodeDictionary::addUserDefinedSnippet(
        const char* name,
        SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations) {

    std::unique_ptr<SkShaderSnippet> entry(new SkShaderSnippet("UserDefined",
                                                               {}, // no uniforms
                                                               SnippetRequirementFlags::kNone,
                                                               {}, // no samplers
                                                               name,
                                                               GenerateDefaultGlueCode,
                                                               kNoChildren,
                                                               dataPayloadExpectations));

    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(std::move(entry));

    return kMinUserDefinedSnippetID + fUserDefinedCodeSnippets.size() - 1;
}

SkBlenderID SkShaderCodeDictionary::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    if (!effect) {
        return {};
    }

    // TODO: at this point we need to extract the uniform definitions, children and helper functions
    // from the runtime effect in order to create a real SkShaderSnippet
    // Additionally, we need to hash the provided code to deduplicate the runtime effects in case
    // the client keeps giving us different rtEffects w/ the same backing SkSL.

    std::unique_ptr<SkShaderSnippet> entry(new SkShaderSnippet("UserDefined",
                                                               {}, // missing uniforms
                                                               SnippetRequirementFlags::kNone,
                                                               {}, // missing samplers
                                                               "foo",
                                                               GenerateDefaultGlueCode,
                                                               kNoChildren,
                                                               {}));  // missing data payload

    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(std::move(entry));

    return SkBlenderID(kMinUserDefinedSnippetID + fUserDefinedCodeSnippets.size() - 1);
}

SkShaderCodeDictionary::SkShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kError] = {
            "Error",
            { },     // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kErrorName,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSolidColorShader] = {
            "SolidColor",
            SkMakeSpan(kSolidShaderUniforms, kNumSolidShaderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kSolidShaderName,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader4] = {
            "LinearGradient4",
            SkMakeSpan(kLinearGradientUniforms4, kNumLinearGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader8] = {
            "LinearGradient8",
            SkMakeSpan(kLinearGradientUniforms8, kNumLinearGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader4] = {
            "RadialGradient4",
            SkMakeSpan(kRadialGradientUniforms4, kNumRadialGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader8] = {
            "RadialGradient8",
            SkMakeSpan(kRadialGradientUniforms8, kNumRadialGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader4] = {
            "SweepGradient4",
            SkMakeSpan(kSweepGradientUniforms4, kNumSweepGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader8] = {
            "SweepGradient8",
            SkMakeSpan(kSweepGradientUniforms8, kNumSweepGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader4] = {
            "ConicalGradient4",
            SkMakeSpan(kConicalGradientUniforms4, kNumConicalGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader8] = {
            "ConicalGradient8",
            SkMakeSpan(kConicalGradientUniforms8, kNumConicalGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLocalMatrixShader] = {
            "LocalMatrixShader",
            SkMakeSpan(kLocalMatrixShaderUniforms, kNumLocalMatrixShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLocalMatrixShaderName,
            GenerateDefaultGlueCode,
            kNumLocalMatrixShaderChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kImageShader] = {
            "ImageShader",
            SkMakeSpan(kImageShaderUniforms, kNumImageShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            SkMakeSpan(kISTexturesAndSamplers, kNumImageShaderTexturesAndSamplers),
            kImageShaderName,
            GenerateImageShaderGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kBlendShader] = {
            "BlendShader",
            { kBlendShaderUniforms, kNumBlendShaderUniforms },
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendShaderName,
            GenerateDefaultGlueCode,
            kNumBlendShaderChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kFixedFunctionBlender] = {
            "FixedFunctionBlender",
            { },     // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            "FF-blending",  // fixed function blending doesn't use static SkSL
            GenerateFixedFunctionBlenderGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kShaderBasedBlender] = {
            "ShaderBasedBlender",
            { kShaderBasedBlenderUniforms, kNumShaderBasedBlenderUniforms },
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderGlueCode,
            kNoChildren,
            { }
    };
}
