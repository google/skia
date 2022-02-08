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
#include "src/core/SkUniform.h"

class SkShaderInfo {
public:
    struct SnippetEntry {
        SkSpan<const SkUniform> fUniforms;
        const char* fName;
        const char* fCode;
    };

    void add(const SnippetEntry& entry) {
        fEntries.push_back(entry);
    }

    // TODO: writing to color should be a property of the SnippetEntries and accumulated as the
    //  entries are added. _Not_ set manually via 'setWritesColor'.
    void setWritesColor() { fWritesColor = true; }
    bool writesColor() const { return fWritesColor; }

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
    std::string toSkSL() const;
#endif

private:
    std::vector<SnippetEntry> fEntries;
    bool fWritesColor = false;
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
        const SkPaintParamsKey& paintParamsKey() const { return fPaintParamsKey; }

    private:
        friend class SkShaderCodeDictionary;

        Entry(const SkPaintParamsKey& paintParamsKey) : fPaintParamsKey(paintParamsKey) {}

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = SkUniquePaintParamsID(newID);
        }

        SkUniquePaintParamsID fUniqueID;  // fixed-size (uint32_t) unique ID assigned to a key
        SkPaintParamsKey fPaintParamsKey; // variable-length paint key descriptor
    };

    const Entry* findOrCreate(const SkPaintParamsKey&) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(SkUniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const SkUniform> getUniforms(SkBuiltInCodeSnippetID) const;
    const SkShaderInfo::SnippetEntry* getEntry(SkBuiltInCodeSnippetID) const;

    void getShaderInfo(SkUniquePaintParamsID, SkShaderInfo*);

private:
    Entry* makeEntry(const SkPaintParamsKey&);

    struct Hash {
        size_t operator()(const SkPaintParamsKey&) const;
    };

    std::array<SkShaderInfo::SnippetEntry, kBuiltInCodeSnippetIDCount> fCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    std::unordered_map<SkPaintParamsKey, Entry*, Hash> fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    SkArenaAlloc fArena{256};
};

#endif // SkShaderCodeDictionary_DEFINED
