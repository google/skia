/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLChildCall.h"

namespace SkSL {

bool ChildCall::hasProperty(Property property) const {
    for (const auto& arg : this->arguments()) {
        if (arg->hasProperty(property)) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<Expression> ChildCall::clone() const {
    return std::make_unique<ChildCall>(fLine, &this->type(), &this->child(),
                                       this->arguments().clone());
}

String ChildCall::description() const {
    String result = String(this->child().name()) + ".eval(";
    String separator;
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += separator;
        result += arg->description();
        separator = ", ";
    }
    result += ")";
    return result;
}

[[maybe_unused]] static bool call_signature_is_valid(const Context& context,
                                                     const Variable& child,
                                                     const ExpressionArray& arguments) {
    const Type* half4 = context.fTypes.fHalf4.get();
    const Type* float2 = context.fTypes.fFloat2.get();

    auto params = [&]() -> SkSTArray<2, const Type*> {
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
    for (size_t i = 0; i < arguments.size(); i++) {
        if (!arguments[i]->type().matches(*params[i])) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Expression> ChildCall::Make(const Context& context,
                                            int line,
                                            const Type* returnType,
                                            const Variable& child,
                                            ExpressionArray arguments) {
    SkASSERT(call_signature_is_valid(context, child, arguments));
    return std::make_unique<ChildCall>(line, returnType, &child, std::move(arguments));
}

}  // namespace SkSL
