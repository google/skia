/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTINUESTATEMENT
#define SKSL_CONTINUESTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'continue' statement.
 */
struct ContinueStatement : public Statement {
    ContinueStatement(Position position)
    : INHERITED(position, kContinue_Kind) {}

    String description() const override {
        return String("continue;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
