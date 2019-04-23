/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTIDENTIFIER
#define SKSL_ASTIDENTIFIER

#include "src/sksl/ast/SkSLASTExpression.h"

namespace SkSL {

/**
 * An identifier in an expression context.
 */
struct ASTIdentifier : public ASTExpression {
    ASTIdentifier(int offset, StringFragment text)
    : INHERITED(offset, kIdentifier_Kind)
    , fText(text) {}

    String description() const override {
        return String(fText);
    }

    const StringFragment fText;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
