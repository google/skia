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
    ExpressionArray cloned;
    cloned.reserve_back(this->arguments().size());
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        cloned.push_back(arg->clone());
    }
    return std::make_unique<ChildCall>(fOffset, &this->type(), &this->child(), std::move(cloned));
}

String ChildCall::description() const {
    String result = String(this->child().name()) + "(";
    String separator;
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += separator;
        result += arg->description();
        separator = ", ";
    }
    result += ")";
    return result;
}

struct ChildCallSignature {
    const Type*               fReturnType = nullptr;
    SkSTArray<2, const Type*> fParamTypes;
};

static ChildCallSignature child_call_signature(const Context& context, const Variable& child) {
    const Type* half4  = context.fTypes.fHalf4.get();
    const Type* float2 = context.fTypes.fFloat2.get();

    switch (child.type().typeKind()) {
        case Type::TypeKind::kBlender:     return { half4, { half4, half4 } };
        case Type::TypeKind::kColorFilter: return { half4, { half4 } };
        case Type::TypeKind::kShader:      return { half4, { float2 } };
        default:
            SkUNREACHABLE;
    }
}

std::unique_ptr<Expression> ChildCall::Convert(const Context& context,
                                               int offset,
                                               const Variable& child,
                                               ExpressionArray arguments) {
    ChildCallSignature signature = child_call_signature(context, child);
    skstd::string_view typeName = child.type().name();

    // Reject function calls with the wrong number of arguments.
    if (signature.fParamTypes.size() != arguments.size()) {
        String msg = "call to '" + typeName + "' expected " +
                     to_string((int)signature.fParamTypes.size()) + " argument";
        if (signature.fParamTypes.size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string(arguments.count());
        context.fErrors->error(offset, msg);
        return nullptr;
    }

    for (size_t i = 0; i < arguments.size(); i++) {
        // Coerce each argument to the proper type.
        arguments[i] = signature.fParamTypes[i]->coerceExpression(std::move(arguments[i]), context);
        if (!arguments[i]) {
            return nullptr;
        }
    }

    return Make(context, offset, signature.fReturnType, child, std::move(arguments));
}

std::unique_ptr<Expression> ChildCall::Make(const Context& context,
                                            int offset,
                                            const Type* returnType,
                                            const Variable& child,
                                            ExpressionArray arguments) {
#ifdef SK_DEBUG
    ChildCallSignature signature = child_call_signature(context, child);
    SkASSERT(signature.fParamTypes.size() == arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        SkASSERT(arguments[i]->type() == *signature.fParamTypes[i]);
    }
#endif

    return std::make_unique<ChildCall>(offset, returnType, &child, std::move(arguments));
}

}  // namespace SkSL
