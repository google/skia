/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERDECLARATION
#define SKSL_MODIFIERDECLARATION

#include "src/sksl/ir/SkSLModifiers.h"

namespace SkSL {

/**
 * A declaration that consists only of modifiers, e.g.:
 *
 * layout(blend_support_all_equations) out;
 */
struct ModifiersDeclaration : public IRNode {
    static constexpr Kind kIRNodeKind = kModifiers_Kind;

    ModifiersDeclaration(Modifiers modifiers)
    : INHERITED(-1, kIRNodeKind)
    , fModifiers(modifiers) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new ModifiersDeclaration(fModifiers));
    }

    String description() const override {
        return fModifiers.description() + ";";
    }

    Modifiers fModifiers;

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
