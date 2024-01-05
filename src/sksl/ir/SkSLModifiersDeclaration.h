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
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"

#include <memory>
#include <string>

namespace SkSL {

class Context;
struct Modifiers;

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
class ModifiersDeclaration final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kModifiers;

    ModifiersDeclaration(Position pos, const Layout& layout, ModifierFlags flags)
            : INHERITED(pos, kIRNodeKind)
            , fLayout(layout)
            , fFlags(flags) {}

    static std::unique_ptr<ModifiersDeclaration> Convert(const Context& context,
                                                         const Modifiers& modifiers);

    static std::unique_ptr<ModifiersDeclaration> Make(const Context& context,
                                                      const Modifiers& modifiers);

    const Layout& layout() const {
        return fLayout;
    }

    ModifierFlags modifierFlags() const {
        return fFlags;
    }

    std::string description() const override {
        return fLayout.paddedDescription() + fFlags.description() + ';';
    }

private:
    Layout fLayout;
    ModifierFlags fFlags;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
