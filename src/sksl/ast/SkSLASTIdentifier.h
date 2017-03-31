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
    ASTIdentifier(Position position, SkString text)
    : INHERITED(position, kIdentifier_Kind)
    , fText(std::move(text)) {}

    SkString description() const override {
        return fText;
    }

    const SkString fText;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
