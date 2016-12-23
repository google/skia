/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTSUFFIXEXPRESSION
#define SKSL_ASTSUFFIXEXPRESSION

#include "SkSLASTSuffix.h"
#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * An expression with an associated suffix.
 */
struct ASTSuffixExpression : public ASTExpression {
    ASTSuffixExpression(sk_up<ASTExpression> base, sk_up<ASTSuffix> suffix)
            : INHERITED(base->fPosition, kSuffix_Kind)
            , fBase(std::move(base))
            , fSuffix(std::move(suffix)) {}

    SkString description() const override {
        return fBase->description() + fSuffix->description();
    }

    const sk_up<ASTExpression> fBase;
    const sk_up<ASTSuffix> fSuffix;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
