/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STATEMENT
#define SKSL_STATEMENT

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLSymbol.h"

namespace SkSL {

/**
 * Abstract supertype of all statements.
 */
class Statement : public IRNode {
public:
    using Kind = StatementKind;

    Statement(Position pos, Kind kind)
    : INHERITED(pos, (int) kind) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind) fKind;
    }

    virtual bool isEmpty() const {
        return false;
    }

    virtual std::unique_ptr<Statement> clone() const = 0;

private:
    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
