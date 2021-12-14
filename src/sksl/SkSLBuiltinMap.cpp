/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLString.h"
#include "src/sksl/SkSLBuiltinMap.h"

namespace SkSL {

void BuiltinMap::insertOrDie(String key, std::unique_ptr<ProgramElement> element) {
    SkASSERT(fElements.find(key) == fElements.end());
    fElements[key] = BuiltinElement{std::move(element), false};
}

const ProgramElement* BuiltinMap::find(const String& key) {
    auto iter = fElements.find(key);
    if (iter == fElements.end()) {
        return fParent ? fParent->find(key) : nullptr;
    }
    return iter->second.fElement.get();
}

// Only returns a builtin element that isn't already marked as included, and then marks it.
const ProgramElement* BuiltinMap::findAndInclude(const String& key) {
    auto iter = fElements.find(key);
    if (iter == fElements.end()) {
        return fParent ? fParent->findAndInclude(key) : nullptr;
    }
    if (iter->second.fAlreadyIncluded) {
        return nullptr;
    }
    iter->second.fAlreadyIncluded = true;
    return iter->second.fElement.get();
}

void BuiltinMap::resetAlreadyIncluded() {
    for (auto& pair : fElements) {
        pair.second.fAlreadyIncluded = false;
    }
    if (fParent) {
        fParent->resetAlreadyIncluded();
    }
}

} // namespace SkSL
