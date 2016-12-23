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
    ASTFunction(Position position, sk_up<ASTType> returnType, SkString name,
                std::vector<sk_up<ASTParameter>> parameters, sk_up<ASTBlock> body)
            : INHERITED(position, kFunction_Kind)
            , fReturnType(std::move(returnType))
            , fName(std::move(name))
            , fParameters(std::move(parameters))
            , fBody(std::move(body)) {}

    SkString description() const override {
        SkString result = fReturnType->description() + " " + fName + "(";
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

    const sk_up<ASTType> fReturnType;
    const SkString fName;
    const std::vector<sk_up<ASTParameter>> fParameters;
    const sk_up<ASTBlock> fBody;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
