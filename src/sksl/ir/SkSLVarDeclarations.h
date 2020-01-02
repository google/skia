/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVariable.h"

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
    : INHERITED(var->fOffset, Statement::kVarDeclaration_Kind)
    , fVar(var)
    , fSizes(std::move(sizes))
    , fValue(std::move(value)) {}

    std::unique_ptr<Statement> clone() const override {
        std::vector<std::unique_ptr<Expression>> sizesClone;
        for (const auto& s : fSizes) {
            if (s) {
                sizesClone.push_back(s->clone());
            } else {
                sizesClone.push_back(nullptr);
            }
        }
        return std::unique_ptr<Statement>(new VarDeclaration(fVar, std::move(sizesClone),
                                                             fValue ? fValue->clone() : nullptr));
    }

#ifdef SK_DEBUG
    String description() const override {
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
#endif

    const Variable* fVar;
    std::vector<std::unique_ptr<Expression>> fSizes;
    std::unique_ptr<Expression> fValue;

    typedef Statement INHERITED;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    VarDeclarations(int offset, const Type* baseType,
                    std::vector<std::unique_ptr<VarDeclaration>> vars)
    : INHERITED(offset, kVar_Kind)
    , fBaseType(*baseType) {
        for (auto& var : vars) {
            fVars.push_back(std::unique_ptr<Statement>(var.release()));
        }
    }

    std::unique_ptr<ProgramElement> clone() const override {
        std::vector<std::unique_ptr<VarDeclaration>> cloned;
        for (const auto& v : fVars) {
            cloned.push_back(std::unique_ptr<VarDeclaration>(
                                                           (VarDeclaration*) v->clone().release()));
        }
        return std::unique_ptr<ProgramElement>(new VarDeclarations(fOffset, &fBaseType,
                                                                     std::move(cloned)));
    }

#ifdef SK_DEBUG
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
#endif

    const Type& fBaseType;
    // this *should* be a vector of unique_ptr<VarDeclaration>, but it significantly simplifies the
    // CFG to only have to worry about unique_ptr<Statement>
    std::vector<std::unique_ptr<Statement>> fVars;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
