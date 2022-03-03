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

void BuiltinMap::insertOrDie(std::string key, std::unique_ptr<ProgramElement> element) {
    SkASSERT(!fElements.find(key));
    fElements.set(std::move(key), BuiltinElement{std::move(element), /*fAlreadyIncluded=*/false});
}

const ProgramElement* BuiltinMap::find(const std::string& key) {
    BuiltinElement* elem = fElements.find(key);
    if (!elem) {
        return fParent ? fParent->find(key) : nullptr;
    }
    return elem->fElement.get();
}

// Only returns a builtin element that isn't already marked as included, and then marks it.
const ProgramElement* BuiltinMap::findAndInclude(const std::string& key) {
    BuiltinElement* elem = fElements.find(key);
    if (!elem) {
        return fParent ? fParent->findAndInclude(key) : nullptr;
    }
    if (elem->fAlreadyIncluded) {
        return nullptr;
    }
    elem->fAlreadyIncluded = true;
    return elem->fElement.get();
}

void BuiltinMap::resetAlreadyIncluded() {
    fElements.foreach([](const std::string&, BuiltinElement* elem) {
        elem->fAlreadyIncluded = false;
    });
    if (fParent) {
        fParent->resetAlreadyIncluded();
    }
}

} // namespace SkSL
