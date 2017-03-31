/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTINTERFACEBLOCK
#define SKSL_ASTINTERFACEBLOCK

#include "SkSLASTVarDeclaration.h"
#include "../ir/SkSLModifiers.h"

namespace SkSL {

/**
 * An interface block, as in:
 *
 * out gl_PerVertex {
 *   layout(builtin=0) vec4 gl_Position;
 *   layout(builtin=1) float gl_PointSize;
 * };
 */
struct ASTInterfaceBlock : public ASTDeclaration {
    // valueName is empty when it was not present in the source
    ASTInterfaceBlock(Position position,
                      Modifiers modifiers,
                      String typeName,
                      std::vector<std::unique_ptr<ASTVarDeclarations>> declarations,
                      String instanceName,
                      std::vector<std::unique_ptr<ASTExpression>> sizes)
    : INHERITED(position, kInterfaceBlock_Kind)
    , fModifiers(modifiers)
    , fTypeName(std::move(typeName))
    , fDeclarations(std::move(declarations))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes)) {}

    String description() const override {
        String result = fModifiers.description() + fTypeName + " {\n";
        for (size_t i = 0; i < fDeclarations.size(); i++) {
            result += fDeclarations[i]->description() + "\n";
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

    const Modifiers fModifiers;
    const String fTypeName;
    const std::vector<std::unique_ptr<ASTVarDeclarations>> fDeclarations;
    const String fInstanceName;
    const std::vector<std::unique_ptr<ASTExpression>> fSizes;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
