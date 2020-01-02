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
    UnresolvedFunction(std::vector<const FunctionDeclaration*> funcs)
    : INHERITED(-1, kUnresolvedFunction_Kind, funcs[0]->fName)
    , fFunctions(std::move(funcs)) {
#ifdef DEBUG
        for (auto func : funcs) {
            SkASSERT(func->fName == fName);
        }
#endif
    }

#ifdef SK_DEBUG
    String description() const override {
        return fName;
    }
#endif

    const std::vector<const FunctionDeclaration*> fFunctions;

    typedef Symbol INHERITED;
};

} // namespace

#endif
