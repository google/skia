/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BUILTINMAP
#define SKSL_BUILTINMAP

#include "include/core/SkSpan.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkTArray.h"

#include <functional>
#include <memory>

namespace SkSL {

class SymbolTable;

/** TODO(johnstiles): remove this class */
class BuiltinMap {
public:
    BuiltinMap(const BuiltinMap* parent,
               std::shared_ptr<SymbolTable> symbolTable,
               SkSpan<std::unique_ptr<ProgramElement>> elements);

    void foreach(const std::function<void(const ProgramElement&)>& fn) const;

    std::shared_ptr<SymbolTable> symbols() const { return fSymbolTable; }

private:
    const BuiltinMap* fParent = nullptr;
    std::shared_ptr<SymbolTable> fSymbolTable;
    SkTArray<std::unique_ptr<ProgramElement>> fUnmappedElements;
};

} // namespace SkSL

#endif
