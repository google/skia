/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTCONTINUESTATEMENT
#define SKSL_ASTCONTINUESTATEMENT

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'continue' statement.
 */
struct ASTContinueStatement : public ASTStatement {
    ASTContinueStatement(int offset)
    : INHERITED(offset, kContinue_Kind) {}

    String description() const override {
        return String("continue;");
    }

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
