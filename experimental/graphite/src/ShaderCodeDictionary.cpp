/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ShaderCodeDictionary.h"

namespace skgpu {

ShaderCodeDictionary::ShaderCodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

ShaderCodeDictionary::Entry* ShaderCodeDictionary::makeEntry(Combination combo) {
    return fArena.make([&](void *ptr) { return new(ptr) Entry(combo); });
}

size_t ShaderCodeDictionary::Hash::operator()(Combination combo) const {
    return combo.key();
}

const ShaderCodeDictionary::Entry* ShaderCodeDictionary::findOrCreate(Combination combo) {
    SkAutoSpinlock lock{fSpinLock};

    auto iter = fHash.find(combo);
    if (iter != fHash.end()) {
        SkASSERT(fEntryVector[iter->second->uniqueID().asUInt()] == iter->second);
        return iter->second;
    }

    Entry* newEntry = this->makeEntry(combo);
    newEntry->setUniqueID(fEntryVector.size());
    fHash.insert(std::make_pair(newEntry->combo(), newEntry));
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
