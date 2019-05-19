/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTBREAKSTATEMENT
#define SKSL_ASTBREAKSTATEMENT

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'break' statement.
 */
struct ASTBreakStatement : public ASTStatement {
    ASTBreakStatement(int offset)
    : INHERITED(offset, kBreak_Kind) {}

    String description() const override {
        return String("break;");
    }

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
