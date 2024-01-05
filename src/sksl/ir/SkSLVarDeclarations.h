/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class Context;
struct Layout;
struct Modifiers;
class Position;
class Type;

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
                           const Layout& layout, ModifierFlags modifierFlags, const Type* type,
                           const Type* baseType, Variable::Storage storage);

    // For use when no Variable yet exists. The newly-created variable will be added to the active
    // symbol table. Performs proper error checking and type coercion; reports errors via
    // ErrorReporter.
    static std::unique_ptr<VarDeclaration> Convert(const Context& context,
                                                   Position overallPos,
                                                   const Modifiers& modifiers,
                                                   const Type& type,
                                                   Position namePos,
                                                   std::string_view name,
                                                   VariableStorage storage,
                                                   std::unique_ptr<Expression> value);

    // For use when a Variable already exists. The passed-in variable will be added to the active
    // symbol table. Performs proper error checking and type coercion; reports errors via
    // ErrorReporter.
    static std::unique_ptr<VarDeclaration> Convert(const Context& context,
                                                   std::unique_ptr<Variable> var,
                                                   std::unique_ptr<Expression> value);

    // The symbol table is left as-is. Reports errors via ASSERT.
    static std::unique_ptr<VarDeclaration> Make(const Context& context,
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

    std::string description() const override;

private:
    static bool ErrorCheckAndCoerce(const Context& context,
                                    const Variable& var,
                                    const Type* baseType,
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

    std::string description() const override {
        return this->declaration()->description();
    }

private:
    std::unique_ptr<Statement> fDeclaration;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
