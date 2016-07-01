/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTCONTINUESTATEMENT
#define SKSL_ASTCONTINUESTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'continue' statement. 
 */
struct ASTContinueStatement : public ASTStatement {
    ASTContinueStatement(Position position)
    : INHERITED(position, kContinue_Kind) {}

    std::string description() const override {
        return "continue;";
    }

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
