/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_VARDECLARATION
#define SKSL_VARDECLARATION

#include "SkSLExpression.h"
#include "SkSLStatement.h"
#include "SkSLVariable.h"

namespace SkSL {

/**
 * A variable declaration, which may consist of multiple individual variables. For instance
 * 'int x, y = 1, z[4][2];' is a single VarDeclaration. This declaration would have a base type of 
 * 'int', names ['x', 'y', 'z'], sizes of [[], [], [4, 2]], and values of [null, 1, null].
 */
struct VarDeclaration : public ProgramElement {
    VarDeclaration(Position position, const Type* baseType, std::vector<const Variable*> vars,
                   std::vector<std::vector<std::unique_ptr<Expression>>> sizes,
                   std::vector<std::unique_ptr<Expression>> values)
    : INHERITED(position, kVar_Kind)
    , fBaseType(*baseType)
    , fVars(std::move(vars))
    , fSizes(std::move(sizes))
    , fValues(std::move(values)) {}

    std::string description() const override {
        std::string result = fVars[0]->fModifiers.description();
        const Type* baseType = &fVars[0]->fType;
        while (baseType->kind() == Type::kArray_Kind) {
            baseType = &baseType->componentType();
        }
        result += baseType->description();
        std::string separator = " ";
        for (size_t i = 0; i < fVars.size(); i++) {
            result += separator;
            separator = ", ";
            result += fVars[i]->fName;
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
        result += ";";
        return result;
    }

    const Type& fBaseType;
    const std::vector<const Variable*> fVars;
    const std::vector<std::vector<std::unique_ptr<Expression>>> fSizes;
    const std::vector<std::unique_ptr<Expression>> fValues;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
