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
 * A bracketed expression, as in '[0]', indicating an array access. Empty brackets (as occur in
 * 'float[](5, 6)' are represented with a null fExpression.
 */
struct ASTIndexSuffix : public ASTSuffix {
    ASTIndexSuffix(Position position)
    : INHERITED(position, ASTSuffix::kIndex_Kind)
    , fExpression(nullptr) {}

    ASTIndexSuffix(std::unique_ptr<ASTExpression> expression)
    : INHERITED(expression ? expression->fPosition : Position(), ASTSuffix::kIndex_Kind)
    , fExpression(std::move(expression)) {}

    String description() const override {
        if (fExpression) {
            return "[" + fExpression->description() + "]";
        } else {
            return String("[]");
        }
    }

    // may be null
    std::unique_ptr<ASTExpression> fExpression;

    typedef ASTSuffix INHERITED;
};

} // namespace

#endif
