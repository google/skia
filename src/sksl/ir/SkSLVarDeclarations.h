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
    static constexpr Kind kStatementKind = kVarDeclaration_Kind;

    VarDeclaration(const Variable* var,
                   std::vector<std::unique_ptr<Expression>> sizes,
                   std::unique_ptr<Expression> value)
    : INHERITED(var->fOffset, kStatementKind)
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

    String description() const override {
        String result = fVar->fModifiers.description() + fVar->fType.name() + " " + fVar->fName;
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

    using INHERITED = Statement;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    static constexpr Kind kProgramElementKind = kVar_Kind;

    VarDeclarations(int offset, const Type* baseType,
                    std::vector<std::unique_ptr<VarDeclaration>> vars)
    : INHERITED(offset, kProgramElementKind)
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

    String description() const override {
        if (!fVars.size()) {
            return String();
        }
        String result;
        for (const auto& var : fVars) {
            if (var->fKind != Statement::kNop_Kind) {
                SkASSERT(var->fKind == Statement::kVarDeclaration_Kind);
                result = ((const VarDeclaration&) *var).fVar->fModifiers.description();
                break;
            }
        }
        result += fBaseType.description() + " ";
        String separator;
        for (const auto& rawVar : fVars) {
            if (rawVar->fKind == Statement::kNop_Kind) {
                continue;
            }
            SkASSERT(rawVar->fKind == Statement::kVarDeclaration_Kind);
            VarDeclaration& var = (VarDeclaration&) *rawVar;
            result += separator;
            separator = ", ";
            result += var.fVar->fName;
            if (var.fValue) {
                result += " = " + var.fValue->description();
            }
        }
        return result;
    }

    const Type& fBaseType;
    // this *should* be a vector of unique_ptr<VarDeclaration>, but it significantly simplifies the
    // CFG to only have to worry about unique_ptr<Statement>
    std::vector<std::unique_ptr<Statement>> fVars;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
