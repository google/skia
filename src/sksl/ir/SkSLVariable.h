/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLE
#define SKSL_VARIABLE

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class Context;
class Expression;
class GlobalVarDeclaration;
class InterfaceBlock;
class Mangler;
class SymbolTable;
class VarDeclaration;

enum class VariableStorage : int8_t {
    kGlobal,
    kInterfaceBlock,
    kLocal,
    kParameter,
};

/**
 * Represents a variable, whether local, global, or a function parameter. This represents the
 * variable itself (the storage location), which is shared between all VariableReferences which
 * read or write that storage location.
 */
class Variable : public Symbol {
public:
    using Storage = VariableStorage;

    inline static constexpr Kind kIRNodeKind = Kind::kVariable;

    Variable(Position pos, Position modifiersPosition, ModifierFlags modifierFlags,
             std::string_view name, const Type* type, bool builtin, Storage storage)
            : INHERITED(pos, kIRNodeKind, name, type)
            , fModifierFlags(modifierFlags)
            , fModifiersPosition(modifiersPosition)
            , fStorage(storage)
            , fBuiltin(builtin) {}

    ~Variable() override;

    static std::unique_ptr<Variable> Convert(const Context& context, Position pos,
                                             Position modifiersPos, const Layout& layout,
                                             ModifierFlags flags, const Type* type,
                                             Position namePos, std::string_view name,
                                             Storage storage);

    static std::unique_ptr<Variable> Make(Position pos, Position modifiersPosition,
                                          const Layout& layout, ModifierFlags flags,
                                          const Type* type, std::string_view name,
                                          std::string mangledName, bool builtin, Storage storage);

    /**
     * Creates a local scratch variable and the associated VarDeclaration statement.
     * Useful when doing IR rewrites, e.g. inlining a function call.
     */
    struct ScratchVariable {
        const Variable* fVarSymbol;
        std::unique_ptr<Statement> fVarDecl;
    };
    static ScratchVariable MakeScratchVariable(const Context& context,
                                               Mangler& mangler,
                                               std::string_view baseName,
                                               const Type* type,
                                               SymbolTable* symbolTable,
                                               std::unique_ptr<Expression> initialValue);
    ModifierFlags modifierFlags() const {
        return fModifierFlags;
    }

    virtual const Layout& layout() const;

    Position modifiersPosition() const {
        return fModifiersPosition;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    Storage storage() const {
        return fStorage;
    }

    const Expression* initialValue() const;

    VarDeclaration* varDeclaration() const;

    void setVarDeclaration(VarDeclaration* declaration);

    GlobalVarDeclaration* globalVarDeclaration() const;

    void setGlobalVarDeclaration(GlobalVarDeclaration* global);

    void detachDeadVarDeclaration() {
        // The VarDeclaration is being deleted, so our reference to it has become stale.
        fDeclaringElement = nullptr;
    }

    // The interfaceBlock methods are no-op stubs here. They have proper implementations in
    // ExtendedVariable, declared below this class, which dedicates extra space to store the pointer
    // back to the InterfaceBlock.
    virtual InterfaceBlock* interfaceBlock() const { return nullptr; }

    virtual void setInterfaceBlock(InterfaceBlock*) { SkUNREACHABLE; }

    virtual void detachDeadInterfaceBlock() {}

    // Only ExtendedVariables support mangled names.
    virtual std::string_view mangledName() const { return this->name(); }

    std::string description() const override {
        return this->layout().paddedDescription() + this->modifierFlags().paddedDescription() +
               this->type().displayName() + " " + std::string(this->name());
    }

private:
    IRNode* fDeclaringElement = nullptr;
    ModifierFlags fModifierFlags;
    Position fModifiersPosition;
    VariableStorage fStorage;
    bool fBuiltin;

    using INHERITED = Symbol;
};

/**
 * ExtendedVariable is functionally equivalent to a regular Variable, but it also contains extra
 * fields that most variables don't need:
 * - The variable's associated InterfaceBlock
 * - The variable's layout
 * - The variable's mangled name
 *
 * Some of these fields can be null/empty.
 */
class ExtendedVariable final : public Variable {
public:
    ExtendedVariable(Position pos, Position modifiersPosition, const Layout& layout,
                     ModifierFlags flags, std::string_view name, const Type* type, bool builtin,
                     Storage storage, std::string mangledName)
            : INHERITED(pos, modifiersPosition, flags, name, type, builtin, storage)
            , fLayout(layout)
            , fMangledName(std::move(mangledName)) {}

    ~ExtendedVariable() override;

    InterfaceBlock* interfaceBlock() const override {
        return fInterfaceBlockElement;
    }

    const Layout& layout() const override {
        return fLayout;
    }

    void setInterfaceBlock(InterfaceBlock* elem) override {
        SkASSERT(!fInterfaceBlockElement);
        fInterfaceBlockElement = elem;
    }

    void detachDeadInterfaceBlock() override {
        // The InterfaceBlock is being deleted, so our reference to it has become stale.
        fInterfaceBlockElement = nullptr;
    }

    std::string_view mangledName() const override;

private:
    InterfaceBlock* fInterfaceBlockElement = nullptr;
    Layout fLayout;
    std::string fMangledName;

    using INHERITED = Variable;
};

} // namespace SkSL

#endif
