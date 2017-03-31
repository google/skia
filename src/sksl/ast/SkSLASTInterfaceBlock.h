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
                      SkString typeName,
                      std::vector<std::unique_ptr<ASTVarDeclarations>> declarations,
                      SkString instanceName,
                      std::vector<std::unique_ptr<ASTExpression>> sizes)
    : INHERITED(position, kInterfaceBlock_Kind)
    , fModifiers(modifiers)
    , fTypeName(std::move(typeName))
    , fDeclarations(std::move(declarations))
    , fInstanceName(std::move(instanceName))
    , fSizes(std::move(sizes)) {}

    SkString description() const override {
        SkString result = fModifiers.description() + fTypeName + " {\n";
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
    const SkString fTypeName;
    const std::vector<std::unique_ptr<ASTVarDeclarations>> fDeclarations;
    const SkString fInstanceName;
    const std::vector<std::unique_ptr<ASTExpression>> fSizes;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
