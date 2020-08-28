/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIRNode.h"

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbol.h"

namespace SkSL {

std::unique_ptr<Expression> IRNode::cloneExpression() const {
    return std::unique_ptr<Expression>(static_cast<Expression*>(this->clone().release()));
}

std::unique_ptr<Statement> IRNode::cloneStatement() const {
    return std::unique_ptr<Statement>(static_cast<Statement*>(this->clone().release()));
}

std::unique_ptr<Symbol> IRNode::cloneSymbol() const {
    return std::unique_ptr<Symbol>(static_cast<Symbol*>(this->clone().release()));
}

}  // namespace SkSL
