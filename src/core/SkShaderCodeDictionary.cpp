/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkShaderCodeDictionary.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/sksl/SkSLUtil.h"

#ifdef SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#endif

#ifdef SK_ENABLE_PRECOMPILE
#include "include/core/SkCombinationBuilder.h"
#endif

using DataPayloadField = SkPaintParamsKey::DataPayloadField;
using DataPayloadType = SkPaintParamsKey::DataPayloadType;

namespace {

std::string get_mangled_name(const char* baseName, int manglingSuffix) {
    return std::string(baseName) + "_" + std::to_string(manglingSuffix);
}

void add_indent(std::string* result, int indent) {
    result->append(4*indent, ' ');
}

#if defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
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

        std::string preLocalMatrixVarName = get_mangled_name("preLocal", entryIndex);

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
#if defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

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
                                               std::string* preamble,
                                               std::string* mainBody,
                                               int indent) const {
    const SkPaintParamsKey::BlockReader& reader = fBlockReaders[*entryIndex];
    int curEntryIndex = *entryIndex;

    std::string scopeOutputVar = get_mangled_name("outColor", curEntryIndex);

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody,
                          "half4 %s; // output of %s\n",
                          scopeOutputVar.c_str(),
                          reader.entry()->fName);
    add_indent(mainBody, indent);
    *mainBody += "{\n";

    *mainBody += generate_default_before_children_glue_code(curEntryIndex, reader,
                                                            parentPreLocalName, indent + 1);

    // TODO: this could be returned by generate_default_before_children_glue_code
    std::string currentPreLocalName;
    if (reader.entry()->needsLocalCoords()) {
        currentPreLocalName = get_mangled_name("preLocal", curEntryIndex);
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
                                                                preamble, mainBody, indent + 1);
        childOutputVarNames.push_back(childOutputVar);
    }

    (reader.entry()->fGlueCodeGenerator)(scopeOutputVar, curEntryIndex, reader,
                                         priorStageOutputName, childOutputVarNames,
                                         preamble, mainBody, indent + 1);
    add_indent(mainBody, indent);
    *mainBody += "}\n";

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
    std::string preamble = "layout(location = 0, index = 0) out half4 sk_FragColor;\n";

    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    // TODO: replace hard-coded bufferID of 2 with the backend's paint uniform-buffer index.
    preamble += skgpu::graphite::GetMtlUniforms(/*bufferID=*/2, "FS", fBlockReaders,
                                                this->needsLocalCoords());
    int binding = 0;
    preamble += skgpu::graphite::GetMtlTexturesAndSamplers(fBlockReaders, &binding);

    std::string mainBody = "void main() {\n";

    if (this->needsLocalCoords()) {
        mainBody += "    const float4x4 initialPreLocal = float4x4(1);\n";
    }

    std::string parentPreLocal = "initialPreLocal";
    std::string lastOutputVar = "initialColor";

    // TODO: what is the correct initial color to feed in?
    add_indent(&mainBody, 1);
    SkSL::String::appendf(&mainBody, "    half4 %s = half4(0);", lastOutputVar.c_str());

    for (int entryIndex = 0; entryIndex < (int) fBlockReaders.size(); ++entryIndex) {
        lastOutputVar = this->emitGlueCodeForEntry(&entryIndex, lastOutputVar, parentPreLocal,
                                                   &preamble, &mainBody, 1);
    }

    SkSL::String::appendf(&mainBody, "    sk_FragColor = %s;\n", lastOutputVar.c_str());
    mainBody += "}\n";

    return preamble + "\n" + mainBody;
}
#endif

SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::makeEntry(
        const SkPaintParamsKey& key
#ifdef SK_GRAPHITE_ENABLED
        , const skgpu::BlendInfo& blendInfo
#endif
        ) {
    uint8_t* newKeyData = fArena.makeArray<uint8_t>(key.sizeInBytes());
    memcpy(newKeyData, key.data(), key.sizeInBytes());

    SkSpan<const uint8_t> newKeyAsSpan = SkSpan(newKeyData, key.sizeInBytes());
#ifdef SK_GRAPHITE_ENABLED
    return fArena.make([&](void *ptr) { return new(ptr) Entry(newKeyAsSpan, blendInfo); });
#else
    return fArena.make([&](void *ptr) { return new(ptr) Entry(newKeyAsSpan); });
#endif
}

size_t SkShaderCodeDictionary::SkPaintParamsKeyPtr::Hash::operator()(SkPaintParamsKeyPtr p) const {
    return SkOpts::hash_fn(p.fKey->data(), p.fKey->sizeInBytes(), 0);
}

size_t SkShaderCodeDictionary::RuntimeEffectKey::Hash::operator()(RuntimeEffectKey k) const {
    return SkOpts::hash_fn(&k, sizeof(k), 0);
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::findOrCreate(
        SkPaintParamsKeyBuilder* builder) {
    const SkPaintParamsKey& key = builder->lockAsKey();

    SkAutoSpinlock lock{fSpinLock};

    Entry** existingEntry = fHash.find(SkPaintParamsKeyPtr{&key});
    if (existingEntry) {
        SkASSERT(fEntryVector[(*existingEntry)->uniqueID().asUInt()] == *existingEntry);
        return *existingEntry;
    }

#ifdef SK_GRAPHITE_ENABLED
    Entry* newEntry = this->makeEntry(key, builder->blendInfo());
#else
    Entry* newEntry = this->makeEntry(key);
#endif
    newEntry->setUniqueID(fEntryVector.size());
    fHash.set(SkPaintParamsKeyPtr{&newEntry->paintParamsKey()}, newEntry);
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

SkSpan<const DataPayloadField> SkShaderCodeDictionary::dataPayloadExpectations(
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

    int userDefinedCodeSnippetID = codeSnippetID - kBuiltInCodeSnippetIDCount;
    if (userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size())) {
        return fUserDefinedCodeSnippets[userDefinedCodeSnippetID].get();
    }

    return nullptr;
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

// The default glue code just calls a helper function with the signature:
//    half4 fStaticFunctionName(/* all uniforms as parameters */,
//                              /* all child output variable names as parameters */);
// and stores the result in a variable named "resultName".
void GenerateDefaultGlueCode(const std::string& resultName,
                             int entryIndex,
                             const SkPaintParamsKey::BlockReader& reader,
                             const std::string& priorStageOutputName,
                             const std::vector<std::string>& childOutputVarNames,
                             std::string* preamble,
                             std::string* mainBody,
                             int indent) {
    const SkShaderSnippet* entry = reader.entry();

    SkASSERT((int)childOutputVarNames.size() == entry->fNumChildren);

    if (entry->needsLocalCoords()) {
        // Every snippet that requests local coordinates must have a localMatrix as its first
        // uniform
        SkASSERT(reader.entry()->fUniforms.size() >= 1);
        SkASSERT(reader.entry()->fUniforms[0].type() == SkSLType::kFloat4x4);
    }

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody,
                          "%s = %s(",
                          resultName.c_str(),
                          entry->fStaticFunctionName);
    const char* separator = "";
    for (size_t i = 0; i < entry->fUniforms.size(); ++i) {
        *mainBody += separator;
        separator = ", ";

        if (i == 0 && reader.entry()->needsLocalCoords()) {
            *mainBody += get_mangled_name("preLocal", entryIndex);
            *mainBody += " * dev2LocalUni";
        } else {
            *mainBody += entry->getMangledUniformName(i, entryIndex);
        }
    }
    for (size_t i = 0; i < childOutputVarNames.size(); ++i) {
        *mainBody += separator;
        separator = ", ";

        *mainBody += childOutputVarNames[i];
    }
    *mainBody += ");\n";
}

//--------------------------------------------------------------------------------------------------
static constexpr int kFourStopGradient = 4;
static constexpr int kEightStopGradient = 8;

static constexpr SkUniform kLinearGradientUniforms4[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
};
static constexpr SkUniform kLinearGradientUniforms8[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "tilemode",    SkSLType::kInt },
};

static constexpr SkUniform kRadialGradientUniforms4[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};
static constexpr SkUniform kRadialGradientUniforms8[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "radius",      SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};

static constexpr SkUniform kSweepGradientUniforms4[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};
static constexpr SkUniform kSweepGradientUniforms8[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "center",      SkSLType::kFloat2 },
        { "bias",        SkSLType::kFloat },
        { "scale",       SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};

static constexpr SkUniform kConicalGradientUniforms4[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
};
static constexpr SkUniform kConicalGradientUniforms8[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kEightStopGradient },
        { "offsets",     SkSLType::kFloat,  kEightStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "tilemode",    SkSLType::kInt },
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
static constexpr SkUniform kSolidShaderUniforms[] = {
        { "color", SkSLType::kFloat4 }
};

static constexpr char kSolidShaderName[] = "sk_solid_shader";

//--------------------------------------------------------------------------------------------------
static constexpr SkUniform kLocalMatrixShaderUniforms[] = {
        { "localMatrix", SkSLType::kFloat4x4 },
};

static constexpr int kNumLocalMatrixShaderChildren = 1;

static constexpr char kLocalMatrixShaderName[] = "sk_local_matrix_shader";

//--------------------------------------------------------------------------------------------------
static constexpr SkUniform kImageShaderUniforms[] = {
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
void GenerateImageShaderGlueCode(const std::string& resultName,
                                 int entryIndex,
                                 const SkPaintParamsKey::BlockReader& reader,
                                 const std::string& priorStageOutputName,
                                 const std::vector<std::string>& childNames,
                                 std::string* preamble,
                                 std::string* mainBody,
                                 int indent) {
    SkASSERT(childNames.empty());

    std::string samplerVarName = std::string("sampler_") + std::to_string(entryIndex) + "_0";
    std::string preLocalMatrixVarName = get_mangled_name("preLocal", entryIndex);

    // Uniform slot 0 is being used for the localMatrix but is handled in
    // generate_default_before_children_glue_code.
    std::string subsetName = reader.entry()->getMangledUniformName(1, entryIndex);
    std::string tmXName = reader.entry()->getMangledUniformName(2, entryIndex);
    std::string tmYName = reader.entry()->getMangledUniformName(3, entryIndex);
    std::string imgWidthName = reader.entry()->getMangledUniformName(4, entryIndex);
    std::string imgHeightName = reader.entry()->getMangledUniformName(5, entryIndex);

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody,
                          "float2 coords = %s(%s * dev2LocalUni, %s, %s, %s, %s, %s);",
                          reader.entry()->fStaticFunctionName,
                          preLocalMatrixVarName.c_str(),
                          subsetName.c_str(),
                          tmXName.c_str(),
                          tmYName.c_str(),
                          imgWidthName.c_str(),
                          imgHeightName.c_str());

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody,
                          "%s = sample(%s, coords);\n",
                          resultName.c_str(),
                          samplerVarName.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr SkUniform kBlendShaderUniforms[] = {
        { "blendMode", SkSLType::kInt },
};

static constexpr int kNumBlendShaderChildren = 2;

static constexpr char kBlendShaderName[] = "sk_blend_shader";

//--------------------------------------------------------------------------------------------------
static constexpr char kRuntimeShaderName[] = "RuntimeEffect";

void GenerateRuntimeShaderGlueCode(const std::string& resultName,
                                   int entryIndex,
                                   const SkPaintParamsKey::BlockReader& reader,
                                   const std::string& priorStageOutputName,
                                   const std::vector<std::string>& childOutputVarNames,
                                   std::string* preamble,
                                   std::string* mainBody,
                                   int indent) {
    const SkShaderSnippet* entry = reader.entry();

    // We prepend a preLocalMatrix as the first uniform, ahead of the runtime effect's uniforms.
    // TODO: we can eliminate this uniform entirely if it's the identity matrix.
    // TODO: if we could inherit the parent's transform, this could be removed entirely.
    SkASSERT(entry->needsLocalCoords());
    SkASSERT(reader.entry()->fUniforms[0].type() == SkSLType::kFloat4x4);

    SkSL::String::appendf(preamble, R"(
half4 %s_%d(float2 coords, half4 color) {
    // TODO: Runtime effect code goes here
    return half4(0.75, 0.0, 1.0, 1.0);
}
)", entry->fName, entryIndex);

    std::string preLocalMatrixVarName = get_mangled_name("preLocal", entryIndex);

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody,
                          "%s = %s_%d((%s * dev2LocalUni * sk_FragCoord).xy, (%s));\n",
                          resultName.c_str(),
                          entry->fName, entryIndex,
                          preLocalMatrixVarName.c_str(),
                          priorStageOutputName.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr char kErrorName[] = "sk_error";

//--------------------------------------------------------------------------------------------------
// This method generates the glue code for the case where the SkBlendMode-based blending is
// handled with fixed function blending.
void GenerateFixedFunctionBlenderGlueCode(const std::string& resultName,
                                          int entryIndex,
                                          const SkPaintParamsKey::BlockReader& reader,
                                          const std::string& priorStageOutputName,
                                          const std::vector<std::string>& childNames,
                                          std::string* preamble,
                                          std::string* mainBody,
                                          int indent) {
    SkASSERT(childNames.empty());
    SkASSERT(reader.entry()->fUniforms.empty());
    SkASSERT(reader.numDataPayloadFields() == 0);

    // The actual blending is set up via the fixed function pipeline so we don't actually
    // need to access the blend mode in the glue code.

    add_indent(mainBody, indent);
    *mainBody += "// Fixed-function blending\n";
    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody, "%s = %s;", resultName.c_str(), priorStageOutputName.c_str());
}

//--------------------------------------------------------------------------------------------------
static constexpr SkUniform kShaderBasedBlenderUniforms[] = {
        { "blendMode", SkSLType::kInt },
};

static constexpr char kBlendHelperName[] = "sk_blend";

// This method generates the glue code for the case where the SkBlendMode-based blending must occur
// in the shader (i.e., fixed function blending isn't possible).
// It exists as custom glue code so that we can deal with the dest reads. If that can be
// standardized (e.g., via a snippets requirement flag) this could be removed.
void GenerateShaderBasedBlenderGlueCode(const std::string& resultName,
                                        int entryIndex,
                                        const SkPaintParamsKey::BlockReader& reader,
                                        const std::string& priorStageOutputName,
                                        const std::vector<std::string>& childNames,
                                        std::string* preamble,
                                        std::string* mainBody,
                                        int indent) {
    SkASSERT(childNames.empty());
    SkASSERT(reader.entry()->fUniforms.size() == 1);
    SkASSERT(reader.numDataPayloadFields() == 0);

    std::string uniformName = reader.entry()->getMangledUniformName(0, entryIndex);

    add_indent(mainBody, indent);
    *mainBody += "// Shader-based blending\n";

    // TODO: emit code to perform dest read here
    add_indent(mainBody, indent);
    *mainBody += "half4 dummyDst = half4(1);\n";

    add_indent(mainBody, indent);
    SkSL::String::appendf(mainBody, "%s = %s(%s, %s, dummyDst);",
                          resultName.c_str(),
                          reader.entry()->fStaticFunctionName,
                          uniformName.c_str(),
                          priorStageOutputName.c_str());
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

    int userDefinedCodeSnippetID = snippetID - kBuiltInCodeSnippetIDCount;
    return userDefinedCodeSnippetID < SkTo<int>(fUserDefinedCodeSnippets.size());
}

static constexpr int kNoChildren = 0;

int SkShaderCodeDictionary::addUserDefinedSnippet(
        const char* name,
        SkSpan<const SkUniform> uniforms,
        SnippetRequirementFlags snippetRequirementFlags,
        SkSpan<const SkTextureAndSampler> texturesAndSamplers,
        const char* functionName,
        SkShaderSnippet::GenerateGlueCodeForEntry glueCodeGenerator,
        int numChildren,
        SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations) {
    // TODO: the memory for user-defined entries could go in the dictionary's arena but that
    // would have to be a thread safe allocation since the arena also stores entries for
    // 'fHash' and 'fEntryVector'
    fUserDefinedCodeSnippets.push_back(std::make_unique<SkShaderSnippet>(name,
                                                                         uniforms,
                                                                         snippetRequirementFlags,
                                                                         texturesAndSamplers,
                                                                         functionName,
                                                                         glueCodeGenerator,
                                                                         numChildren,
                                                                         dataPayloadExpectations));

    return kBuiltInCodeSnippetIDCount + fUserDefinedCodeSnippets.size() - 1;
}

// TODO: this version needs to be removed
int SkShaderCodeDictionary::addUserDefinedSnippet(
        const char* name,
        SkSpan<const DataPayloadField> dataPayloadExpectations) {
    return this->addUserDefinedSnippet("UserDefined",
                                       {},  // no uniforms
                                       SnippetRequirementFlags::kNone,
                                       {},  // no samplers
                                       name,
                                       GenerateDefaultGlueCode,
                                       kNoChildren,
                                       dataPayloadExpectations);
}

#ifdef SK_ENABLE_PRECOMPILE
SkBlenderID SkShaderCodeDictionary::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    if (!effect) {
        return {};
    }

    // TODO: at this point we need to extract the uniform definitions, children and helper functions
    // from the runtime effect in order to create a real SkShaderSnippet
    // Additionally, we need to hash the provided code to deduplicate the runtime effects in case
    // the client keeps giving us different rtEffects w/ the same backing SkSL.
    int codeSnippetID = this->addUserDefinedSnippet("UserDefined",
                                                    {},  // missing uniforms
                                                    SnippetRequirementFlags::kNone,
                                                    {},  // missing samplers
                                                    "foo",
                                                    GenerateDefaultGlueCode,
                                                    kNoChildren,
                                                    /*dataPayloadExpectations=*/{});
    return SkBlenderID(codeSnippetID);
}

const SkShaderSnippet* SkShaderCodeDictionary::getEntry(SkBlenderID id) const {
    return this->getEntry(id.asUInt());
}

#endif // SK_ENABLE_PRECOMPILE

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

const char* SkShaderCodeDictionary::addTextToArena(std::string_view text) {
    char* textInArena = fArena.makeArrayDefault<char>(text.size() + 1);
    memcpy(textInArena, text.data(), text.size());
    textInArena[text.size()] = '\0';
    return textInArena;
}

SkSpan<const SkUniform> SkShaderCodeDictionary::convertUniforms(const SkRuntimeEffect* effect) {
    using Uniform = SkRuntimeEffect::Uniform;
    SkSpan<const Uniform> uniforms = effect->uniforms();

    // Convert the SkRuntimeEffect::Uniform array into its SkUniform equivalent.
    int numUniforms = uniforms.size() + 1;
    SkUniform* uniformArray = fArena.makeInitializedArray<SkUniform>(numUniforms, [&](int index) {
        // Graphite wants a `localMatrix` float4x4 uniform at the front of the uniform list.
        if (index == 0) {
            return SkUniform("localMatrix", SkSLType::kFloat4x4);
        }
        const Uniform& u = uniforms[index - 1];

        // The existing uniform names are in SkStrings and may disappear. Copy them into fArena.
        // (It's safe to do this within makeInitializedArray; the entire array is allocated in one
        // big slab before any initialization calls are done.)
        const char* name = this->addTextToArena(std::string_view(u.name.c_str(), u.name.size()));

        // Add one SkUniform to our array.
        SkSLType type = uniform_type_to_sksl_type(u);
        return (u.flags & Uniform::kArray_Flag) ? SkUniform(name, type, u.count)
                                                : SkUniform(name, type);
    });

    return SkSpan<const SkUniform>(uniformArray, numUniforms);
}

int SkShaderCodeDictionary::findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect) {
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

    // TODO(skia:13405): consider removing these data fields, they don't seem to add value anymore
    static constexpr DataPayloadField kRuntimeShaderDataPayload[] = {
            {"runtime effect hash", DataPayloadType::kInt, 1},
            {"uniform data size (bytes)", DataPayloadType::kInt, 1},
    };

    // TODO(skia:13405): arguments to `addUserDefinedSnippet` here are placeholder
    int newCodeSnippetID = this->addUserDefinedSnippet("RuntimeEffect",
                                                       this->convertUniforms(effect),
                                                       SnippetRequirementFlags::kLocalCoords,
                                                       /*texturesAndSamplers=*/{},
                                                       kRuntimeShaderName,
                                                       GenerateRuntimeShaderGlueCode,
                                                       /*numChildren=*/0,
                                                       SkSpan(kRuntimeShaderDataPayload));
    fRuntimeEffectMap.set(key, newCodeSnippetID);
    return newCodeSnippetID;
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
            SkSpan(kSolidShaderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kSolidShaderName,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader4] = {
            "LinearGradient4",
            SkSpan(kLinearGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader8] = {
            "LinearGradient8",
            SkSpan(kLinearGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader4] = {
            "RadialGradient4",
            SkSpan(kRadialGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader8] = {
            "RadialGradient8",
            SkSpan(kRadialGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kRadialGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader4] = {
            "SweepGradient4",
            SkSpan(kSweepGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader8] = {
            "SweepGradient8",
            SkSpan(kSweepGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kSweepGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader4] = {
            "ConicalGradient4",
            SkSpan(kConicalGradientUniforms4),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader8] = {
            "ConicalGradient8",
            SkSpan(kConicalGradientUniforms8),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kConicalGradient8Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLocalMatrixShader] = {
            "LocalMatrixShader",
            SkSpan(kLocalMatrixShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLocalMatrixShaderName,
            GenerateDefaultGlueCode,
            kNumLocalMatrixShaderChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kImageShader] = {
            "ImageShader",
            SkSpan(kImageShaderUniforms),
            SnippetRequirementFlags::kLocalCoords,
            SkSpan(kISTexturesAndSamplers, kNumImageShaderTexturesAndSamplers),
            kImageShaderName,
            GenerateImageShaderGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kBlendShader] = {
            "BlendShader",
            SkSpan(kBlendShaderUniforms),
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
            SkSpan(kShaderBasedBlenderUniforms),
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kBlendHelperName,
            GenerateShaderBasedBlenderGlueCode,
            kNoChildren,
            { }
    };
}
