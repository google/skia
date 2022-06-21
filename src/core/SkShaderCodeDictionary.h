/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderCodeDictionary_DEFINED
#define SkShaderCodeDictionary_DEFINED

#include <array>
#include <unordered_map>
#include <vector>
#include "include/core/SkSpan.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEnumBitMask.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkUniform.h"

namespace SkSL {
struct ShaderCaps;
}

class SkBlenderID;
class SkRuntimeEffect;

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
};
SK_MAKE_BITMASK_OPS(SnippetRequirementFlags);

struct SkShaderSnippet {
    using GenerateGlueCodeForEntry = void (*)(const std::string& resultName,
                                              int entryIndex,  // for uniform name mangling
                                              const SkPaintParamsKey::BlockReader&,
                                              const std::string& priorStageOutputName,
                                              const std::vector<std::string>& childNames,
                                              std::string* preamble,
                                              std::string* mainBody,
                                              int indent);

    SkShaderSnippet() = default;

    SkShaderSnippet(const char* name,
                    SkSpan<const SkUniform> uniforms,
                    SnippetRequirementFlags snippetRequirementFlags,
                    SkSpan<const SkTextureAndSampler> texturesAndSamplers,
                    const char* functionName,
                    GenerateGlueCodeForEntry glueCodeGenerator,
                    int numChildren,
                    SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations)
            : fName(name)
            , fUniforms(uniforms)
            , fSnippetRequirementFlags(snippetRequirementFlags)
            , fTexturesAndSamplers(texturesAndSamplers)
            , fStaticFunctionName(functionName)
            , fGlueCodeGenerator(glueCodeGenerator)
            , fNumChildren(numChildren)
            , fDataPayloadExpectations(dataPayloadExpectations) {}

    std::string getMangledUniformName(int uniformIndex, int mangleId) const;

    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }

    const char* fName = nullptr;
    SkSpan<const SkUniform> fUniforms;
    SnippetRequirementFlags fSnippetRequirementFlags;
    SkSpan<const SkTextureAndSampler> fTexturesAndSamplers;
    const char* fStaticFunctionName = nullptr;
    GenerateGlueCodeForEntry fGlueCodeGenerator = nullptr;
    int fNumChildren = 0;
    SkSpan<const SkPaintParamsKey::DataPayloadField> fDataPayloadExpectations;
};

// This is just a simple collection object that gathers together all the information needed
// for program creation and its invocation.
class SkShaderInfo {
public:
    void add(const SkPaintParamsKey::BlockReader& reader) {
        fBlockReaders.push_back(reader);
    }
    void addFlags(SnippetRequirementFlags flags) {
        fSnippetRequirementFlags |= flags;
    }
    bool needsLocalCoords() const {
        return fSnippetRequirementFlags & SnippetRequirementFlags::kLocalCoords;
    }

#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

#if (SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)) && defined(SK_METAL)
    std::string toSkSL() const;
#endif

private:
    std::string emitGlueCodeForEntry(int* entryIndex,
                                     const std::string& priorStageOutputName,
                                     const std::string& parentPreLocalName,
                                     std::string* preamble,
                                     std::string* mainBody,
                                     int indent) const;

    std::vector<SkPaintParamsKey::BlockReader> fBlockReaders;

    SkEnumBitMask<SnippetRequirementFlags> fSnippetRequirementFlags =SnippetRequirementFlags::kNone;
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
    const SkShaderSnippet* getEntry(SkBlenderID) const;

    void getShaderInfo(SkUniquePaintParamsID, SkShaderInfo*);

    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code
    // TODO: add hooks for user to actually provide code.
    int addUserDefinedSnippet(const char* name,
                              SkSpan<const SkPaintParamsKey::DataPayloadField> expectations);

    SkBlenderID addUserDefinedBlender(sk_sp<SkRuntimeEffect>);

private:
#ifdef SK_GRAPHITE_ENABLED
    Entry* makeEntry(const SkPaintParamsKey&, const skgpu::BlendInfo&);
#else
    Entry* makeEntry(const SkPaintParamsKey&);
#endif

    struct Hash {
        size_t operator()(const SkPaintParamsKey*) const;
    };

    struct KeyEqual {
        bool operator()(const SkPaintParamsKey* k1, const SkPaintParamsKey* k2) const {
            return k1->operator==(*k2);
        }
    };

    std::array<SkShaderSnippet, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    std::vector<std::unique_ptr<SkShaderSnippet>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    using PaintHashMap = std::unordered_map<const SkPaintParamsKey*, Entry*, Hash, KeyEqual>;

    PaintHashMap fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    // This arena holds:
    //    the Entries held in 'fHash' and 'fEntryVector' - thus, guarded by 'fSpinLock'
    SkArenaAlloc fArena{256};
};

#endif // SkShaderCodeDictionary_DEFINED
