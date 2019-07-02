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
    InterfaceBlock(IRGenerator* irGenerator, int offset, IRNode::ID var, String typeName,
                   String instanceName, std::vector<IRNode::ID> sizes,
                   std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(irGenerator, offset, kInterfaceBlock_Kind)
    , fVariable(var)
    , fTypeName(std::move(typeName))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes))
    , fTypeOwner(typeOwner) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new InterfaceBlock(fIRGenerator, fOffset, fVariable,
                                                           fTypeName, fInstanceName, fSizes,
                                                           fTypeOwner));
    }

    String description() const override {
        Variable& var = (Variable&) fVariable.node();
        String result = var.fModifiers.description() + fTypeName + " {\n";
        const Type* structType = &var.fType.typeNode();
        while (structType->kind() == Type::kArray_Kind) {
            structType = &structType->componentType().typeNode();
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
                    result += size.node().description();
                }
                result += "]";
            }
        }
        return result + ";";
    }

    IRNode::ID fVariable;
    const String fTypeName;
    const String fInstanceName;
    std::vector<IRNode::ID> fSizes;
    const std::shared_ptr<SymbolTable> fTypeOwner;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
