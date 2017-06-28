/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTFUNCTION
#define SKSL_ASTFUNCTION

#include "SkSLASTBlock.h"
#include "SkSLASTDeclaration.h"
#include "SkSLASTParameter.h"
#include "SkSLASTType.h"

namespace SkSL {

/**
 * A function declaration or definition. The fBody field will be null for declarations.
 */
struct ASTFunction : public ASTDeclaration {
    ASTFunction(Position position, Modifiers modifiers,  std::unique_ptr<ASTType> returnType,
                String name, std::vector<std::unique_ptr<ASTParameter>> parameters,
                std::unique_ptr<ASTBlock> body)
    : INHERITED(position, kFunction_Kind)
    , fModifiers(modifiers)
    , fReturnType(std::move(returnType))
    , fName(std::move(name))
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
    const String fName;
    const std::vector<std::unique_ptr<ASTParameter>> fParameters;
    const std::unique_ptr<ASTBlock> fBody;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
