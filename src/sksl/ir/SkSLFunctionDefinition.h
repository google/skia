/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "SkSLBlock.h"
#include "SkSLFunctionDeclaration.h"
#include "SkSLProgramElement.h"

namespace SkSL {

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public ProgramElement {
    FunctionDefinition(Position position, const FunctionDeclaration& declaration,
                       std::unique_ptr<Block> body)
    : INHERITED(position, kFunction_Kind)
    , fDeclaration(declaration)
    , fBody(std::move(body)) {}

    String description() const override {
        return fDeclaration.description() + " " + fBody->description();
    }

    const FunctionDeclaration& fDeclaration;
    const std::unique_ptr<Block> fBody;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
