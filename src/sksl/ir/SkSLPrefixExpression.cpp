/*
 * Copyright 2016 Google Inc.
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
            return std::make_unique<FloatLiteral>(irGenerator.fContext, fOffset,
                                                  -this->operand()->as<FloatLiteral>().value());
        }
        if (this->operand()->is<IntLiteral>()) {
            return std::make_unique<IntLiteral>(irGenerator.fContext, fOffset,
                                                -this->operand()->as<IntLiteral>().value());
        }
        if (this->operand()->is<Constructor>()) {
            Constructor& constructor = this->operand()->as<Constructor>();
            if (constructor.isCompileTimeConstant()) {
                // We're negating a constant vector, e.g. -float4(1). Simplify it to float4(-1).
                const ExpressionArray& args = constructor.arguments();
                const Type& type = args[0]->type();
                if (type.isInteger()) {
                    auto result = std::make_unique<Constructor>(
                            constructor.fOffset, &constructor.type(), ExpressionArray{});
                    result->arguments().reserve_back(args.size());
                    for (size_t index=0; index<args.size(); ++index) {
                        int64_t value = constructor.getIVecComponent(index);
                        result->arguments().push_back(std::make_unique<IntLiteral>(
                                irGenerator.fContext, fOffset, -value));
                    }
                    return std::move(result);
                } else if (type.isFloat()) {
                    auto result = std::make_unique<Constructor>(
                            constructor.fOffset, &constructor.type(), ExpressionArray{});
                    result->arguments().reserve_back(args.size());
                    for (size_t index=0; index<args.size(); ++index) {
                        SKSL_FLOAT value = constructor.getFVecComponent(index);
                        result->arguments().push_back(std::make_unique<FloatLiteral>(
                                irGenerator.fContext, fOffset, -value));
                    }
                    return std::move(result);
                }
            }
        }
    }
    return nullptr;
}

}  // namespace SkSL
