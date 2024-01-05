/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLDoStatement.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> DoStatement::Convert(const Context& context,
                                                Position pos,
                                                std::unique_ptr<Statement> stmt,
                                                std::unique_ptr<Expression> test) {
    if (context.fConfig->strictES2Mode()) {
        context.fErrors->error(pos, "do-while loops are not supported");
        return nullptr;
    }
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test) {
        return nullptr;
    }
    if (Analysis::DetectVarDeclarationWithoutScope(*stmt, context.fErrors)) {
        return nullptr;
    }
    return DoStatement::Make(context, pos, std::move(stmt), std::move(test));
}

std::unique_ptr<Statement> DoStatement::Make(const Context& context,
                                             Position pos,
                                             std::unique_ptr<Statement> stmt,
                                             std::unique_ptr<Expression> test) {
    SkASSERT(!context.fConfig->strictES2Mode());
    SkASSERT(test->type().matches(*context.fTypes.fBool));
    SkASSERT(!Analysis::DetectVarDeclarationWithoutScope(*stmt));
    return std::make_unique<DoStatement>(pos, std::move(stmt), std::move(test));
}

std::string DoStatement::description() const {
    return "do " + this->statement()->description() +
           " while (" + this->test()->description() + ");";
}

}  // namespace SkSL

