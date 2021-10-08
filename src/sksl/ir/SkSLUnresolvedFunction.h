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
class UnresolvedFunction final : public Symbol {
public:
    inline static constexpr Kind kSymbolKind = Kind::kUnresolvedFunction;

    UnresolvedFunction(std::vector<const FunctionDeclaration*> funcs)
    : INHERITED(-1, kSymbolKind, funcs[0]->name())
    , fFunctions(std::move(funcs)) {
#ifdef SK_DEBUG
        SkASSERT(!this->functions().empty());
        for (auto func : this->functions()) {
            SkASSERT(func->name() == name());
        }
#endif
    }

    const std::vector<const FunctionDeclaration*>& functions() const {
        return fFunctions;
    }

    String description() const override {
        return String(this->name());
    }

private:
    std::vector<const FunctionDeclaration*> fFunctions;

    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
