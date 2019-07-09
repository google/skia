/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERDECLARATION
#define SKSL_MODIFIERDECLARATION

#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
struct ModifiersDeclaration : public ProgramElement {
    ModifiersDeclaration(IRGenerator* irGenerator, Modifiers modifiers)
    : INHERITED(irGenerator, -1, kModifiers_Kind)
    , fModifiers(modifiers) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ModifiersDeclaration(fIRGenerator, fModifiers));
    }

    String description() const override {
        return fModifiers.description() + ";";
    }

    Modifiers fModifiers;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
