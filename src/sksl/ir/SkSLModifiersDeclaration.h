/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERDECLARATION
#define SKSL_MODIFIERDECLARATION

#include "SkSLProgramElement.h"
#include "SkSLModifiers.h"

namespace SkSL {

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
struct ModifiersDeclaration : public ProgramElement {
    ModifiersDeclaration(Modifiers modifiers)
    : INHERITED(Position(), kModifiers_Kind)
    , fModifiers(modifiers) {}

    String description() const {
        return fModifiers.description() + ";";
    }

    Modifiers fModifiers;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
