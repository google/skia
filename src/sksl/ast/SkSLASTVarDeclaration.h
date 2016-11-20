/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTVARDECLARATIONS
#define SKSL_ASTVARDECLARATIONS

#include "SkSLASTDeclaration.h"
#include "SkSLASTModifiers.h"
#include "SkSLASTStatement.h"
#include "SkSLASTType.h"
#include "../SkSLUtil.h"

namespace SkSL {

/**
 * A single variable declaration within a var declaration statement. For instance, the statement
 * 'int x = 2, y[3];' is an ASTVarDeclarations statement containing two individual ASTVarDeclaration 
 * instances.
 */
struct ASTVarDeclaration {
    ASTVarDeclaration(const std::string name,
                      std::vector<std::unique_ptr<ASTExpression>> sizes,
                      std::unique_ptr<ASTExpression> value)
    : fName(name)
    , fSizes(std::move(sizes))
    , fValue(std::move(value)) {}

    std::string description() const {
        std::string result = fName;
        for (const auto& size : fSizes) {
            if (size) {
                result += "[" + size->description() + "]";
            } else {
                result += "[]";
            }
        }
        if (fValue) {
            result += " = " + fValue->description();
        }
        return result;        
    }

    std::string fName;

    // array sizes, if any. e.g. 'foo[3][]' has sizes [3, null]
    std::vector<std::unique_ptr<ASTExpression>> fSizes;

    // initial value, may be null
    std::unique_ptr<ASTExpression> fValue;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct ASTVarDeclarations : public ASTDeclaration {
    ASTVarDeclarations(ASTModifiers modifiers, 
                       std::unique_ptr<ASTType> type, 
                       std::vector<ASTVarDeclaration> vars)
    : INHERITED(type->fPosition, kVar_Kind)
    , fModifiers(modifiers)
    , fType(std::move(type))
    , fVars(std::move(vars)) {}

    std::string description() const override {
        std::string result = fModifiers.description() + fType->description() + " ";
        std::string separator = "";
        for (const auto& var : fVars) {
            result += separator;
            separator = ", ";
            result += var.description();
        }
        return result;
    }

    const ASTModifiers fModifiers;
    const std::unique_ptr<ASTType> fType;
    const std::vector<ASTVarDeclaration> fVars;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
