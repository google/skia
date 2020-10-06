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
    static constexpr Kind kStatementKind = Kind::kVarDeclaration;

    VarDeclaration(const Variable* var,
                   const Type* baseType,
                   std::vector<std::unique_ptr<Expression>> sizes,
                   std::unique_ptr<Expression> value)
            : INHERITED(var->fOffset, kStatementKind)
            , fVar(var)
            , fBaseType(*baseType)
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
        return std::make_unique<VarDeclaration>(fVar, &fBaseType, std::move(sizesClone),
                                                fValue ? fValue->clone() : nullptr);
    }

    String description() const override {
        String result =
                fVar->fModifiers.description() + fBaseType.description() + " " + fVar->name();
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
        result += ";";
        return result;
    }

    const Variable* fVar;
    const Type& fBaseType;
    std::vector<std::unique_ptr<Expression>> fSizes;
    std::unique_ptr<Expression> fValue;

    using INHERITED = Statement;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    static constexpr Kind kProgramElementKind = Kind::kVar;

    // var must be a unique_ptr<VarDeclaration>, but to simplify the CFG, we store
    // (and thus are constructed with) a Statement.
    VarDeclarations(int offset, std::unique_ptr<Statement> var)
            : INHERITED(offset, kProgramElementKind)
            , fVar(std::move(var)) {
        SkASSERT(fVar->is<VarDeclaration>());
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<VarDeclarations>(fOffset, fVar->clone());
    }

    String description() const override {
        return fVar->isEmpty() ? String() : fVar->description();
    }

    // this *should* be a unique_ptr<VarDeclaration>, but it significantly simplifies the
    // CFG to only have to worry about unique_ptr<Statement>
    std::unique_ptr<Statement> fVar;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
