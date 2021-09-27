/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLPostfixExpression.h"

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

std::unique_ptr<Expression> PostfixExpression::Convert(const Context& context,
                                                       std::unique_ptr<Expression> base,
                                                       Operator op) {
    const Type& baseType = base->type();
    if (!baseType.isNumber()) {
        context.fErrors->error(base->fLine,
                               "'" + String(op.operatorName()) + "' cannot operate on '" +
                               baseType.displayName() + "'");
        return nullptr;
    }
    if (!Analysis::UpdateVariableRefKind(base.get(), VariableRefKind::kReadWrite,
                                         context.fErrors)) {
        return nullptr;
    }
    return PostfixExpression::Make(context, std::move(base), op);
}

std::unique_ptr<Expression> PostfixExpression::Make(const Context&,
                                                    std::unique_ptr<Expression> base,
                                                    Operator op) {
    SkASSERT(base->type().isNumber());
    SkASSERT(Analysis::IsAssignable(*base));
    return std::make_unique<PostfixExpression>(std::move(base), op);
}

}  // namespace SkSL
