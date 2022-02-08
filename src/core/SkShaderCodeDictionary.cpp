/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkShaderCodeDictionary.h"

#include "src/core/SkOpts.h"

// TODO: SkShaderInfo::toSkSL needs to work outside of both just graphite and metal. To do
// so we'll need to switch over to using SkSL's uniform capabilities.
#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)

#include "include/private/SkSLString.h"

// TODO: switch this over to using SkSL's uniform system
namespace skgpu::mtl {
std::string GetMtlUniforms(int bufferID,
                           const char* name,
                           const std::vector<SkShaderInfo::SnippetEntry>&);
}

// The current, incomplete, model for shader construction is:
//   each code snippet defines a method with a unique name that takes 0 params and returns a half4.
//   For each code snippet in the shader info, the matching method will be called and the result
//            placed in a variable named "outColor%d". The integer is just that snippet's position
//            in the shaderInfo.
//   Currently, the last shader snippet's "outColor%d" is then copied into "sk_FragColor".
// Note: that each entry's 'fName' field must match the name of the method in the 'fCode' field.
std::string SkShaderInfo::toSkSL() const {
    std::string result = skgpu::mtl::GetMtlUniforms(2, "FS", fEntries);

    for (auto c : fEntries) {
        result += c.fCode;
    }

    result += "layout(location = 0, index = 0) out half4 sk_FragColor;\n";
    result += "void main() {\n";

    for (size_t i = 0; i < fEntries.size(); ++i) {
        SkSL::String::appendf(&result, "half4 outColor%d = %s();\n", (int) i, fEntries[i].fName);
    }
    SkSL::String::appendf(&result, "sk_FragColor = outColor%d;\n", (int) fEntries.size()-1);
    result += "}\n";

    return result;
}
#endif

SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::makeEntry(const SkPaintParamsKey& key) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(key); });
}

size_t SkShaderCodeDictionary::Hash::operator()(const SkPaintParamsKey& key) const {
    return SkOpts::hash_fn(key.data(), key.sizeInBytes(), 0);
}

const SkShaderCodeDictionary::Entry* SkShaderCodeDictionary::findOrCreate(
        const SkPaintParamsKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    auto iter = fHash.find(key);
    if (iter != fHash.end()) {
        SkASSERT(fEntryVector[iter->second->uniqueID().asUInt()] == iter->second);
        return iter->second;
    }

    Entry* newEntry = this->makeEntry(key);
    newEntry->setUniqueID(fEntryVector.size());
    fHash.insert(std::make_pair(newEntry->paintParamsKey(), newEntry));
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

SkSpan<const SkUniform> SkShaderCodeDictionary::getUniforms(CodeSnippetID id) const {
    return fCodeSnippets[(int) id].fUniforms;
}

const SkShaderInfo::SnippetEntry* SkShaderCodeDictionary::getEntry(CodeSnippetID id) const {
    if (fCodeSnippets[(int) id].fCode) {
        return &fCodeSnippets[(int) id];
    }

    // If we're missing a code snippet just draw solid blue
    return this->getEntry(CodeSnippetID::kDepthStencilOnlyDraw);
}

void SkShaderCodeDictionary::getShaderInfo(SkUniquePaintParamsID uniqueID, SkShaderInfo* info) {
    auto entry = this->lookup(uniqueID);

    entry->paintParamsKey().toShaderInfo(this, info);
}
//--------------------------------------------------------------------------------------------------
namespace {

static constexpr int kFourStopGradient = 4;

// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 6;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] = {
        { "colors",  SkSLType::kHalf4, kFourStopGradient },
        { "offsets", SkSLType::kFloat, kFourStopGradient },
        { "point0",  SkSLType::kFloat2 },
        { "point1",  SkSLType::kFloat2 },
        { "radius0", SkSLType::kFloat },
        { "radius1", SkSLType::kFloat },
};

static const char *kLinearGradient4Name = "linear_grad_4_shader";
static const char *kLinearGradient4SkSL =
        // TODO: This should use local coords
        "half4 linear_grad_4_shader() {\n"
        "    float2 pos = sk_FragCoord.xy;\n"
        "    float2 delta = point1 - point0;\n"
        "    float2 pt = pos - point0;\n"
        "    float t = dot(pt, delta) / dot(delta, delta);\n"
        "    float4 result = colors[0];\n"
        "    result = mix(result, colors[1],\n"
        "                 clamp((t-offsets[0])/(offsets[1]-offsets[0]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colors[2],\n"
        "                 clamp((t-offsets[1])/(offsets[2]-offsets[1]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colors[3],\n"
        "                 clamp((t-offsets[2])/(offsets[3]-offsets[2]),\n"
        "                 0, 1));\n"
        "    return half4(result);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumSolidUniforms = 1;
static constexpr SkUniform kSolidUniforms[kNumSolidUniforms] = {
        { "color", SkSLType::kFloat4 }
};

static const char* kSolidColorName = "solid_shader";
static const char* kSolidColorSkSL =
        "half4 solid_shader() {\n"
        "    return half4(color);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumImageUniforms = 0;

static const char* kImageName = "image_shader";
static const char* kImageSkSL =
        "half4 image_shader() {\n"
        "    float r = fract(abs(sk_FragCoord.x/10.0));\n"
        "    return half4(r, 0.0, 0.0, 1.0);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
// TODO: kNone is for depth-only draws, so should actually have a fragment output type
// that only defines a [[depth]] attribute but no color calculation.
static const char* kNoneName = "none";
static const char* kNoneSkSL =
        "half4 none() {\n"
        "    return half4(0.0, 0.0, 1.0, 1.0);\n"
        "}\n";

} // anonymous namespace

SkShaderCodeDictionary::SkShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);

    fCodeSnippets[(int) CodeSnippetID::kDepthStencilOnlyDraw] = {
            {}, kNoneName, kNoneSkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kSolidColorShader] = {
            SkMakeSpan(kSolidUniforms, kNumSolidUniforms),
            kSolidColorName, kSolidColorSkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kLinearGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kRadialGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kSweepGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kConicalGradientShader] = {
            SkMakeSpan(kGradientUniforms, kNumGradientUniforms),
            kLinearGradient4Name, kLinearGradient4SkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kImageShader] = {
            { nullptr, kNumImageUniforms },
            kImageName, kImageSkSL
    };
    fCodeSnippets[(int) CodeSnippetID::kBlendShader] = {
            {}, nullptr, nullptr
    };
    fCodeSnippets[(int) CodeSnippetID::kSimpleBlendMode] = {
            {}, nullptr, nullptr
    };
}
