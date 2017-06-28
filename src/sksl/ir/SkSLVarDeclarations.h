/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "SkSLExpression.h"
#include "SkSLProgramElement.h"
#include "SkSLStatement.h"
#include "SkSLVariable.h"

namespace SkSL {

/**
 * A single variable declaration within a var declaration statement. For instance, the statement
 * 'int x = 2, y[3];' is a VarDeclarations statement containing two individual VarDeclaration
 * instances.
 */
struct VarDeclaration : public Statement {
    VarDeclaration(const Variable* var,
                   std::vector<std::unique_ptr<Expression>> sizes,
                   std::unique_ptr<Expression> value)
    : INHERITED(var->fPosition, Statement::kVarDeclaration_Kind)
    , fVar(var)
    , fSizes(std::move(sizes))
    , fValue(std::move(value)) {}

    String description() const {
        String result = fVar->fName;
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

    const Variable* fVar;
    std::vector<std::unique_ptr<Expression>> fSizes;
    std::unique_ptr<Expression> fValue;

    typedef Statement INHERITED;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    VarDeclarations(Position position, const Type* baseType,
                    std::vector<std::unique_ptr<VarDeclaration>> vars)
    : INHERITED(position, kVar_Kind)
    , fBaseType(*baseType) {
        for (auto& var : vars) {
            fVars.push_back(std::unique_ptr<Statement>(var.release()));
        }
    }

    String description() const override {
        if (!fVars.size()) {
            return String();
        }
        String result = ((VarDeclaration&) *fVars[0]).fVar->fModifiers.description() +
                fBaseType.description() + " ";
        String separator;
        for (const auto& var : fVars) {
            result += separator;
            separator = ", ";
            result += var->description();
        }
        return result;
    }

    const Type& fBaseType;
    // this *should* be a vector of unique_ptr<VarDeclaration>, but it significantly simplifies the
    // CFG to only have to worry about unique_ptr<Statement>
    std::vector<std::unique_ptr<Statement>> fVars;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
