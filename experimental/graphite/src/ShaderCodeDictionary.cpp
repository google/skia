/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ShaderCodeDictionary.h"
#include "src/core/SkOpts.h"

namespace skgpu {

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

ShaderCodeDictionary::Entry* ShaderCodeDictionary::makeEntry(const SkPaintParamsKey& key) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(key); });
}

size_t ShaderCodeDictionary::Hash::operator()(const SkPaintParamsKey& key) const {
    return SkOpts::hash_fn(key.data(), key.sizeInBytes(), 0);
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::findOrCreate(const SkPaintParamsKey& key) {
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

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::lookup(UniquePaintParamsID codeID) const {
    if (!codeID.isValid()) {
        return nullptr;
    }

    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

} // namespace skgpu
