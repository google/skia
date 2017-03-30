/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSTATEMENT
#define SKSL_ASTSTATEMENT

#include <vector>
#include "SkSLASTPositionNode.h"
#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * Abstract supertype of all statements.
 */
struct ASTStatement : public ASTPositionNode {
    enum Kind {
        kBlock_Kind,
        kVarDeclaration_Kind,
        kExpression_Kind,
        kIf_Kind,
        kFor_Kind,
        kWhile_Kind,
        kDo_Kind,
        kSwitch_Kind,
        kReturn_Kind,
        kBreak_Kind,
        kContinue_Kind,
        kDiscard_Kind
    };

    ASTStatement(Position position, Kind kind)
    : INHERITED(position)
    , fKind(kind) {}

    Kind fKind;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
