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
    UnresolvedFunction(IRGenerator* irGenerator, std::vector<IRNode::ID> funcs)
    : INHERITED(irGenerator, -1, kUnresolvedFunction_Kind,
                ((FunctionDeclaration&) funcs[0].node()).fName)
    , fFunctions(std::move(funcs)) {
#ifdef DEBUG
        for (auto func : funcs) {
            SkASSERT(func->fName == fName);
        }
#endif
    }

    String description() const override {
        return fName;
    }

    const std::vector<IRNode::ID> fFunctions;

    typedef Symbol INHERITED;
};

} // namespace

#endif
