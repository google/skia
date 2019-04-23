/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSUFFIX
#define SKSL_ASTSUFFIX

#include "src/sksl/ast/SkSLASTExpression.h"
#include "src/sksl/ast/SkSLASTPositionNode.h"

namespace SkSL {

/**
 * This and its subclasses represents expression suffixes, such as '[0]' or '.rgb'. Suffixes are not
 * expressions in and of themselves; they are attached to expressions to modify them.
 */
struct ASTSuffix : public ASTPositionNode {
    enum Kind {
        kIndex_Kind,
        kCall_Kind,
        kField_Kind,
        kPostIncrement_Kind,
        kPostDecrement_Kind
    };

    ASTSuffix(int offset, Kind kind)
    : INHERITED(offset)
    , fKind(kind) {}

    String description() const override {
        switch (fKind) {
            case kPostIncrement_Kind:
                return String("++");
            case kPostDecrement_Kind:
                return String("--");
            default:
                ABORT("unsupported suffix operator");
        }
    }

    Kind fKind;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
