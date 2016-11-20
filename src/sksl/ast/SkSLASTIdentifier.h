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
    ASTIdentifier(Position position, std::string text)
    : INHERITED(position, kIdentifier_Kind)
    , fText(std::move(text)) {}

    std::string description() const override {
        return fText;
    }

    const std::string fText;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
