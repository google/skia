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
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkUniform.h"

// TODO: How to represent the type (e.g., 2D) of texture being sampled?
class SkTextureAndSampler {
public:
    constexpr SkTextureAndSampler(const char* name) : fName(name) {}

    const char* name() const { return fName; }

private:
    const char* fName;
};

struct SkShaderSnippet {
    using GenerateGlueCodeForEntry = std::string (*)(const std::string& resultName,
                                                     int entryIndex, // for uniform name mangling
                                                     const SkPaintParamsKey::BlockReader&,
                                                     const std::string& priorStageOutputName,
                                                     const std::vector<std::string>& childNames,
                                                     int indent);

    SkShaderSnippet() = default;

    SkShaderSnippet(SkSpan<const SkUniform> uniforms,
                    SkSpan<const SkTextureAndSampler> texturesAndSamplers,
                    const char* functionName,
                    const char* code,
                    GenerateGlueCodeForEntry glueCodeGenerator,
                    int numChildren,
                    SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations)
            : fUniforms(uniforms)
            , fTexturesAndSamplers(texturesAndSamplers)
            , fStaticFunctionName(functionName)
            , fStaticSkSL(code)
            , fGlueCodeGenerator(glueCodeGenerator)
            , fNumChildren(numChildren)
            , fDataPayloadExpectations(dataPayloadExpectations) {
    }

    std::string getMangledUniformName(int uniformIndex, int mangleId) const;

    SkSpan<const SkUniform> fUniforms;
    SkSpan<const SkTextureAndSampler> fTexturesAndSamplers;
    const char* fStaticFunctionName = nullptr;
    const char* fStaticSkSL = nullptr;
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
#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const SkPipelineDataGatherer::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const SkPipelineDataGatherer::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
    std::string toSkSL() const;
#endif

private:
    std::string emitGlueCodeForEntry(int* entryIndex,
                                     const std::string& priorStageOutputName,
                                     std::string* result,
                                     int indent) const;

    std::vector<SkPaintParamsKey::BlockReader> fBlockReaders;

#ifdef SK_GRAPHITE_ENABLED
    // The blendInfo doesn't actually contribute to the program's creation but, it contains the
    // matching fixed-function settings that the program's caller needs to set up.
    SkPipelineDataGatherer::BlendInfo fBlendInfo;
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
        const SkPipelineDataGatherer::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

    private:
        friend class SkShaderCodeDictionary;

#ifdef SK_GRAPHITE_ENABLED
        Entry(const SkPaintParamsKey& key, const SkPipelineDataGatherer::BlendInfo& blendInfo)
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
        SkPipelineDataGatherer::BlendInfo fBlendInfo;
#endif
    };

#ifdef SK_GRAPHITE_ENABLED
    const Entry* findOrCreate(const SkPaintParamsKey&,
                              const SkPipelineDataGatherer::BlendInfo&) SK_EXCLUDES(fSpinLock);
#else
    const Entry* findOrCreate(const SkPaintParamsKey&) SK_EXCLUDES(fSpinLock);
#endif

    const Entry* lookup(SkUniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const SkUniform> getUniforms(SkBuiltInCodeSnippetID) const;

    SkSpan<const SkPaintParamsKey::DataPayloadField> dataPayloadExpectations(int snippetID) const;

    // This method can return nullptr
    const SkShaderSnippet* getEntry(int codeSnippetID) const;
    const SkShaderSnippet* getEntry(SkBuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    void getShaderInfo(SkUniquePaintParamsID, SkShaderInfo*);

    int maxCodeSnippetID() const {
        return static_cast<int>(SkBuiltInCodeSnippetID::kLast) + fUserDefinedCodeSnippets.size();
    }

    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code
    // TODO: add hooks for user to actually provide code.
    int addUserDefinedSnippet(const char* name,
                              SkSpan<const SkPaintParamsKey::DataPayloadField> expectations);

private:
#ifdef SK_GRAPHITE_ENABLED
    Entry* makeEntry(const SkPaintParamsKey&, const SkPipelineDataGatherer::BlendInfo&);
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
