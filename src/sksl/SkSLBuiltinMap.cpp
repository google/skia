/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinMap.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"

#include <utility>

namespace SkSL {

void BuiltinMap::insertOrDie(std::string key, std::unique_ptr<ProgramElement> element) {
    SkASSERT(!fElements.find(key));
    fElements.set(std::move(key), std::move(element));
}

const ProgramElement* BuiltinMap::find(const std::string& key) const {
    if (std::unique_ptr<ProgramElement>* elem = fElements.find(key)) {
        return elem->get();
    }
    return fParent ? fParent->find(key) : nullptr;
}

void BuiltinMap::foreach (
        const std::function<void(const std::string&, const ProgramElement&)>& fn) const {
    fElements.foreach([&](const std::string& name, const std::unique_ptr<ProgramElement>& elem) {
        fn(name, *elem);
    });
    if (fParent) {
        fParent->foreach(fn);
    }
}

} // namespace SkSL
