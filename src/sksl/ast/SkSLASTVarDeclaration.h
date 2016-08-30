/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTVARDECLARATION
#define SKSL_ASTVARDECLARATION

#include "SkSLASTDeclaration.h"
#include "SkSLASTModifiers.h"
#include "SkSLASTStatement.h"
#include "SkSLASTType.h"
#include "../SkSLUtil.h"

namespace SkSL {

/**
 * A variable declaration, which may consist of multiple individual variables. For instance
 * 'int x, y = 1, z[4][2]' is a single ASTVarDeclaration. This declaration would have a type of 
 * 'int', names ['x', 'y', 'z'], sizes of [[], [], [4, 2]], and values of [null, 1, null].
 */
struct ASTVarDeclaration : public ASTDeclaration {
    ASTVarDeclaration(ASTModifiers modifiers, 
                      std::unique_ptr<ASTType> type, 
                      std::vector<std::string> names, 
                      std::vector<std::vector<std::unique_ptr<ASTExpression>>> sizes,
                      std::vector<std::unique_ptr<ASTExpression>> values)
    : INHERITED(type->fPosition, kVar_Kind)
    , fModifiers(modifiers)
    , fType(std::move(type))
    , fNames(std::move(names))
    , fSizes(std::move(sizes))
    , fValues(std::move(values)) {
        ASSERT(fNames.size() == fValues.size());
    }

    std::string description() const override {
        std::string result = fModifiers.description() + fType->description() + " ";
        std::string separator = "";
        for (size_t i = 0; i < fNames.size(); i++) {
            result += separator;
            separator = ", ";
            result += fNames[i];
            for (size_t j = 0; j < fSizes[i].size(); j++) {
                if (fSizes[i][j]) {
                    result += "[" + fSizes[i][j]->description() + "]";
                } else {
                    result += "[]";
                }
            }
            if (fValues[i]) {
                result += " = " + fValues[i]->description();
            }
        }
        return result;        
    }

    const ASTModifiers fModifiers;
    const std::unique_ptr<ASTType> fType;
    const std::vector<std::string> fNames;
    const std::vector<std::vector<std::unique_ptr<ASTExpression>>> fSizes;
    const std::vector<std::unique_ptr<ASTExpression>> fValues;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
