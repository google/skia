/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLNop.h"

namespace SkSL {

std::unique_ptr<Statement> ExpressionStatement::Make(const Context& context,
                                                     std::unique_ptr<Expression> expr) {
    if (expr->isIncomplete(context)) {
        return nullptr;
    }

    if (context.fConfig->fSettings.fOptimize) {
        // Expression-statements without any side effect can be replaced with a Nop.
        if (!expr->hasSideEffects()) {
            return Nop::Make();
        }
    }

    return std::make_unique<ExpressionStatement>(std::move(expr));
}

}  // namespace SkSL
