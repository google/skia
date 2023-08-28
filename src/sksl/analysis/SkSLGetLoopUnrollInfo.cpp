/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/analysis/SkSLNoOpErrorReporter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <cmath>
#include <memory>

namespace SkSL {

class Context;

// Loops that run for 100000+ iterations will exceed our program size limit.
static constexpr int kLoopTerminationLimit = 100000;

static int calculate_count(double start, double end, double delta, bool forwards, bool inclusive) {
    if ((forwards && start > end) || (!forwards && start < end)) {
        // The loop starts in a completed state (the start has already advanced past the end).
        return 0;
    }
    if ((delta == 0.0) || forwards != (delta > 0.0)) {
        // The loop does not progress toward a completed state, and will never terminate.
        return kLoopTerminationLimit;
    }
    double iterations = sk_ieee_double_divide(end - start, delta);
    double count = std::ceil(iterations);
    if (inclusive && (count == iterations)) {
        count += 1.0;
    }
    if (count > kLoopTerminationLimit || !std::isfinite(count)) {
        // The loop runs for more iterations than we can safely unroll.
        return kLoopTerminationLimit;
    }
    return (int)count;
}

std::unique_ptr<LoopUnrollInfo> Analysis::GetLoopUnrollInfo(const Context& context,
                                                            Position loopPos,
                                                            const ForLoopPositions& positions,
                                                            const Statement* loopInitializer,
                                                            std::unique_ptr<Expression>* loopTest,
                                                            const Expression* loopNext,
                                                            const Statement* loopStatement,
                                                            ErrorReporter* errorPtr) {
    NoOpErrorReporter unused;
    ErrorReporter& errors = errorPtr ? *errorPtr : unused;

    auto loopInfo = std::make_unique<LoopUnrollInfo>();

    //
    // init_declaration has the form: type_specifier identifier = constant_expression
    //
    if (!loopInitializer) {
        Position pos = positions.initPosition.valid() ? positions.initPosition : loopPos;
        errors.error(pos, "missing init declaration");
        return nullptr;
    }
    if (!loopInitializer->is<VarDeclaration>()) {
        errors.error(loopInitializer->fPosition, "invalid init declaration");
        return nullptr;
    }
    const VarDeclaration& initDecl = loopInitializer->as<VarDeclaration>();
    if (!initDecl.baseType().isNumber()) {
        errors.error(loopInitializer->fPosition, "invalid type for loop index");
        return nullptr;
    }
    if (initDecl.arraySize() != 0) {
        errors.error(loopInitializer->fPosition, "invalid type for loop index");
        return nullptr;
    }
    if (!initDecl.value()) {
        errors.error(loopInitializer->fPosition, "missing loop index initializer");
        return nullptr;
    }
    if (!ConstantFolder::GetConstantValue(*initDecl.value(), &loopInfo->fStart)) {
        errors.error(loopInitializer->fPosition,
                     "loop index initializer must be a constant expression");
        return nullptr;
    }

    loopInfo->fIndex = initDecl.var();

    auto is_loop_index = [&](const std::unique_ptr<Expression>& expr) {
        return expr->is<VariableReference>() &&
               expr->as<VariableReference>().variable() == loopInfo->fIndex;
    };

    //
    // condition has the form: loop_index relational_operator constant_expression
    //
    if (!loopTest || !*loopTest) {
        Position pos = positions.conditionPosition.valid() ? positions.conditionPosition : loopPos;
        errors.error(pos, "missing condition");
        return nullptr;
    }
    if (!loopTest->get()->is<BinaryExpression>()) {
        errors.error(loopTest->get()->fPosition, "invalid condition");
        return nullptr;
    }
    const BinaryExpression* cond = &loopTest->get()->as<BinaryExpression>();
    if (!is_loop_index(cond->left())) {
        errors.error(cond->fPosition, "expected loop index on left hand side of condition");
        return nullptr;
    }
    // relational_operator is one of: > >= < <= == or !=
    switch (cond->getOperator().kind()) {
        case Operator::Kind::GT:
        case Operator::Kind::GTEQ:
        case Operator::Kind::LT:
        case Operator::Kind::LTEQ:
        case Operator::Kind::EQEQ:
        case Operator::Kind::NEQ:
            break;
        default:
            errors.error(cond->fPosition, "invalid relational operator");
            return nullptr;
    }
    double loopEnd = 0;
    if (!ConstantFolder::GetConstantValue(*cond->right(), &loopEnd)) {
        errors.error(cond->fPosition, "loop index must be compared with a constant expression");
        return nullptr;
    }

    //
    // expression has one of the following forms:
    //   loop_index++
    //   loop_index--
    //   loop_index += constant_expression
    //   loop_index -= constant_expression
    // The spec doesn't mention prefix increment and decrement, but there is some consensus that
    // it's an oversight, so we allow those as well.
    //
    if (!loopNext) {
        Position pos = positions.nextPosition.valid() ? positions.nextPosition : loopPos;
        errors.error(pos, "missing loop expression");
        return nullptr;
    }
    switch (loopNext->kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& next = loopNext->as<BinaryExpression>();
            if (!is_loop_index(next.left())) {
                errors.error(loopNext->fPosition, "expected loop index in loop expression");
                return nullptr;
            }
            if (!ConstantFolder::GetConstantValue(*next.right(), &loopInfo->fDelta)) {
                errors.error(loopNext->fPosition,
                             "loop index must be modified by a constant expression");
                return nullptr;
            }
            switch (next.getOperator().kind()) {
                case Operator::Kind::PLUSEQ:                                        break;
                case Operator::Kind::MINUSEQ: loopInfo->fDelta = -loopInfo->fDelta; break;
                default:
                    errors.error(loopNext->fPosition, "invalid operator in loop expression");
                    return nullptr;
            }
            break;
        }
        case Expression::Kind::kPrefix: {
            const PrefixExpression& next = loopNext->as<PrefixExpression>();
            if (!is_loop_index(next.operand())) {
                errors.error(loopNext->fPosition, "expected loop index in loop expression");
                return nullptr;
            }
            switch (next.getOperator().kind()) {
                case Operator::Kind::PLUSPLUS:   loopInfo->fDelta =  1; break;
                case Operator::Kind::MINUSMINUS: loopInfo->fDelta = -1; break;
                default:
                    errors.error(loopNext->fPosition, "invalid operator in loop expression");
                    return nullptr;
            }
            break;
        }
        case Expression::Kind::kPostfix: {
            const PostfixExpression& next = loopNext->as<PostfixExpression>();
            if (!is_loop_index(next.operand())) {
                errors.error(loopNext->fPosition, "expected loop index in loop expression");
                return nullptr;
            }
            switch (next.getOperator().kind()) {
                case Operator::Kind::PLUSPLUS:   loopInfo->fDelta =  1; break;
                case Operator::Kind::MINUSMINUS: loopInfo->fDelta = -1; break;
                default:
                    errors.error(loopNext->fPosition, "invalid operator in loop expression");
                    return nullptr;
            }
            break;
        }
        default:
            errors.error(loopNext->fPosition, "invalid loop expression");
            return nullptr;
    }

    //
    // Within the body of the loop, the loop index is not statically assigned to, nor is it used as
    // argument to a function 'out' or 'inout' parameter.
    //
    if (Analysis::StatementWritesToVariable(*loopStatement, *initDecl.var())) {
        errors.error(loopStatement->fPosition,
                     "loop index must not be modified within body of the loop");
        return nullptr;
    }

    // Finally, compute the iteration count, based on the bounds, and the termination operator.
    loopInfo->fCount = 0;

    switch (cond->getOperator().kind()) {
        case Operator::Kind::LT:
            loopInfo->fCount = calculate_count(loopInfo->fStart, loopEnd, loopInfo->fDelta,
                                              /*forwards=*/true, /*inclusive=*/false);
            break;

        case Operator::Kind::GT:
            loopInfo->fCount = calculate_count(loopInfo->fStart, loopEnd, loopInfo->fDelta,
                                              /*forwards=*/false, /*inclusive=*/false);
            break;

        case Operator::Kind::LTEQ:
            loopInfo->fCount = calculate_count(loopInfo->fStart, loopEnd, loopInfo->fDelta,
                                              /*forwards=*/true, /*inclusive=*/true);
            break;

        case Operator::Kind::GTEQ:
            loopInfo->fCount = calculate_count(loopInfo->fStart, loopEnd, loopInfo->fDelta,
                                              /*forwards=*/false, /*inclusive=*/true);
            break;

        case Operator::Kind::NEQ: {
            float iterations = sk_ieee_double_divide(loopEnd - loopInfo->fStart, loopInfo->fDelta);
            loopInfo->fCount = std::ceil(iterations);
            if (loopInfo->fCount < 0 || loopInfo->fCount != iterations ||
                !std::isfinite(iterations)) {
                // The loop doesn't reach the exact endpoint and so will never terminate.
                loopInfo->fCount = kLoopTerminationLimit;
            }
            if (loopInfo->fIndex->type().componentType().isFloat()) {
                // Rewrite `x != n` tests as `x < n` or `x > n` depending on the loop direction.
                // Less-than and greater-than tests avoid infinite loops caused by rounding error.
                Operator::Kind op = (loopInfo->fDelta > 0) ? Operator::Kind::LT
                                                           : Operator::Kind::GT;
                *loopTest = BinaryExpression::Make(context,
                                                   cond->fPosition,
                                                   cond->left()->clone(),
                                                   op,
                                                   cond->right()->clone());
                cond = &loopTest->get()->as<BinaryExpression>();
            }
            break;
        }
        case Operator::Kind::EQEQ: {
            if (loopInfo->fStart == loopEnd) {
                // Start and end begin in the same place, so we can run one iteration...
                if (loopInfo->fDelta) {
                    // ... and then they diverge, so the loop terminates.
                    loopInfo->fCount = 1;
                } else {
                    // ... but they never diverge, so the loop runs forever.
                    loopInfo->fCount = kLoopTerminationLimit;
                }
            } else {
                // Start never equals end, so the loop will not run a single iteration.
                loopInfo->fCount = 0;
            }
            break;
        }
        default: SkUNREACHABLE;
    }

    SkASSERT(loopInfo->fCount >= 0);
    if (loopInfo->fCount >= kLoopTerminationLimit) {
        errors.error(loopPos, "loop must guarantee termination in fewer iterations");
        return nullptr;
    }

    return loopInfo;
}

}  // namespace SkSL
