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
class ModifiersDeclaration : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kModifiers;

    ModifiersDeclaration(ModifiersPool::Handle modifiers)
    : INHERITED(-1, ModifiersDeclarationData{modifiers}) {}

    const Modifiers& modifiers() const {
        return *this->modifiersDeclarationData().fModifiersHandle;
    }

    const ModifiersPool::Handle& modifiersHandle() const {
        return this->modifiersDeclarationData().fModifiersHandle;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new ModifiersDeclaration(this->modifiersHandle()));
    }

    String description() const override {
        return this->modifiers().description() + ";";
    }

private:
    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
