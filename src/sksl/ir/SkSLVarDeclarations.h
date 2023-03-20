/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "include/core/SkTypes.h"
#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class Position;
class Type;

struct Modifiers;

/**
 * A single variable declaration statement. Multiple variables declared together are expanded to
 * separate (sequential) statements. For instance, the SkSL 'int x = 2, y[3];' produces two
 * VarDeclaration instances (wrapped in an unscoped Block).
 */
class VarDeclaration final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kVarDeclaration;

    VarDeclaration(Variable* var,
                   const Type* baseType,
                   int arraySize,
                   std::unique_ptr<Expression> value,
                   bool isClone = false)
            : INHERITED(var->fPosition, kIRNodeKind)
            , fVar(var)
            , fBaseType(*baseType)
            , fArraySize(arraySize)
            , fValue(std::move(value))
            , fIsClone(isClone) {}

    ~VarDeclaration() override {
        // Unhook this VarDeclaration from its associated Variable, since we're being deleted.
        if (fVar && !fIsClone) {
            fVar->detachDeadVarDeclaration();
        }
    }

    // Checks the modifiers, baseType, and storage for compatibility with one another and reports
    // errors if needed. This method is implicitly called during Convert(), but is also explicitly
    // called while processing interface block fields.
    static void ErrorCheck(const Context& context, Position pos, Position modifiersPosition,
            const Modifiers& modifiers, const Type* type, Variable::Storage storage);

    // Does proper error checking and type coercion; reports errors via ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context, std::unique_ptr<Variable> var,
            std::unique_ptr<Expression> value, bool addToSymbolTable = true);

    // Reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           Variable* var,
                                           const Type* baseType,
                                           int arraySize,
                                           std::unique_ptr<Expression> value);
    const Type& baseType() const {
        return fBaseType;
    }

    Variable* var() const {
        return fVar;
    }

    void detachDeadVariable() {
        fVar = nullptr;
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

    std::unique_ptr<Statement> clone() const override;

    std::string description() const override;

private:
    static bool ErrorCheckAndCoerce(const Context& context,
                                    const Variable& var,
                                    std::unique_ptr<Expression>& value);

    Variable* fVar;
    const Type& fBaseType;
    int fArraySize;  // zero means "not an array"
    std::unique_ptr<Expression> fValue;
    // if this VarDeclaration is a clone, it doesn't actually own the associated variable
    bool fIsClone;

    using INHERITED = Statement;
};

/**
 * A variable declaration appearing at global scope. A global declaration like 'int x, y;' produces
 * two GlobalVarDeclaration elements, each containing the declaration of one variable.
 */
class GlobalVarDeclaration final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kGlobalVar;

    GlobalVarDeclaration(std::unique_ptr<Statement> decl)
            : INHERITED(decl->fPosition, kIRNodeKind)
            , fDeclaration(std::move(decl)) {
        SkASSERT(this->declaration()->is<VarDeclaration>());
        this->varDeclaration().var()->setGlobalVarDeclaration(this);
    }

    std::unique_ptr<Statement>& declaration() {
        return fDeclaration;
    }

    const std::unique_ptr<Statement>& declaration() const {
        return fDeclaration;
    }

    VarDeclaration& varDeclaration() {
        return fDeclaration->as<VarDeclaration>();
    }

    const VarDeclaration& varDeclaration() const {
        return fDeclaration->as<VarDeclaration>();
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<GlobalVarDeclaration>(this->declaration()->clone());
    }

    std::string description() const override {
        return this->declaration()->description();
    }

private:
    std::unique_ptr<Statement> fDeclaration;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
