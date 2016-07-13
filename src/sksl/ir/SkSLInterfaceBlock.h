/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include "SkSLProgramElement.h"
#include "SkSLVarDeclaration.h"

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
    InterfaceBlock(Position position, std::shared_ptr<Variable> var)
    : INHERITED(position, kInterfaceBlock_Kind) 
    , fVariable(std::move(var)) {
        ASSERT(fVariable->fType->kind() == Type::kStruct_Kind);
    }

    std::string description() const override {
        std::string result = fVariable->fModifiers.description() + fVariable->fName + " {\n";
        for (size_t i = 0; i < fVariable->fType->fields().size(); i++) {
            result += fVariable->fType->fields()[i].description() + "\n";
        }
        result += "};";
        return result;
    }

    const std::shared_ptr<Variable> fVariable;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
