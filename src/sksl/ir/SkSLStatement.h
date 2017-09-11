/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STATEMENT
#define SKSL_STATEMENT

#include "SkSLIRNode.h"
#include "SkSLType.h"

namespace SkSL {

/**
 * Abstract supertype of all statements.
 */
struct Statement : public IRNode {
    enum Kind {
        kBlock_Kind,
        kBreak_Kind,
        kContinue_Kind,
        kDiscard_Kind,
        kDo_Kind,
        kExpression_Kind,
        kFor_Kind,
        kGroup_Kind,
        kIf_Kind,
        kNop_Kind,
        kReturn_Kind,
        kSwitch_Kind,
        kVarDeclaration_Kind,
        kVarDeclarations_Kind,
        kWhile_Kind
    };

    Statement(Position position, Kind kind)
    : INHERITED(position)
    , fKind(kind) {}

    virtual bool isEmpty() const {
        return false;
    }

    const Kind fKind;

    typedef IRNode INHERITED;
};

} // namespace

#endif
