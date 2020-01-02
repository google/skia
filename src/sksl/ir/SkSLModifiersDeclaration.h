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
    ModifiersDeclaration(Modifiers modifiers)
    : INHERITED(-1, kModifiers_Kind)
    , fModifiers(modifiers) {}

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new ModifiersDeclaration(fModifiers));
    }

#ifdef SK_DEBUG
    String description() const override {
        return fModifiers.description() + ";";
    }
#endif

    Modifiers fModifiers;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
