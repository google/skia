/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_DISCARDSTATEMENT
#define SKSL_DISCARDSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'discard' statement. 
 */
struct DiscardStatement : public Statement {
    DiscardStatement(Position position)
    : INHERITED(position, kDiscard_Kind) {}

    SkString description() const override {
        return SkString("discard;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
