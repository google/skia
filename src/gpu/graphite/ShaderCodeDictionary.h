/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ShaderCodeDictionary_DEFINED
#define skgpu_graphite_ShaderCodeDictionary_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSpinlock.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class SkRuntimeEffect;

namespace skgpu::graphite {

class RenderStep;
class RuntimeEffectDictionary;

struct ResourceBindingRequirements;

// TODO: How to represent the type (e.g., 2D) of texture being sampled?
class TextureAndSampler {
public:
    constexpr TextureAndSampler(const char* name) : fName(name) {}

    const char* name() const { return fName; }

private:
    const char* fName;
};

enum class SnippetRequirementFlags : uint32_t {
    kNone = 0x0,
    kLocalCoords = 0x1,
    kPriorStageOutput = 0x2,  // AKA the "input" color, or the "source" color for a blender
    kDestColor = 0x4,
};
SK_MAKE_BITMASK_OPS(SnippetRequirementFlags);

struct ShaderSnippet {
    using GeneratePreambleForSnippetFn = void (*)(const ShaderInfo& shaderInfo,
                                                  int* entryIndex,
                                                  const PaintParamsKey::BlockReader&,
                                                  std::string* preamble);

    struct Args {
        std::string_view fPriorStageOutput;
        std::string_view fDestColor;
        std::string_view fFragCoord;
    };
    using GenerateExpressionForSnippetFn = std::string (*)(const ShaderInfo& shaderInfo,
                                                           int entryIndex,
                                                           const PaintParamsKey::BlockReader&,
                                                           const Args& args);

    ShaderSnippet() = default;

    ShaderSnippet(const char* name,
                  SkSpan<const Uniform> uniforms,
                  SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
                  SkSpan<const TextureAndSampler> texturesAndSamplers,
                  const char* functionName,
                  GenerateExpressionForSnippetFn expressionGenerator,
                  GeneratePreambleForSnippetFn preambleGenerator,
                  int numChildren,
                  SkSpan<const PaintParamsKey::DataPayloadField> dataPayloadExpectations)
        : fName(name)
        , fUniforms(uniforms)
        , fSnippetRequirementFlags(snippetRequirementFlags)
        , fTexturesAndSamplers(texturesAndSamplers)
        , fStaticFunctionName(functionName)
        , fExpressionGenerator(expressionGenerator)
        , fPreambleGenerator(preambleGenerator)
        , fNumChildren(numChildren)
        , fDataPayloadExpectations(dataPayloadExpectations) {}

    std::string getMangledUniformName(const ShaderInfo& shaderInfo,
                                      int uniformIdx,
                                      int mangleId) const;
    std::string getMangledSamplerName(int samplerIdx, int mangleId) const;

    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }
    bool needsPriorStageOutput() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kPriorStageOutput;
    }
    bool needsDestColor() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kDestColor;
    }

    const char* fName = nullptr;
    SkSpan<const Uniform> fUniforms;
    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags{SnippetRequirementFlags::kNone};
    SkSpan<const TextureAndSampler> fTexturesAndSamplers;
    const char* fStaticFunctionName = nullptr;
    GenerateExpressionForSnippetFn fExpressionGenerator = nullptr;
    GeneratePreambleForSnippetFn fPreambleGenerator = nullptr;
    int fNumChildren = 0;
    SkSpan<const PaintParamsKey::DataPayloadField> fDataPayloadExpectations;
};

// This is just a simple collection object that gathers together all the information needed
// for program creation and its invocation.
class ShaderInfo {
public:
    ShaderInfo(const RuntimeEffectDictionary* rteDict = nullptr,
               const char* ssboIndex = nullptr)
            : fRuntimeEffectDictionary(rteDict)
            , fSsboIndex(ssboIndex) {}
    ~ShaderInfo() = default;
    ShaderInfo(ShaderInfo&&) = default;
    ShaderInfo& operator=(ShaderInfo&&) = default;
    ShaderInfo(const ShaderInfo&) = delete;
    ShaderInfo& operator=(const ShaderInfo&) = delete;

    void add(const PaintParamsKey::BlockReader& reader) {
        fBlockReaders.push_back(reader);
    }
    void addFlags(SkEnumBitMask<SnippetRequirementFlags> flags) {
        fSnippetRequirementFlags |= flags;
    }
    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }
    const PaintParamsKey::BlockReader& blockReader(int index) const {
        return fBlockReaders[index];
    }
    const RuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRuntimeEffectDictionary;
    }
    const char* ssboIndex() const { return fSsboIndex; }

    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    std::string toSkSL(const ResourceBindingRequirements& bindingReqs,
                       const RenderStep* step,
                       const bool useStorageBuffers,
                       const bool defineLocalCoordsVarying,
                       int* numTexturesAndSamplersUsed) const;

private:
    std::vector<PaintParamsKey::BlockReader> fBlockReaders;

    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags{SnippetRequirementFlags::kNone};
    const RuntimeEffectDictionary* fRuntimeEffectDictionary = nullptr;

    const char* fSsboIndex;

    // The blendInfo doesn't actually contribute to the program's creation but, it contains the
    // matching fixed-function settings that the program's caller needs to set up.
    skgpu::BlendInfo fBlendInfo;
};

class ShaderCodeDictionary {
public:
    ShaderCodeDictionary();

    struct Entry {
    public:
        UniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        const PaintParamsKey& paintParamsKey() const { return fKey; }
        const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    private:
        friend class ShaderCodeDictionary;

        Entry(const PaintParamsKey& key, const skgpu::BlendInfo& blendInfo)
                : fKey(key.asSpan())
                , fBlendInfo(blendInfo) {
        }

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = UniquePaintParamsID(newID);
        }

        UniquePaintParamsID fUniqueID;  // fixed-size (uint32_t) unique ID assigned to a key
        PaintParamsKey fKey; // variable-length paint key descriptor

        // The BlendInfo isn't used in the hash (that is the key's job) but it does directly vary
        // with the key. It could, theoretically, be recreated from the key but that would add
        // extra complexity.
        skgpu::BlendInfo fBlendInfo;
    };

    const Entry* findOrCreate(PaintParamsKeyBuilder*) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(UniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const Uniform> getUniforms(BuiltInCodeSnippetID) const;
    SkEnumBitMask<SnippetRequirementFlags> getSnippetRequirementFlags(
            BuiltInCodeSnippetID id) const {
        return fBuiltInCodeSnippets[(int) id].fSnippetRequirementFlags;
    }

    SkSpan<const PaintParamsKey::DataPayloadField> dataPayloadExpectations(int snippetID) const;

    bool isValidID(int snippetID) const;

    // This method can return nullptr
    const ShaderSnippet* getEntry(int codeSnippetID) const;
    const ShaderSnippet* getEntry(BuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    void getShaderInfo(UniquePaintParamsID, ShaderInfo*) const;

    int findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect);

    int addUserDefinedSnippet(const char* name,
                              SkSpan<const PaintParamsKey::DataPayloadField> expectations);

private:
    Entry* makeEntry(const PaintParamsKey&, const skgpu::BlendInfo&);

    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code
    int addUserDefinedSnippet(
        const char* name,
        SkSpan<const Uniform> uniforms,
        SkEnumBitMask<SnippetRequirementFlags> snippetRequirementFlags,
        SkSpan<const TextureAndSampler> texturesAndSamplers,
        const char* functionName,
        ShaderSnippet::GenerateExpressionForSnippetFn expressionGenerator,
        ShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
        int numChildren,
        SkSpan<const PaintParamsKey::DataPayloadField> dataPayloadExpectations);

    const char* addTextToArena(std::string_view text);

    SkSpan<const Uniform> convertUniforms(const SkRuntimeEffect* effect);

    std::array<ShaderSnippet, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    std::vector<std::unique_ptr<ShaderSnippet>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    struct PaintParamsKeyPtr {
        const PaintParamsKey* fKey;

        bool operator==(PaintParamsKeyPtr rhs) const {
            return *fKey == *rhs.fKey;
        }
        struct Hash {
            size_t operator()(PaintParamsKeyPtr) const;
        };
    };

    using PaintHashMap = SkTHashMap<PaintParamsKeyPtr, Entry*, PaintParamsKeyPtr::Hash>;

    PaintHashMap fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    SK_BEGIN_REQUIRE_DENSE
    struct RuntimeEffectKey {
        uint32_t fHash;
        uint32_t fUniformSize;

        bool operator==(RuntimeEffectKey rhs) const {
            return fHash == rhs.fHash && fUniformSize == rhs.fUniformSize;
        }
        struct Hash {
            size_t operator()(RuntimeEffectKey) const;
        };
    };
    SK_END_REQUIRE_DENSE

    // A map from RuntimeEffectKeys (hash plus uniforms) to code-snippet IDs. RuntimeEffectKeys
    // don't track the lifetime of a runtime effect at all; they live forever, and a newly-
    // instantiated runtime effect with the same program as a previously-discarded effect will reuse
    // an existing ID. Entries in the runtime-effect map are never removed; they only disappear when
    // the context is discarded, which takes the ShaderCodeDictionary along with it. However, they
    // are extremely small (< 20 bytes) so the memory footprint should be unnoticeable.
    using RuntimeEffectMap = SkTHashMap<RuntimeEffectKey, int32_t>;
    RuntimeEffectMap fRuntimeEffectMap SK_GUARDED_BY(fSpinLock);

    // This arena holds:
    //   - the Entries held in `fHash` and `fEntryVector`
    //   - Uniform data created by `findOrCreateRuntimeEffectSnippet`
    // and in all cases is guarded by `fSpinLock`
    SkArenaAlloc fArena{256};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ShaderCodeDictionary_DEFINED
