/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTIDENTIFIER
#define SKSL_ASTIDENTIFIER

#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * An identifier in an expression context.
 */
struct ASTIdentifier : public ASTExpression {
    ASTIdentifier(Position position, String text)
    : INHERITED(position, kIdentifier_Kind)
    , fText(std::move(text)) {}

    String description() const override {
        return fText;
    }

    const String fText;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
