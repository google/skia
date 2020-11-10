/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLPrefixExpression.h"

#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

std::unique_ptr<Expression> PrefixExpression::constantPropagate(const IRGenerator& irGenerator,
                                                                const DefinitionMap& definitions) {
    if (this->isNegationOfCompileTimeConstant()) {
        if (this->operand()->is<FloatLiteral>()) {
            // Converts -floatLiteral(1) to floatLiteral(-1).
            return std::make_unique<FloatLiteral>(irGenerator.fContext, fOffset,
                                                  -this->operand()->as<FloatLiteral>().value());
        }
        if (this->operand()->is<IntLiteral>()) {
            // Converts -intLiteral(1) to intLiteral(-1).
            return std::make_unique<IntLiteral>(irGenerator.fContext, fOffset,
                                                -this->operand()->as<IntLiteral>().value());
        }
        if (this->operand()->is<Constructor>()) {
            // We've found a negated constant vector, e.g.:
            //     -float4(float3(floatLiteral(1)), floatLiteral(2))
            // To optimize this, the outer negation is removed and each argument is negated:
            //     float4(-float3(floatLiteral(1)), -floatLiteral(2))
            // After another optimization pass:
            //     float4(float3(-floatLiteral(1)), floatLiteral(-2))
            // Steady state is reached after another optimization pass:
            //     float4(float3(floatLiteral(-1)), floatLiteral(-2))
            std::unique_ptr<Expression> result = this->operand()->clone();
            for (std::unique_ptr<Expression>& arg : result->as<Constructor>().arguments()) {
                arg = std::make_unique<PrefixExpression>(Token::Kind::TK_MINUS, std::move(arg));
            }
            return result;
        }
    }
    return nullptr;
}

}  // namespace SkSL
