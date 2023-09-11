/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLChildCall.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

using namespace skia_private;

namespace SkSL {

std::unique_ptr<Expression> ChildCall::clone(Position pos) const {
    return std::make_unique<ChildCall>(pos, &this->type(), &this->child(),
                                       this->arguments().clone());
}

std::string ChildCall::description(OperatorPrecedence) const {
    std::string result = std::string(this->child().name()) + ".eval(";
    auto separator = SkSL::String::Separator();
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += separator();
        result += arg->description(OperatorPrecedence::kSequence);
    }
    result += ")";
    return result;
}

[[maybe_unused]] static bool call_signature_is_valid(const Context& context,
                                                     const Variable& child,
                                                     const ExpressionArray& arguments) {
    const Type* half4 = context.fTypes.fHalf4.get();
    const Type* float2 = context.fTypes.fFloat2.get();

    auto params = [&]() -> STArray<2, const Type*> {
        switch (child.type().typeKind()) {
            case Type::TypeKind::kBlender:     return { half4, half4 };
            case Type::TypeKind::kColorFilter: return { half4 };
            case Type::TypeKind::kShader:      return { float2 };
            default:
                SkUNREACHABLE;
        }
    }();

    if (params.size() != arguments.size()) {
        return false;
    }
    for (int i = 0; i < arguments.size(); i++) {
        if (!arguments[i]->type().matches(*params[i])) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Expression> ChildCall::Make(const Context& context,
                                            Position pos,
                                            const Type* returnType,
                                            const Variable& child,
                                            ExpressionArray arguments) {
    SkASSERT(call_signature_is_valid(context, child, arguments));
    return std::make_unique<ChildCall>(pos, returnType, &child, std::move(arguments));
}

}  // namespace SkSL
