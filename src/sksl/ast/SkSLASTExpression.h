/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTEXPRESSION
#define SKSL_ASTEXPRESSION

#include "SkSLASTPositionNode.h"

namespace SkSL {

/**
 * Abstract supertype of all expressions.
 */
struct ASTExpression : public ASTPositionNode {
    enum Kind {
        kBinary_Kind,
        kBool_Kind,
        kFloat_Kind,
        kIdentifier_Kind,
        kInt_Kind,
        kNull_Kind,
        kPrefix_Kind,
        kSuffix_Kind,
        kTernary_Kind
    };

    ASTExpression(int offset, Kind kind)
    : INHERITED(offset)
    , fKind(kind) {}

    const Kind fKind;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
