/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

/**
 * An interface block, as in:
 *
 * out sk_PerVertex {
 *   layout(builtin=0) float4 sk_Position;
 *   layout(builtin=1) float sk_PointSize;
 * };
 *
 * At the IR level, this is represented by a single variable of struct type.
 */
struct InterfaceBlock : public ProgramElement {
    static constexpr Kind kProgramElementKind = Kind::kInterfaceBlock;

    InterfaceBlock(int offset, const Variable* var, String typeName, String instanceName,
                   ExpressionArray sizes, std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(offset, InterfaceBlockData{var, std::move(typeName), std::move(instanceName),
                                           std::move(typeOwner)}) {
        fExpressionChildren.move_back_n(sizes.size(), sizes.data());
    }

    const Variable& variable() const {
        return *this->interfaceBlockData().fVariable;
    }

    void setVariable(const Variable* var) {
        this->interfaceBlockData().fVariable = var;
    }

    const String& typeName() const {
        return this->interfaceBlockData().fTypeName;
    }

    const String& instanceName() const {
        return this->interfaceBlockData().fInstanceName;
    }

    const std::shared_ptr<SymbolTable>& typeOwner() const {
        return this->interfaceBlockData().fTypeOwner;
    }

    ExpressionArray& sizes() {
        return fExpressionChildren;
    }

    const ExpressionArray& sizes() const {
        return fExpressionChildren;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        ExpressionArray sizesClone;
        sizesClone.reserve_back(this->sizes().size());
        for (const auto& size : this->sizes()) {
            sizesClone.push_back(size ? size->clone() : nullptr);
        }
        return std::make_unique<InterfaceBlock>(fOffset, &this->variable(), this->typeName(),
                                                this->instanceName(), std::move(sizesClone),
                                                this->typeOwner());
    }

    String description() const override {
        String result = this->variable().modifiers().description() + this->typeName() + " {\n";
        const Type* structType = &this->variable().type();
        while (structType->typeKind() == Type::TypeKind::kArray) {
            structType = &structType->componentType();
        }
        for (const auto& f : structType->fields()) {
            result += f.description() + "\n";
        }
        result += "}";
        if (this->instanceName().size()) {
            result += " " + this->instanceName();
            for (const auto& size : this->sizes()) {
                result += "[";
                if (size) {
                    result += size->description();
                }
                result += "]";
            }
        }
        return result + ";";
    }

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
