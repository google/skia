/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTINDEXSUFFIX
#define SKSL_ASTINDEXSUFFIX

#include "SkSLASTExpression.h"
#include "SkSLASTSuffix.h"

namespace SkSL {

/**
 * A bracketed expression, as in '[0]', indicating an array access. 
 */
struct ASTIndexSuffix : public ASTSuffix {
    ASTIndexSuffix(std::unique_ptr<ASTExpression> expression) 
    : INHERITED(expression->fPosition, ASTSuffix::kIndex_Kind)
    , fExpression(std::move(expression)) {}

    std::string description() const override {
        return "[" + fExpression->description() + "]";
    }

    std::unique_ptr<ASTExpression> fExpression;

    typedef ASTSuffix INHERITED;
};

} // namespace

#endif
