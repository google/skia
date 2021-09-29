/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLString.h"
#include "src/sksl/SkSLIntrinsicMap.h"

namespace SkSL {

void IntrinsicMap::insertOrDie(String key, std::unique_ptr<ProgramElement> element) {
    SkASSERT(fIntrinsics.find(key) == fIntrinsics.end());
    fIntrinsics[key] = Intrinsic{std::move(element), false};
}

const ProgramElement* IntrinsicMap::find(const String& key) {
    auto iter = fIntrinsics.find(key);
    if (iter == fIntrinsics.end()) {
        return fParent ? fParent->find(key) : nullptr;
    }
    return iter->second.fIntrinsic.get();
}

// Only returns an intrinsic that isn't already marked as included, and then marks it.
const ProgramElement* IntrinsicMap::findAndInclude(const String& key) {
    auto iter = fIntrinsics.find(key);
    if (iter == fIntrinsics.end()) {
        return fParent ? fParent->findAndInclude(key) : nullptr;
    }
    if (iter->second.fAlreadyIncluded) {
        return nullptr;
    }
    iter->second.fAlreadyIncluded = true;
    return iter->second.fIntrinsic.get();
}

void IntrinsicMap::resetAlreadyIncluded() {
    for (auto& pair : fIntrinsics) {
        pair.second.fAlreadyIncluded = false;
    }
    if (fParent) {
        fParent->resetAlreadyIncluded();
    }
}

} // namespace SkSL
