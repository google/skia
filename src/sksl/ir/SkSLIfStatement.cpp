/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIfStatement.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::string IfStatement::description() const {
    std::string result;
    result += "if (" + this->test()->description() + ") " + this->ifTrue()->description();
    if (this->ifFalse()) {
        result += " else " + this->ifFalse()->description();
    }
    return result;
}

std::unique_ptr<Statement> IfStatement::Convert(const Context& context,
                                                Position pos,
                                                std::unique_ptr<Expression> test,
                                                std::unique_ptr<Statement> ifTrue,
                                                std::unique_ptr<Statement> ifFalse) {
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test) {
        return nullptr;
    }
    SkASSERT(ifTrue);
    if (Analysis::DetectVarDeclarationWithoutScope(*ifTrue, context.fErrors)) {
        return nullptr;
    }
    if (ifFalse && Analysis::DetectVarDeclarationWithoutScope(*ifFalse, context.fErrors)) {
        return nullptr;
    }
    return IfStatement::Make(context, pos, std::move(test), std::move(ifTrue), std::move(ifFalse));
}

static std::unique_ptr<Statement> replace_empty_with_nop(std::unique_ptr<Statement> stmt,
                                                         bool isEmpty) {
    return (stmt && (!isEmpty || stmt->is<Nop>())) ? std::move(stmt)
                                                   : Nop::Make();
}

std::unique_ptr<Statement> IfStatement::Make(const Context& context,
                                             Position pos,
                                             std::unique_ptr<Expression> test,
                                             std::unique_ptr<Statement> ifTrue,
                                             std::unique_ptr<Statement> ifFalse) {
    SkASSERT(test->type().matches(*context.fTypes.fBool));
    SkASSERT(!Analysis::DetectVarDeclarationWithoutScope(*ifTrue));
    SkASSERT(!ifFalse || !Analysis::DetectVarDeclarationWithoutScope(*ifFalse));

    const bool optimize = context.fConfig->fSettings.fOptimize;
    bool trueIsEmpty = false;
    bool falseIsEmpty = false;

    if (optimize) {
        // If both sides are empty, the if statement can be reduced to its test expression.
        trueIsEmpty = ifTrue->isEmpty();
        falseIsEmpty = !ifFalse || ifFalse->isEmpty();
        if (trueIsEmpty && falseIsEmpty) {
            return ExpressionStatement::Make(context, std::move(test));
        }
    }

    if (optimize) {
        // Static Boolean values can fold down to a single branch.
        const Expression* testValue = ConstantFolder::GetConstantValueForVariable(*test);
        if (testValue->isBoolLiteral()) {
            if (testValue->as<Literal>().boolValue()) {
                return replace_empty_with_nop(std::move(ifTrue), trueIsEmpty);
            } else {
                return replace_empty_with_nop(std::move(ifFalse), falseIsEmpty);
            }
        }
    }

    if (optimize) {
        // Replace an empty if-true branches with Nop; eliminate empty if-false branches entirely.
        ifTrue = replace_empty_with_nop(std::move(ifTrue), trueIsEmpty);
        if (falseIsEmpty) {
            ifFalse = nullptr;
        }
    }

    return std::make_unique<IfStatement>(
            pos, std::move(test), std::move(ifTrue), std::move(ifFalse));
}

}  // namespace SkSL
