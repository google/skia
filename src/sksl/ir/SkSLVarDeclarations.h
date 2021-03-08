/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {


namespace dsl {
    class DSLCore;
}

/**
 * A single variable declaration statement. Multiple variables declared together are expanded to
 * separate (sequential) statements. For instance, the SkSL 'int x = 2, y[3];' produces two
 * VarDeclaration instances (wrapped in an unscoped Block).
 */
class VarDeclaration final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kVarDeclaration;

    VarDeclaration(const Variable* var,
                   const Type* baseType,
                   int arraySize,
                   std::unique_ptr<Expression> value)
            : INHERITED(var->fOffset, kStatementKind)
            , fVar(var)
            , fBaseType(*baseType)
            , fArraySize(arraySize)
            , fValue(std::move(value)) {}

    const Type& baseType() const {
        return fBaseType;
    }

    const Variable& var() const {
        return *fVar;
    }

    void setVar(const Variable* var) {
        fVar = var;
    }

    int arraySize() const {
        return fArraySize;
    }

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<VarDeclaration>(&this->var(),
                                                &this->baseType(),
                                                fArraySize,
                                                this->value() ? this->value()->clone() : nullptr);
    }

    String description() const override {
        String result = this->var().modifiers().description() + this->baseType().description() +
                        " " + this->var().name();
        if (this->arraySize() > 0) {
            result.appendf("[%d]", this->arraySize());
        } else if (this->arraySize() == Type::kUnsizedArray){
            result += "[]";
        }
        if (this->value()) {
            result += " = " + this->value()->description();
        }
        result += ";";
        return result;
    }

private:
    const Variable* fVar;
    const Type& fBaseType;
    int fArraySize;  // zero means "not an array", Type::kUnsizedArray means var[]
    std::unique_ptr<Expression> fValue;

    friend class IRGenerator;

    using INHERITED = Statement;
};

/**
 * A variable declaration appearing at global scope. A global declaration like 'int x, y;' produces
 * two GlobalVarDeclaration elements, each containing the declaration of one variable.
 */
class GlobalVarDeclaration final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kGlobalVar;

    GlobalVarDeclaration(int offset, std::unique_ptr<Statement> decl)
            : INHERITED(offset, kProgramElementKind)
            , fDeclaration(std::move(decl)) {
        SkASSERT(this->declaration()->is<VarDeclaration>());
    }

    std::unique_ptr<Statement>& declaration() {
        return fDeclaration;
    }

    const std::unique_ptr<Statement>& declaration() const {
        return fDeclaration;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<GlobalVarDeclaration>(fOffset, this->declaration()->clone());
    }

    String description() const override {
        return this->declaration()->description();
    }

private:
    std::unique_ptr<Statement> fDeclaration;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
