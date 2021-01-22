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
class InterfaceBlock final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kInterfaceBlock;

    InterfaceBlock(int offset, const Variable* var, String typeName, String instanceName,
                   ExpressionArray sizes, std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(offset, kProgramElementKind)
    , fVariable(var)
    , fTypeName(std::move(typeName))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes))
    , fTypeOwner(std::move(typeOwner)) {}

    const Variable& variable() const {
        return *fVariable;
    }

    void setVariable(const Variable* var) {
        fVariable = var;
    }

    const String& typeName() const {
        return fTypeName;
    }

    const String& instanceName() const {
        return fInstanceName;
    }

    const std::shared_ptr<SymbolTable>& typeOwner() const {
        return fTypeOwner;
    }

    ExpressionArray& sizes() {
        return fSizes;
    }

    const ExpressionArray& sizes() const {
        return fSizes;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        ExpressionArray sizesClone;
        sizesClone.reserve_back(this->sizes().size());
        for (const auto& size : this->sizes()) {
            sizesClone.push_back(size ? size->clone() : nullptr);
        }
        return std::make_unique<InterfaceBlock>(fOffset, &this->variable(), this->typeName(),
                                                this->instanceName(), std::move(sizesClone),
                                                SymbolTable::WrapIfBuiltin(this->typeOwner()));
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

private:
    const Variable* fVariable;
    String fTypeName;
    String fInstanceName;
    ExpressionArray fSizes;
    std::shared_ptr<SymbolTable> fTypeOwner;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
