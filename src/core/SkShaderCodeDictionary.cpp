/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkShaderCodeDictionary.h"

#include "include/private/SkSLString.h"
#include "src/core/SkOpts.h"

namespace {

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
std::string get_mangled_local_var_name(const char* baseName, int manglingSuffix) {
    return std::string(baseName) + "_" + std::to_string(manglingSuffix);
}
#endif

void add_indent(std::string* result, int indent) {
    result->append(4*indent, ' ');
}

} // anonymous namespace


std::string SkShaderSnippet::getMangledUniformName(int uniformIndex, int mangleId) const {
    std::string result;
    result = fUniforms[uniformIndex].name() + std::string("_") + std::to_string(mangleId);
    return result;
}

// TODO: SkShaderInfo::toSkSL needs to work outside of both just graphite and metal. To do
// so we'll need to switch over to using SkSL's uniform capabilities.
#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

#include <set>

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

    // Although the children appear after the parent in the shader info they are emitted
    // before the parent
    std::vector<std::string> childOutputVarNames;
    for (int j = 0; j < reader.numChildren(); ++j) {
        *entryIndex += 1;
        std::string childOutputVar = this->emitGlueCodeForEntry(entryIndex, priorStageOutputName,
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
//   each static code snippet (which can have an arbitrary signature) gets emitted once as a
//            preamble
//   glue code is then generated w/in the "main" method. The glue code is responsible for:
//            1) gathering the correct (mangled) uniforms
//            2) passing the uniforms and any other parameters to the helper method
//   The result of the last code snippet is then copied into "sk_FragColor".
// Note: that each entry's 'fStaticFunctionName' field must match the name of a function in the
// Graphite pre-compiled module, located at `src/sksl/sksl_graphite_frag.sksl`.
std::string SkShaderInfo::toSkSL() const {
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    std::string result = skgpu::graphite::GetMtlUniforms(2, "FS", fBlockReaders,
                                                         this->needsLocalCoords());

    int binding = 0;
    result += skgpu::graphite::GetMtlTexturesAndSamplers(fBlockReaders, &binding);
    result += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
    result += "void main() {\n";

    std::string lastOutputVar = "initialColor";

    // TODO: what is the correct initial color to feed in?
    add_indent(&result, 1);
    SkSL::String::appendf(&result, "    half4 %s = half4(0.0);", lastOutputVar.c_str());

    for (int entryIndex = 0; entryIndex < (int) fBlockReaders.size(); ++entryIndex) {
        lastOutputVar = this->emitGlueCodeForEntry(&entryIndex, lastOutputVar, &result, 1);
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
    SkASSERT(codeSnippetID >= 0 && codeSnippetID <= this->maxCodeSnippetID());

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

using DataPayloadField = SkPaintParamsKey::DataPayloadField;

// The default glue code just calls a helper function with the signature:
//    half4 fStaticFunctionName(/* all uniforms as parameters */);
// and stores the result in a variable named "resultName".
std::string GenerateDefaultGlueCode(const std::string& resultName,
                                    int entryIndex,
                                    const SkPaintParamsKey::BlockReader& reader,
                                    const std::string& priorStageOutputName,
                                    const std::vector<std::string>& childNames,
                                    int indent) {
    SkASSERT(childNames.empty());

    const SkShaderSnippet* entry = reader.entry();
    if (entry->needsLocalCoords()) {
        // Every snippet that requests local coordinates must have a localMatrix as its first
        // uniform
        SkASSERT(reader.entry()->fUniforms.size() >= 1);
        SkASSERT(reader.entry()->fUniforms[0].type() == SkSLType::kFloat4x4);
    }

    std::string result;

    add_indent(&result, indent);
    SkSL::String::appendf(&result,
                          "%s = %s(", resultName.c_str(),
                          entry->fStaticFunctionName);
    for (size_t i = 0; i < entry->fUniforms.size(); ++i) {
        if (i == 0 && reader.entry()->needsLocalCoords()) {
            result += entry->getMangledUniformName(i, entryIndex);
            result += " * dev2LocalUni";
        } else {
            result += entry->getMangledUniformName(i, entryIndex);
        }
        if (i+1 < entry->fUniforms.size()) {
            result += ", ";
        }
    }
    result += ");\n";

    return result;
}

//--------------------------------------------------------------------------------------------------
static constexpr int kFourStopGradient = 4;

// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 8;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] = {
        { "localMatrix", SkSLType::kFloat4x4 },
        { "colors",      SkSLType::kFloat4, kFourStopGradient },
        { "offsets",     SkSLType::kFloat,  kFourStopGradient },
        { "point0",      SkSLType::kFloat2 },
        { "point1",      SkSLType::kFloat2 },
        { "radius0",     SkSLType::kFloat },
        { "radius1",     SkSLType::kFloat },
        { "padding",     SkSLType::kFloat2 } // TODO: add automatic uniform padding
};

static constexpr char kLinearGradient4Name[] = "sk_linear_grad_4_shader";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumSolidShaderUniforms = 1;
static constexpr SkUniform kSolidShaderUniforms[kNumSolidShaderUniforms] = {
        { "color", SkSLType::kFloat4 }
};

static constexpr char kSolidShaderName[] = "sk_solid_shader";

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

    std::string localMatrixName = reader.entry()->getMangledUniformName(0, entryIndex);
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
                          localMatrixName.c_str(),
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

static constexpr char kBlendHelperName[] = "sk_blend";

// This exists as custom glue code to handle passing the result of the children into the
// static function. If this were handled in the default glue code, we could remove this.
std::string GenerateBlendShaderGlueCode(const std::string& resultName,
                                        int entryIndex,
                                        const SkPaintParamsKey::BlockReader& reader,
                                        const std::string& priorStageOutputName,
                                        const std::vector<std::string>& childNames,
                                        int indent) {
    SkASSERT(childNames.size() == kNumBlendShaderChildren);
    SkASSERT(reader.entry()->fUniforms.size() == 4); // actual blend uniform + 3 padding int

    std::string blendModeUniformName = reader.entry()->getMangledUniformName(0, entryIndex);

    std::string result;

    add_indent(&result, indent);
    SkSL::String::appendf(&result, "%s = %s(%s, %s, %s);\n",
                          resultName.c_str(),
                          reader.entry()->fStaticFunctionName,
                          blendModeUniformName.c_str(),
                          childNames[1].c_str(),
                          childNames[0].c_str());

    return result;
}

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

static constexpr int kNoChildren = 0;

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

    return kBuiltInCodeSnippetIDCount + fUserDefinedCodeSnippets.size() - 1;
}

SkShaderCodeDictionary::SkShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw] = {
            "DepthStencil",
            { },     // no uniforms
            SnippetRequirementFlags::kNone,
            { },     // no samplers
            kErrorName,
            GenerateDefaultGlueCode,
            kNoChildren,
            {}
    };
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
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kLinearGradientShader] = {
            "LinearGradient4",
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kRadialGradientShader] = {
            "RadialGradient4",
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kSweepGradientShader] = {
            "SweepGradient4",
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
            { }
    };
    fBuiltInCodeSnippets[(int) SkBuiltInCodeSnippetID::kConicalGradientShader] = {
            "ConicalGradient4",
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            SnippetRequirementFlags::kLocalCoords,
            { },     // no samplers
            kLinearGradient4Name,
            GenerateDefaultGlueCode,
            kNoChildren,
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
            kBlendHelperName,
            GenerateBlendShaderGlueCode,
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
