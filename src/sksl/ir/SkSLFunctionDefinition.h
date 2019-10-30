/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public ProgramElement {
    FunctionDefinition(int offset, const FunctionDeclaration& declaration,
                       std::unique_ptr<Statement> body)
    : INHERITED(offset, kFunction_Kind)
    , fDeclaration(declaration)
    , fBody(std::move(body)) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new FunctionDefinition(fOffset, fDeclaration,
                                                                      fBody->clone()));
    }

    String description() const override {
        return fDeclaration.description() + " " + fBody->description();
    }

    const FunctionDeclaration& fDeclaration;
    std::unique_ptr<Statement> fBody;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
