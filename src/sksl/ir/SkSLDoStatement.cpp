/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLDoStatement.h"

namespace SkSL {

std::unique_ptr<Statement> DoStatement::Convert(const Context& context,
                                                std::unique_ptr<Statement> stmt,
                                                std::unique_ptr<Expression> test) {
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(stmt->fOffset, "do-while loops are not supported");
        return nullptr;
    }
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test) {
        return nullptr;
    }
    return DoStatement::Make(context, std::move(stmt), std::move(test));
}

std::unique_ptr<Statement> DoStatement::Make(const Context& context,
                                             std::unique_ptr<Statement> stmt,
                                             std::unique_ptr<Expression> test) {
    SkASSERT(!context.fConfig->strictES2Mode());
    SkASSERT(test->type() == *context.fTypes.fBool);
    return std::make_unique<DoStatement>(stmt->fOffset, std::move(stmt), std::move(test));
}

std::unique_ptr<Statement> DoStatement::clone() const {
    return std::make_unique<DoStatement>(fOffset, this->statement()->clone(),
                                         this->test()->clone());
}

String DoStatement::description() const {
    return "do " + this->statement()->description() +
           " while (" + this->test()->description() + ");";
}

}  // namespace SkSL

