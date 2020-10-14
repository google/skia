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
class VarDeclaration : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kVarDeclaration;

    VarDeclaration(const Variable* var,
                   const Type* baseType,
                   ExpressionArray sizes,
                   std::unique_ptr<Expression> value)
            : INHERITED(var->fOffset, VarDeclarationData{baseType, var}) {
        fExpressionChildren.reserve_back(sizes.size() + 1);
        fExpressionChildren.move_back_n(sizes.size(), sizes.data());
        fExpressionChildren.push_back(std::move(value));
    }

    const Type& baseType() const {
        return *this->varDeclarationData().fBaseType;
    }

    const Variable& var() const {
        return *this->varDeclarationData().fVar;
    }

    void setVar(const Variable* var) {
        this->varDeclarationData().fVar = var;
    }

    int sizeCount() const {
        return fExpressionChildren.size() - 1;
    }

    const std::unique_ptr<Expression>& size(int index) const {
        SkASSERT(index >= 0 && index < this->sizeCount());
        return fExpressionChildren[index];
    }

    std::unique_ptr<Expression>& value() {
        return fExpressionChildren.back();
    }

    const std::unique_ptr<Expression>& value() const {
        return fExpressionChildren.back();
    }

    std::unique_ptr<Statement> clone() const override {
        ExpressionArray sizesClone;
        sizesClone.reserve_back(this->sizeCount());
        for (int i = 0; i < this->sizeCount(); ++i) {
            if (this->size(i)) {
                sizesClone.push_back(this->size(i)->clone());
            } else {
                sizesClone.push_back(nullptr);
            }
        }
        return std::make_unique<VarDeclaration>(&this->var(),
                                                &this->baseType(),
                                                std::move(sizesClone),
                                                this->value() ? this->value()->clone() : nullptr);
    }

    String description() const override {
        String result = this->var().modifiers().description() + this->baseType().description() +
                        " " + this->var().name();
        for (int i = 0; i < this->sizeCount(); ++i) {
            if (this->size(i)) {
                result += "[" + this->size(i)->description() + "]";
            } else {
                result += "[]";
            }
        }
        if (this->value()) {
            result += " = " + this->value()->description();
        }
        result += ";";
        return result;
    }

    using INHERITED = Statement;
};

/**
 * A variable declaration appearing at global scope. A global declaration like 'int x, y;' produces
 * two GlobalVarDeclaration elements, each containing the declaration of one variable.
 */
class GlobalVarDeclaration : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kGlobalVar;

    GlobalVarDeclaration(int offset, std::unique_ptr<Statement> decl)
            : INHERITED(offset, kProgramElementKind) {
        SkASSERT(decl->is<VarDeclaration>());
        fStatementChildren.push_back(std::move(decl));
    }

    std::unique_ptr<Statement>& declaration() {
        return fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& declaration() const {
        return fStatementChildren[0];
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<GlobalVarDeclaration>(fOffset, this->declaration()->clone());
    }

    String description() const override {
        return this->declaration()->description();
    }

private:
    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
