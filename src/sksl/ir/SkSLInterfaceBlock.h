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
    InterfaceBlock(Position position, const Variable& var, std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(position, kInterfaceBlock_Kind) 
    , fVariable(std::move(var))
    , fTypeOwner(typeOwner) {
        ASSERT(fVariable.fType.kind() == Type::kStruct_Kind);
    }

    SkString description() const override {
        SkString result = fVariable.fModifiers.description() + fVariable.fName + " {\n";
        for (size_t i = 0; i < fVariable.fType.fields().size(); i++) {
            result += fVariable.fType.fields()[i].description() + "\n";
        }
        result += "};";
        return result;
    }

    const Variable& fVariable;
    const std::shared_ptr<SymbolTable> fTypeOwner;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
