/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> IfStatement::clone() const {
    return std::make_unique<IfStatement>(fOffset, this->isStatic(), this->test()->clone(),
                                         this->ifTrue()->clone(),
                                         this->ifFalse() ? this->ifFalse()->clone() : nullptr);
}

String IfStatement::description() const {
    String result;
    if (this->isStatic()) {
        result += "@";
    }
    result += "if (" + this->test()->description() + ") " + this->ifTrue()->description();
    if (this->ifFalse()) {
        result += " else " + this->ifFalse()->description();
    }
    return result;
}

std::unique_ptr<Statement> IfStatement::Make(const Context& context, int offset, bool isStatic,
                                             std::unique_ptr<Expression> test,
                                             std::unique_ptr<Statement> ifTrue,
                                             std::unique_ptr<Statement> ifFalse) {
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test) {
        return nullptr;
    }
    if (test->is<BoolLiteral>()) {
        // Static Boolean values can fold down to a single branch.
        if (test->as<BoolLiteral>().value()) {
            return ifTrue;
        }
        if (ifFalse) {
            return ifFalse;
        }
        // False, but no else-clause. Not an error, so don't return null!
        return std::make_unique<Nop>();
    }
    return std::make_unique<IfStatement>(offset, isStatic, std::move(test), std::move(ifTrue),
                                         std::move(ifFalse));
}

}  // namespace SkSL
