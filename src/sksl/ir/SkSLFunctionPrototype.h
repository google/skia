/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONPROTOTYPE
#define SKSL_FUNCTIONPROTOTYPE

#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * A function prototype (a function declaration as a top-level program element)
 */
class FunctionPrototype final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFunctionPrototype;

    FunctionPrototype(Position pos, const FunctionDeclaration* declaration)
            : INHERITED(pos, kIRNodeKind)
            , fDeclaration(declaration) {}

    const FunctionDeclaration& declaration() const {
        return *fDeclaration;
    }

    std::string description() const override {
        return this->declaration().description() + ";";
    }

private:
    const FunctionDeclaration* fDeclaration;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
