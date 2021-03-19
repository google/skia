/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLFunctionCall.h"

namespace SkSL {

bool FunctionCall::hasProperty(Property property) const {
    if (property == Property::kSideEffects &&
        (this->function().modifiers().fFlags & Modifiers::kHasSideEffects_Flag)) {
        return true;
    }
    for (const auto& arg : this->arguments()) {
        if (arg->hasProperty(property)) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<Expression> FunctionCall::clone() const {
    ExpressionArray cloned;
    cloned.reserve_back(this->arguments().size());
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        cloned.push_back(arg->clone());
    }
    return std::make_unique<FunctionCall>(
            fOffset, &this->type(), &this->function(), std::move(cloned));
}

String FunctionCall::description() const {
    String result = String(this->function().name()) + "(";
    String separator;
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += separator;
        result += arg->description();
        separator = ", ";
    }
    result += ")";
    return result;
}

std::unique_ptr<Expression> FunctionCall::Convert(const Context& context,
                                                  int offset,
                                                  const FunctionDeclaration& function,
                                                  ExpressionArray arguments) {
    // Reject function calls with the wrong number of arguments.
    if (function.parameters().size() != arguments.size()) {
        String msg = "call to '" + function.name() + "' expected " +
                     to_string((int)function.parameters().size()) + " argument";
        if (function.parameters().size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string(arguments.count());
        context.fErrors.error(offset, msg);
        return nullptr;
    }

    // GLSL ES 1.0 requires static recursion be rejected by the compiler. Also, our CPU back-end
    // cannot handle recursion (and is tied to strictES2Mode front-ends). The safest way to reject
    // all (potentially) recursive code is to disallow calls to functions before they're defined.
    if (context.fConfig->strictES2Mode() && !function.definition() && !function.isBuiltin()) {
        context.fErrors.error(offset, "call to undefined function '" + function.name() + "'");
        return nullptr;
    }

    // Resolve generic types.
    FunctionDeclaration::ParamTypes types;
    const Type* returnType;
    if (!function.determineFinalTypes(arguments, &types, &returnType)) {
        String msg = "no match for " + function.name() + "(";
        String separator;
        for (const std::unique_ptr<Expression>& arg : arguments) {
            msg += separator;
            msg += arg->type().displayName();
            separator = ", ";
        }
        msg += ")";
        context.fErrors.error(offset, msg);
        return nullptr;
    }

    for (size_t i = 0; i < arguments.size(); i++) {
        // Coerce each argument to the proper type.
        arguments[i] = types[i]->coerceExpression(std::move(arguments[i]), context);
        if (!arguments[i]) {
            return nullptr;
        }
        // Update the refKind on out-parameters, and ensure that they are actually assignable.
        const Modifiers& paramModifiers = function.parameters()[i]->modifiers();
        if (paramModifiers.fFlags & Modifiers::kOut_Flag) {
            const VariableRefKind refKind = paramModifiers.fFlags & Modifiers::kIn_Flag
                                                    ? VariableReference::RefKind::kReadWrite
                                                    : VariableReference::RefKind::kPointer;
            if (!Analysis::MakeAssignmentExpr(arguments[i].get(), refKind, &context.fErrors)) {
                return nullptr;
            }
        }
    }

    return Make(context, offset, returnType, function, std::move(arguments));
}

std::unique_ptr<Expression> FunctionCall::Make(const Context& context,
                                               int offset,
                                               const Type* returnType,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments) {
    SkASSERT(function.parameters().size() == arguments.size());
    SkASSERT(function.definition() || function.isBuiltin() || !context.fConfig->strictES2Mode());

    return std::make_unique<FunctionCall>(offset, returnType, &function, std::move(arguments));
}

}  // namespace SkSL
