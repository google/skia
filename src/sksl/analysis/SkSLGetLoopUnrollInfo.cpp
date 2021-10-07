/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkFloatingPoint.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <cmath>
#include <memory>

namespace SkSL {

// Loops that run for 100000+ iterations will exceed our program size limit.
static constexpr int kLoopTerminationLimit = 100000;

static int calculate_count(double start, double end, double delta, bool forwards, bool inclusive) {
    if (forwards != (start < end)) {
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

static const char* get_es2_loop_unroll_info(const Statement* loopInitializer,
                                            const Expression* loopTest,
                                            const Expression* loopNext,
                                            const Statement* loopStatement,
                                            LoopUnrollInfo& loopInfo) {
    //
    // init_declaration has the form: type_specifier identifier = constant_expression
    //
    if (!loopInitializer) {
        return "missing init declaration";
    }
    if (!loopInitializer->is<VarDeclaration>()) {
        return "invalid init declaration";
    }
    const VarDeclaration& initDecl = loopInitializer->as<VarDeclaration>();
    if (!initDecl.baseType().isNumber()) {
        return "invalid type for loop index";
    }
    if (initDecl.arraySize() != 0) {
        return "invalid type for loop index";
    }
    if (!initDecl.value()) {
        return "missing loop index initializer";
    }
    if (!ConstantFolder::GetConstantValue(*initDecl.value(), &loopInfo.fStart)) {
        return "loop index initializer must be a constant expression";
    }

    loopInfo.fIndex = &initDecl.var();

    auto is_loop_index = [&](const std::unique_ptr<Expression>& expr) {
        return expr->is<VariableReference>() &&
               expr->as<VariableReference>().variable() == loopInfo.fIndex;
    };

    //
    // condition has the form: loop_index relational_operator constant_expression
    //
    if (!loopTest) {
        return "missing condition";
    }
    if (!loopTest->is<BinaryExpression>()) {
        return "invalid condition";
    }
    const BinaryExpression& cond = loopTest->as<BinaryExpression>();
    if (!is_loop_index(cond.left())) {
        return "expected loop index on left hand side of condition";
    }
    // relational_operator is one of: > >= < <= == or !=
    switch (cond.getOperator().kind()) {
        case Token::Kind::TK_GT:
        case Token::Kind::TK_GTEQ:
        case Token::Kind::TK_LT:
        case Token::Kind::TK_LTEQ:
        case Token::Kind::TK_EQEQ:
        case Token::Kind::TK_NEQ:
            break;
        default:
            return "invalid relational operator";
    }
    double loopEnd = 0;
    if (!ConstantFolder::GetConstantValue(*cond.right(), &loopEnd)) {
        return "loop index must be compared with a constant expression";
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
        return "missing loop expression";
    }
    switch (loopNext->kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& next = loopNext->as<BinaryExpression>();
            if (!is_loop_index(next.left())) {
                return "expected loop index in loop expression";
            }
            if (!ConstantFolder::GetConstantValue(*next.right(), &loopInfo.fDelta)) {
                return "loop index must be modified by a constant expression";
            }
            switch (next.getOperator().kind()) {
                case Token::Kind::TK_PLUSEQ:                                      break;
                case Token::Kind::TK_MINUSEQ: loopInfo.fDelta = -loopInfo.fDelta; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPrefix: {
            const PrefixExpression& next = loopNext->as<PrefixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator().kind()) {
                case Token::Kind::TK_PLUSPLUS:   loopInfo.fDelta =  1; break;
                case Token::Kind::TK_MINUSMINUS: loopInfo.fDelta = -1; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPostfix: {
            const PostfixExpression& next = loopNext->as<PostfixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator().kind()) {
                case Token::Kind::TK_PLUSPLUS:   loopInfo.fDelta =  1; break;
                case Token::Kind::TK_MINUSMINUS: loopInfo.fDelta = -1; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        default:
            return "invalid loop expression";
    }

    //
    // Within the body of the loop, the loop index is not statically assigned to, nor is it used as
    // argument to a function 'out' or 'inout' parameter.
    //
    if (Analysis::StatementWritesToVariable(*loopStatement, initDecl.var())) {
        return "loop index must not be modified within body of the loop";
    }

    // Finally, compute the iteration count, based on the bounds, and the termination operator.
    loopInfo.fCount = 0;

    switch (cond.getOperator().kind()) {
        case Token::Kind::TK_LT:
            loopInfo.fCount = calculate_count(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                              /*forwards=*/true, /*inclusive=*/false);
            break;

        case Token::Kind::TK_GT:
            loopInfo.fCount = calculate_count(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                              /*forwards=*/false, /*inclusive=*/false);
            break;

        case Token::Kind::TK_LTEQ:
            loopInfo.fCount = calculate_count(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                              /*forwards=*/true, /*inclusive=*/true);
            break;

        case Token::Kind::TK_GTEQ:
            loopInfo.fCount = calculate_count(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                              /*forwards=*/false, /*inclusive=*/true);
            break;

        case Token::Kind::TK_NEQ: {
            float iterations = sk_ieee_double_divide(loopEnd - loopInfo.fStart, loopInfo.fDelta);
            loopInfo.fCount = std::ceil(iterations);
            if (loopInfo.fCount < 0 || loopInfo.fCount != iterations ||
                !std::isfinite(iterations)) {
                // The loop doesn't reach the exact endpoint and so will never terminate.
                loopInfo.fCount = kLoopTerminationLimit;
            }
            break;
        }
        case Token::Kind::TK_EQEQ: {
            if (loopInfo.fStart == loopEnd) {
                // Start and end begin in the same place, so we can run one iteration...
                if (loopInfo.fDelta) {
                    // ... and then they diverge, so the loop terminates.
                    loopInfo.fCount = 1;
                } else {
                    // ... but they never diverge, so the loop runs forever.
                    loopInfo.fCount = kLoopTerminationLimit;
                }
            } else {
                // Start never equals end, so the loop will not run a single iteration.
                loopInfo.fCount = 0;
            }
            break;
        }
        default: SkUNREACHABLE;
    }

    SkASSERT(loopInfo.fCount >= 0);
    if (loopInfo.fCount >= kLoopTerminationLimit) {
        return "loop must guarantee termination in fewer iterations";
    }

    return nullptr;  // All checks pass
}

std::unique_ptr<LoopUnrollInfo> Analysis::GetLoopUnrollInfo(int line,
                                                            const Statement* loopInitializer,
                                                            const Expression* loopTest,
                                                            const Expression* loopNext,
                                                            const Statement* loopStatement,
                                                            ErrorReporter* errors) {
    auto result = std::make_unique<LoopUnrollInfo>();
    if (const char* msg = get_es2_loop_unroll_info(loopInitializer, loopTest, loopNext,
                                                   loopStatement, *result)) {
        result = nullptr;
        if (errors) {
            errors->error(line, msg);
        }
    }
    return result;
}

}  // namespace SkSL
