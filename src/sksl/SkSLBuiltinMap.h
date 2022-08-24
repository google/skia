/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BUILTINMAP
#define SKSL_BUILTINMAP

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkTHash.h"

#include <functional>
#include <memory>
#include <string>

namespace SkSL {

/**
 * Represents the builtin elements in the Context.
 */
class BuiltinMap {
public:
    BuiltinMap(const BuiltinMap* parent) : fParent(parent) {}

    void insertOrDie(std::string key, std::unique_ptr<ProgramElement> element);

    const ProgramElement* find(const std::string& key) const;

    void foreach(const std::function<void(const std::string&, const ProgramElement&)>& fn) const;

private:
    SkTHashMap<std::string, std::unique_ptr<ProgramElement>> fElements;
    const BuiltinMap* fParent = nullptr;
};

} // namespace SkSL

#endif
