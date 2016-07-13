/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_UNRESOLVEDFUNCTION
#define SKSL_UNRESOLVEDFUNCTION

#include "SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A symbol representing multiple functions with the same name.
 */
struct UnresolvedFunction : public Symbol {
    UnresolvedFunction(std::vector<std::shared_ptr<FunctionDeclaration>> funcs)
    : INHERITED(Position(), kUnresolvedFunction_Kind, funcs[0]->fName)
    , fFunctions(std::move(funcs)) {
    	for (auto func : funcs) {
    		ASSERT(func->fName == fName);
    	}
    }

    virtual std::string description() const override {
        return fName;
    }

    const std::vector<std::shared_ptr<FunctionDeclaration>> fFunctions;

    typedef Symbol INHERITED;
};

} // namespace

#endif
