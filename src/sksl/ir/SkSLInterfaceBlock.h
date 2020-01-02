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
    InterfaceBlock(int offset, const Variable* var, String typeName, String instanceName,
                   std::vector<std::unique_ptr<Expression>> sizes,
                   std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(offset, kInterfaceBlock_Kind)
    , fVariable(*var)
    , fTypeName(std::move(typeName))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes))
    , fTypeOwner(typeOwner) {}

    std::unique_ptr<ProgramElement> clone() const override {
        std::vector<std::unique_ptr<Expression>> sizesClone;
        for (const auto& s : fSizes) {
            sizesClone.push_back(s->clone());
        }
        return std::unique_ptr<ProgramElement>(new InterfaceBlock(fOffset, &fVariable, fTypeName,
                                                                  fInstanceName,
                                                                  std::move(sizesClone),
                                                                  fTypeOwner));
    }

#ifdef SK_DEBUG
    String description() const override {
        String result = fVariable.fModifiers.description() + fTypeName + " {\n";
        const Type* structType = &fVariable.fType;
        while (structType->kind() == Type::kArray_Kind) {
            structType = &structType->componentType();
        }
        for (const auto& f : structType->fields()) {
            result += f.description() + "\n";
        }
        result += "}";
        if (fInstanceName.size()) {
            result += " " + fInstanceName;
            for (const auto& size : fSizes) {
                result += "[";
                if (size) {
                    result += size->description();
                }
                result += "]";
            }
        }
        return result + ";";
    }
#endif

    const Variable& fVariable;
    const String fTypeName;
    const String fInstanceName;
    std::vector<std::unique_ptr<Expression>> fSizes;
    const std::shared_ptr<SymbolTable> fTypeOwner;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
