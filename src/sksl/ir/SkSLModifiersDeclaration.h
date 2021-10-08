/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERDECLARATION
#define SKSL_MODIFIERDECLARATION

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"

namespace SkSL {

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
class ModifiersDeclaration final : public ProgramElement {
public:
    inline static constexpr Kind kProgramElementKind = Kind::kModifiers;

    ModifiersDeclaration(const Modifiers* modifiers)
        : INHERITED(-1, kProgramElementKind)
        , fModifiers(modifiers) {}

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<ModifiersDeclaration>(&this->modifiers());
    }

    String description() const override {
        return this->modifiers().description() + ";";
    }

private:
    const Modifiers* fModifiers;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
