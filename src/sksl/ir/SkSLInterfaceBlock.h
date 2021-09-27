/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include <memory>

#include "include/core/SkStringView.h"
#include "include/private/SkSLProgramElement.h"
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

    InterfaceBlock(int line, const Variable& var, skstd::string_view typeName,
                   skstd::string_view instanceName, int arraySize,
                   std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(line, kProgramElementKind)
    , fVariable(var)
    , fTypeName(typeName)
    , fInstanceName(instanceName)
    , fArraySize(arraySize)
    , fTypeOwner(std::move(typeOwner)) {}

    const Variable& variable() const {
        return fVariable;
    }

    skstd::string_view typeName() const {
        return fTypeName;
    }

    skstd::string_view instanceName() const {
        return fInstanceName;
    }

    const std::shared_ptr<SymbolTable>& typeOwner() const {
        return fTypeOwner;
    }

    int arraySize() const {
        return fArraySize;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<InterfaceBlock>(fLine, this->variable(), this->typeName(),
                                                this->instanceName(), this->arraySize(),
                                                SymbolTable::WrapIfBuiltin(this->typeOwner()));
    }

    String description() const override {
        String result = this->variable().modifiers().description() + this->typeName() + " {\n";
        const Type* structType = &this->variable().type();
        if (structType->isArray()) {
            structType = &structType->componentType();
        }
        for (const auto& f : structType->fields()) {
            result += f.description() + "\n";
        }
        result += "}";
        if (!this->instanceName().empty()) {
            result += " " + this->instanceName();
            if (this->arraySize() > 0) {
                result.appendf("[%d]", this->arraySize());
            }
        }
        return result + ";";
    }

private:
    const Variable& fVariable;
    skstd::string_view fTypeName;
    skstd::string_view fInstanceName;
    int fArraySize;
    std::shared_ptr<SymbolTable> fTypeOwner;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
