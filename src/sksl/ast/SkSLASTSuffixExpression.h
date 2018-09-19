/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSUFFIXEXPRESSION
#define SKSL_ASTSUFFIXEXPRESSION

#include "src/sksl/ast/SkSLASTExpression.h"
#include "src/sksl/ast/SkSLASTSuffix.h"

namespace SkSL {

/**
 * An expression with an associated suffix.
 */
struct ASTSuffixExpression : public ASTExpression {
    ASTSuffixExpression(std::unique_ptr<ASTExpression> base, std::unique_ptr<ASTSuffix> suffix)
    : INHERITED(base->fOffset, kSuffix_Kind)
    , fBase(std::move(base))
    , fSuffix(std::move(suffix)) {}

    String description() const override {
        return fBase->description() + fSuffix->description();
    }

    const std::unique_ptr<ASTExpression> fBase;
    const std::unique_ptr<ASTSuffix> fSuffix;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
