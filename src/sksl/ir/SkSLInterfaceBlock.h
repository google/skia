/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include "SkSLProgramElement.h"
#include "SkSLSymbolTable.h"
#include "SkSLVarDeclarations.h"

namespace SkSL {

/**
 * An interface block, as in:
 *
 * out gl_PerVertex {
 *   layout(builtin=0) vec4 gl_Position;
 *   layout(builtin=1) float gl_PointSize;
 * };
 *
 * At the IR level, this is represented by a single variable of struct type.
 */
struct InterfaceBlock : public ProgramElement {
    InterfaceBlock(Position position, const Variable* var, String typeName, String instanceName,
                   std::vector<std::unique_ptr<Expression>> sizes,
                   std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(position, kInterfaceBlock_Kind)
    , fVariable(*var)
    , fTypeName(std::move(typeName))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes))
    , fTypeOwner(typeOwner) {}

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

    const Variable& fVariable;
    const String fTypeName;
    const String fInstanceName;
    const std::vector<std::unique_ptr<Expression>> fSizes;
    const std::shared_ptr<SymbolTable> fTypeOwner;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
