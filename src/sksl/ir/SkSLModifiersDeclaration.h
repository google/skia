/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERDECLARATION
#define SKSL_MODIFIERDECLARATION

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLProgramElement.h"

#include <memory>
#include <string>

namespace SkSL {

class Context;

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
class ModifiersDeclaration final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kModifiers;

    ModifiersDeclaration(Position pos, const Modifiers* modifiers)
            : INHERITED(pos, kIRNodeKind)
            , fModifiers(modifiers) {}

    static std::unique_ptr<ModifiersDeclaration> Convert(const Context& context,
                                                         Position pos,
                                                         const Modifiers& modifiers);

    static std::unique_ptr<ModifiersDeclaration> Make(const Context& context,
                                                      Position pos,
                                                      const Modifiers& modifiers);

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<ModifiersDeclaration>(fPosition, fModifiers);
    }

    std::string description() const override {
        return this->modifiers().description() + ";";
    }

private:
    const Modifiers* fModifiers;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
