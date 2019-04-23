/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTEXTENSION
#define SKSL_ASTEXTENSION

#include "src/sksl/ast/SkSLASTDeclaration.h"

namespace SkSL {

/**
 * An extension declaration.
 */
struct ASTExtension : public ASTDeclaration {
    ASTExtension(int offset, String name)
    : INHERITED(offset, kExtension_Kind)
    , fName(std::move(name)) {}

    String description() const override {
        return "#extension " + fName + " : enable";
    }

    const String fName;

    typedef ASTDeclaration INHERITED;
};

} // namespace

#endif
