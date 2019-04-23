/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTFUNCTION
#define SKSL_ASTFUNCTION

#include "src/sksl/ast/SkSLASTBlock.h"
#include "src/sksl/ast/SkSLASTDeclaration.h"
#include "src/sksl/ast/SkSLASTParameter.h"
#include "src/sksl/ast/SkSLASTType.h"

namespace SkSL {

/**
 * A function declaration or definition. The fBody field will be null for declarations.
 */
struct ASTFunction : public ASTDeclaration {
    ASTFunction(int offset, Modifiers modifiers,  std::unique_ptr<ASTType> returnType,
                StringFragment name, std::vector<std::unique_ptr<ASTParameter>> parameters,
                std::unique_ptr<ASTBlock> body)
    : INHERITED(offset, kFunction_Kind)
    , fModifiers(modifiers)
    , fReturnType(std::move(returnType))
    , fName(name)
    , fParameters(std::move(parameters))
    , fBody(std::move(body)) {}

    String description() const override {
        String result = fReturnType->description() + " " + fName + "(";
        for (size_t i = 0; i < fParameters.size(); i++) {
            if (i > 0) {
                result += ", ";
            }
            result += fParameters[i]->description();
        }
        if (fBody) {
            result += ") " + fBody->description();
        } else {
            result += ");";
        }
        return result;
    }

    const Modifiers fModifiers;
    const std::unique_ptr<ASTType> fReturnType;
    const StringFragment fName;
    const std::vector<std::unique_ptr<ASTParameter>> fParameters;
    const std::unique_ptr<ASTBlock> fBody;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
