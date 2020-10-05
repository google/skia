/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_UNRESOLVEDFUNCTION
#define SKSL_UNRESOLVEDFUNCTION

#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A symbol representing multiple functions with the same name.
 */
struct UnresolvedFunction : public Symbol {
    static constexpr Kind kSymbolKind = Kind::kUnresolvedFunction;

    UnresolvedFunction(std::vector<const FunctionDeclaration*> funcs)
    : INHERITED(-1, kSymbolKind, funcs[0]->name())
    , fFunctions(std::move(funcs)) {
#ifdef SK_DEBUG
        for (auto func : funcs) {
            SkASSERT(func->name() == name());
        }
#endif
    }

    String description() const override {
        return this->name();
    }

    const std::vector<const FunctionDeclaration*> fFunctions;

    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
