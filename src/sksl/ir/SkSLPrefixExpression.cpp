/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLPrefixExpression.h"

#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

static std::unique_ptr<Expression> negate_operand(const Expression& operand) {
    switch (operand.kind()) {
        case Expression::Kind::kFloatLiteral:
            // Convert -floatLiteral(1) to floatLiteral(-1).
            return std::make_unique<FloatLiteral>(operand.fOffset,
                                                  -operand.as<FloatLiteral>().value(),
                                                  &operand.type());

        case Expression::Kind::kIntLiteral:
            // Convert -intLiteral(1) to intLiteral(-1).
            return std::make_unique<IntLiteral>(operand.fOffset,
                                                -operand.as<IntLiteral>().value(),
                                                &operand.type());

        default:
            // Convert Expr to Prefix(TK_MINUS, Expr).
            return std::make_unique<PrefixExpression>(Token::Kind::TK_MINUS,
                                                      operand.clone());
    }
}

std::unique_ptr<Expression> PrefixExpression::constantPropagate(const IRGenerator& irGenerator,
                                                                const DefinitionMap& definitions) {
    if (this->operand()->isCompileTimeConstant()) {
        if (this->getOperator() == Token::Kind::TK_MINUS) {
            // Constant-propagate negation onto compile-time constants.
            switch (this->operand()->kind()) {
                case Expression::Kind::kFloatLiteral:
                case Expression::Kind::kIntLiteral:
                    return negate_operand(*this->operand());

                case Expression::Kind::kConstructor: {
                    // We've found a negated constant vector, e.g.:
                    //     -float4(float3(floatLiteral(1)), floatLiteral(2))
                    // To optimize this, the outer negation is removed and each argument is negated:
                    //     float4(-float3(floatLiteral(1)), floatLiteral(-2))
                    // Steady state is reached after another optimization pass:
                    //     float4(float3(floatLiteral(-1)), floatLiteral(-2))
                    const Constructor& constructor = this->operand()->as<Constructor>();
                    ExpressionArray args;
                    args.reserve_back(constructor.arguments().size());
                    for (const std::unique_ptr<Expression>& arg : constructor.arguments()) {
                        args.push_back(negate_operand(*arg));
                    }
                    return std::make_unique<Constructor>(constructor.fOffset,
                                                         &constructor.type(),
                                                         std::move(args));
                }

                default:
                    break;
            }
        } else if (this->getOperator() == Token::Kind::TK_LOGICALNOT) {
            if (this->operand()->is<BoolLiteral>()) {
                // Convert !boolLiteral(true) to boolLiteral(false).
                const BoolLiteral& b = this->operand()->as<BoolLiteral>();
                return std::make_unique<BoolLiteral>(b.fOffset, !b.value(), &b.type());
            }
        }
    }
    return nullptr;
}

}  // namespace SkSL
