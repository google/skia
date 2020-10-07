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
 * A single variable declaration statement. Multiple variables declared together are expanded to
 * separate (sequential) statements. For instance, the SkSL 'int x = 2, y[3];' produces two
 * VarDeclaration instances (wrapped in an unscoped Block).
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
        String result = fVar->modifiers().description() + fBaseType.description() + " " +
                        fVar->name();
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
 * A variable declaration appearing at global scope. A global declaration like 'int x, y;' produces
 * two GlobalVarDeclaration elements, each containing the declaration of one variable.
 */
struct GlobalVarDeclaration : public ProgramElement {
    static constexpr Kind kProgramElementKind = Kind::kGlobalVar;

    // decl must be a unique_ptr<VarDeclaration>, but to simplify construction, we take a Statement
    GlobalVarDeclaration(int offset, std::unique_ptr<Statement> decl)
            : INHERITED(offset, kProgramElementKind) {
        SkASSERT(decl->is<VarDeclaration>());
        fDecl.reset(static_cast<VarDeclaration*>(decl.release()));
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<GlobalVarDeclaration>(fOffset, fDecl->clone());
    }

    String description() const override {
        return fDecl->description();
    }

    std::unique_ptr<VarDeclaration> fDecl;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
