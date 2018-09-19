/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTINTERFACEBLOCK
#define SKSL_ASTINTERFACEBLOCK

#include "src/sksl/ast/SkSLASTVarDeclaration.h"
#include "src/sksl/ir/SkSLModifiers.h"

namespace SkSL {

/**
 * An interface block, as in:
 *
 * out sk_PerVertex {
 *   layout(builtin=0) float4 sk_Position;
 *   layout(builtin=1) float sk_PointSize;
 * };
 */
struct ASTInterfaceBlock : public ASTDeclaration {
    // valueName is empty when it was not present in the source
    ASTInterfaceBlock(int offset,
                      Modifiers modifiers,
                      StringFragment typeName,
                      std::vector<std::unique_ptr<ASTVarDeclarations>> declarations,
                      StringFragment instanceName,
                      std::vector<std::unique_ptr<ASTExpression>> sizes)
    : INHERITED(offset, kInterfaceBlock_Kind)
    , fModifiers(modifiers)
    , fTypeName(typeName)
    , fDeclarations(std::move(declarations))
    , fInstanceName(instanceName)
    , fSizes(std::move(sizes)) {}

    String description() const override {
        String result = fModifiers.description() + fTypeName + " {\n";
        for (size_t i = 0; i < fDeclarations.size(); i++) {
            result += fDeclarations[i]->description() + "\n";
        }
        result += "}";
        if (fInstanceName.fLength) {
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

    const Modifiers fModifiers;
    const StringFragment fTypeName;
    const std::vector<std::unique_ptr<ASTVarDeclarations>> fDeclarations;
    const StringFragment fInstanceName;
    const std::vector<std::unique_ptr<ASTExpression>> fSizes;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
