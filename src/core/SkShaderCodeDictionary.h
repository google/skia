/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderCodeDictionary_DEFINED
#define SkShaderCodeDictionary_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkTHash.h"
#include "include/private/SkThreadAnnotations.h"
#include "include/private/SkTo.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBuiltInCodeSnippetID.h"
#include "src/core/SkEnumBitMask.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkUniform.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#ifdef SK_GRAPHITE_ENABLED
namespace skgpu::graphite {
class RenderStep;
}
#endif

class SkRuntimeEffect;
class SkRuntimeEffectDictionary;
#ifdef SK_ENABLE_PRECOMPILE
class SkBlenderID;
#endif

// TODO: How to represent the type (e.g., 2D) of texture being sampled?
class SkTextureAndSampler {
public:
    constexpr SkTextureAndSampler(const char* name) : fName(name) {}

    const char* name() const { return fName; }

private:
    const char* fName;
};

enum class SnippetRequirementFlags : uint32_t {
    kNone = 0x0,
    kLocalCoords = 0x1,
    kPriorStageOutput = 0x2
};
SK_MAKE_BITMASK_OPS(SnippetRequirementFlags);

struct SkShaderSnippet {
    using GeneratePreambleForSnippetFn = void (*)(const SkShaderInfo& shaderInfo,
                                                  int* entryIndex,
                                                  const SkPaintParamsKey::BlockReader&,
                                                  std::string* preamble);
    using GenerateExpressionForSnippetFn = std::string (*)(const SkShaderInfo& shaderInfo,
                                                           int entryIndex,
                                                           const SkPaintParamsKey::BlockReader&,
                                                           const std::string& priorStageOutputName,
                                                           const std::string& fragCoord,
                                                           const std::string& currentPreLocalName);

    SkShaderSnippet() = default;

    SkShaderSnippet(const char* name,
                    SkSpan<const SkUniform> uniforms,
                    SnippetRequirementFlags snippetRequirementFlags,
                    SkSpan<const SkTextureAndSampler> texturesAndSamplers,
                    const char* functionName,
                    GenerateExpressionForSnippetFn expressionGenerator,
                    GeneratePreambleForSnippetFn preambleGenerator,
                    int numChildren,
                    SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations)
            : fName(name)
            , fUniforms(uniforms)
            , fSnippetRequirementFlags(snippetRequirementFlags)
            , fTexturesAndSamplers(texturesAndSamplers)
            , fStaticFunctionName(functionName)
            , fExpressionGenerator(expressionGenerator)
            , fPreambleGenerator(preambleGenerator)
            , fNumChildren(numChildren)
            , fDataPayloadExpectations(dataPayloadExpectations) {}

    std::string getMangledUniformName(int uniformIdx, int mangleId) const;
    std::string getMangledSamplerName(int samplerIdx, int mangleId) const;

    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }
    bool needsPriorStageOutput() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kPriorStageOutput;
    }

    const char* fName = nullptr;
    SkSpan<const SkUniform> fUniforms;
    SnippetRequirementFlags fSnippetRequirementFlags;
    SkSpan<const SkTextureAndSampler> fTexturesAndSamplers;
    const char* fStaticFunctionName = nullptr;
    GenerateExpressionForSnippetFn fExpressionGenerator = nullptr;
    GeneratePreambleForSnippetFn fPreambleGenerator = nullptr;
    int fNumChildren = 0;
    SkSpan<const SkPaintParamsKey::DataPayloadField> fDataPayloadExpectations;
};

// This is just a simple collection object that gathers together all the information needed
// for program creation and its invocation.
class SkShaderInfo {
public:
    SkShaderInfo(SkRuntimeEffectDictionary* rteDict = nullptr)
            : fRuntimeEffectDictionary(rteDict) {}
    ~SkShaderInfo() = default;
    SkShaderInfo(SkShaderInfo&&) = default;
    SkShaderInfo& operator=(SkShaderInfo&&) = default;
    SkShaderInfo(const SkShaderInfo&) = delete;
    SkShaderInfo& operator=(const SkShaderInfo&) = delete;

    void add(const SkPaintParamsKey::BlockReader& reader) {
        fBlockReaders.push_back(reader);
    }
    void addFlags(SnippetRequirementFlags flags) {
        fSnippetRequirementFlags |= flags;
    }
    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }
    const SkPaintParamsKey::BlockReader& blockReader(int index) const {
        return fBlockReaders[index];
    }
    const SkRuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRuntimeEffectDictionary;
    }

#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

#if defined(SK_GRAPHITE_ENABLED) && defined(SK_ENABLE_SKSL)
    std::string toSkSL(const skgpu::graphite::RenderStep* step,
                       const bool defineLocalCoordsVarying) const;
#endif

private:
    std::vector<SkPaintParamsKey::BlockReader> fBlockReaders;

    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags{SnippetRequirementFlags::kNone};
    SkRuntimeEffectDictionary* fRuntimeEffectDictionary = nullptr;

#ifdef SK_GRAPHITE_ENABLED
    // The blendInfo doesn't actually contribute to the program's creation but, it contains the
    // matching fixed-function settings that the program's caller needs to set up.
    skgpu::BlendInfo fBlendInfo;
#endif
};

class SkShaderCodeDictionary {
public:
    SkShaderCodeDictionary();

    struct Entry {
    public:
        SkUniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        const SkPaintParamsKey& paintParamsKey() const { return fKey; }
#ifdef SK_GRAPHITE_ENABLED
        const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

    private:
        friend class SkShaderCodeDictionary;

#ifdef SK_GRAPHITE_ENABLED
        Entry(const SkPaintParamsKey& key, const skgpu::BlendInfo& blendInfo)
                : fKey(key.asSpan())
                , fBlendInfo(blendInfo) {
        }
#else
        Entry(const SkPaintParamsKey& key) : fKey(key.asSpan()) {}
#endif

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = SkUniquePaintParamsID(newID);
        }

        SkUniquePaintParamsID fUniqueID;  // fixed-size (uint32_t) unique ID assigned to a key
        SkPaintParamsKey fKey; // variable-length paint key descriptor

#ifdef SK_GRAPHITE_ENABLED
        // The BlendInfo isn't used in the hash (that is the key's job) but it does directly vary
        // with the key. It could, theoretically, be recreated from the key but that would add
        // extra complexity.
        skgpu::BlendInfo fBlendInfo;
#endif
    };

    const Entry* findOrCreate(SkPaintParamsKeyBuilder*) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(SkUniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const SkUniform> getUniforms(SkBuiltInCodeSnippetID) const;
    SnippetRequirementFlags getSnippetRequirementFlags(SkBuiltInCodeSnippetID id) const {
        return fBuiltInCodeSnippets[(int) id].fSnippetRequirementFlags;
    }

    SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations(int snippetID) const;

    bool isValidID(int snippetID) const;

    // This method can return nullptr
    const SkShaderSnippet* getEntry(int codeSnippetID) const;
    const SkShaderSnippet* getEntry(SkBuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    void getShaderInfo(SkUniquePaintParamsID, SkShaderInfo*) const;

    int findOrCreateRuntimeEffectSnippet(const SkRuntimeEffect* effect);

    int addUserDefinedSnippet(const char* name,
                              SkSpan<const SkPaintParamsKey::DataPayloadField> expectations);

#ifdef SK_ENABLE_PRECOMPILE
    SkBlenderID addUserDefinedBlender(sk_sp<SkRuntimeEffect>);
    const SkShaderSnippet* getEntry(SkBlenderID) const;
#endif

private:
#ifdef SK_GRAPHITE_ENABLED
    Entry* makeEntry(const SkPaintParamsKey&, const skgpu::BlendInfo&);
#else
    Entry* makeEntry(const SkPaintParamsKey&);
#endif

    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code
    int addUserDefinedSnippet(
            const char* name,
            SkSpan<const SkUniform> uniforms,
            SnippetRequirementFlags snippetRequirementFlags,
            SkSpan<const SkTextureAndSampler> texturesAndSamplers,
            const char* functionName,
            SkShaderSnippet::GenerateExpressionForSnippetFn expressionGenerator,
            SkShaderSnippet::GeneratePreambleForSnippetFn preambleGenerator,
            int numChildren,
            SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations);

    const char* addTextToArena(std::string_view text);

    SkSpan<const SkUniform> convertUniforms(const SkRuntimeEffect* effect);

    std::array<SkShaderSnippet, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    std::vector<std::unique_ptr<SkShaderSnippet>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    struct SkPaintParamsKeyPtr {
        const SkPaintParamsKey* fKey;

        bool operator==(SkPaintParamsKeyPtr rhs) const {
            return *fKey == *rhs.fKey;
        }
        struct Hash {
            size_t operator()(SkPaintParamsKeyPtr) const;
        };
    };

    using PaintHashMap = SkTHashMap<SkPaintParamsKeyPtr, Entry*, SkPaintParamsKeyPtr::Hash>;

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
    //   - SkUniform data created by `findOrCreateRuntimeEffectSnippet`
    // and in all cases is guarded by `fSpinLock`
    SkArenaAlloc fArena{256};
};

#endif // SkShaderCodeDictionary_DEFINED
