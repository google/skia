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
class UnresolvedFunction : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kUnresolvedFunction;

    UnresolvedFunction(std::vector<const FunctionDeclaration*> funcs)
    : INHERITED(-1, UnresolvedFunctionData{std::move(funcs)}) {
#ifdef SK_DEBUG
        SkASSERT(!this->functions().empty());
        for (auto func : this->functions()) {
            SkASSERT(func->name() == name());
        }
#endif
    }

    StringFragment name() const override {
        return this->functions()[0]->name();
    }

    const std::vector<const FunctionDeclaration*>& functions() const {
        return this->unresolvedFunctionData().fFunctions;
    }

    String description() const override {
        return this->name();
    }

private:
    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
