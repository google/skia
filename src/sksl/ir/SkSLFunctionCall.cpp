/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

static bool has_compile_time_constant_arguments(const ExpressionArray& arguments) {
    for (const std::unique_ptr<Expression>& arg : arguments) {
        const Expression* expr = ConstantFolder::GetConstantValueForVariable(*arg);
        if (!expr->isCompileTimeConstant()) {
            return false;
        }
    }
    return true;
}

static std::unique_ptr<Expression> coalesce_bool_vector(
        const ExpressionArray& arguments,
        bool startingState,
        const std::function<bool(bool, bool)>& coalesce) {
    SkASSERT(arguments.size() == 1);
    const Expression* arg = ConstantFolder::GetConstantValueForVariable(*arguments.front());
    const Type& type = arg->type();
    SkASSERT(type.isVector());
    SkASSERT(type.componentType().isBoolean());

    bool value = startingState;
    for (int index = 0; index < type.columns(); ++index) {
        const Expression* subexpression = arg->getConstantSubexpression(index);
        SkASSERT(subexpression);
        value = coalesce(value, subexpression->as<BoolLiteral>().value());
    }

    return BoolLiteral::Make(arg->fOffset, value, &type.componentType());
}

template <typename LITERAL, typename FN>
static std::unique_ptr<Expression> optimize_comparison_of_type(const Context& context,
                                                               const Expression& left,
                                                               const Expression& right,
                                                               const FN& compare) {
    const Type& type = left.type();
    SkASSERT(type.isVector());
    SkASSERT(type.componentType().isNumber());
    SkASSERT(type == right.type());

    ExpressionArray result;
    result.reserve_back(type.columns());

    for (int index = 0; index < type.columns(); ++index) {
        const Expression* leftSubexpr = left.getConstantSubexpression(index);
        const Expression* rightSubexpr = right.getConstantSubexpression(index);
        SkASSERT(leftSubexpr);
        SkASSERT(rightSubexpr);
        bool value = compare(leftSubexpr->as<LITERAL>().value(),
                             rightSubexpr->as<LITERAL>().value());
        result.push_back(BoolLiteral::Make(context, leftSubexpr->fOffset, value));
    }

    const Type& bvecType = context.fTypes.fBool->toCompound(context, type.columns(), /*rows=*/1);
    return ConstructorCompound::Make(context, left.fOffset, bvecType, std::move(result));
}

template <typename FN>
static std::unique_ptr<Expression> optimize_comparison(const Context& context,
                                                       const ExpressionArray& arguments,
                                                       const FN& compare) {
    SkASSERT(arguments.size() == 2);
    const Expression* left = ConstantFolder::GetConstantValueForVariable(*arguments[0]);
    const Expression* right = ConstantFolder::GetConstantValueForVariable(*arguments[1]);
    const Type& type = left->type().componentType();

    if (type.isFloat()) {
        return optimize_comparison_of_type<FloatLiteral>(context, *left, *right, compare);
    }
    if (type.isInteger()) {
        return optimize_comparison_of_type<IntLiteral>(context, *left, *right, compare);
    }
    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

template <typename LITERAL, typename FN>
static std::unique_ptr<Expression> evaluate_intrinsic_1_of_type(const Context& context,
                                                                const Expression* arg,
                                                                const FN& evaluate) {
    const Type& vecType = arg->type();
    const Type& type = vecType.componentType();
    SkASSERT(type.isNumber());

    ExpressionArray result;
    result.reserve_back(vecType.columns());

    for (int index = 0; index < vecType.columns(); ++index) {
        const Expression* subexpr = arg->getConstantSubexpression(index);
        SkASSERT(subexpr);
        auto value = evaluate(subexpr->as<LITERAL>().value());
        result.push_back(LITERAL::Make(subexpr->fOffset, value, &type));
    }

    return ConstructorCompound::Make(context, arg->fOffset, vecType, std::move(result));
}


template <typename FN, bool kSupportsFloat = true, bool kSupportsInt = true>
static std::unique_ptr<Expression> evaluate_intrinsic_generic1(const Context& context,
                                                               const ExpressionArray& arguments,
                                                               const FN& evaluate) {
    SkASSERT(arguments.size() == 1);
    const Expression* arg = ConstantFolder::GetConstantValueForVariable(*arguments.front());
    const Type& type = arg->type().componentType();

    if constexpr (kSupportsFloat) {
        if (type.isFloat()) {
            return evaluate_intrinsic_1_of_type<FloatLiteral>(context, arg, evaluate);
        }
    }
    if constexpr (kSupportsInt) {
        if (type.isInteger()) {
            return evaluate_intrinsic_1_of_type<IntLiteral>(context, arg, evaluate);
        }
    }
    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

template <typename FN>
static std::unique_ptr<Expression> evaluate_intrinsic_float1(const Context& context,
                                                             const ExpressionArray& arguments,
                                                             const FN& evaluate) {
    return evaluate_intrinsic_generic1<FN, /*kSupportsFloat=*/true, /*kSupportsInt=*/false>(
            context, arguments, evaluate);
}

static std::unique_ptr<Expression> optimize_intrinsic_call(const Context& context,
                                                           IntrinsicKind intrinsic,
                                                           const ExpressionArray& arguments) {
    switch (intrinsic) {
        case k_all_IntrinsicKind:
            return coalesce_bool_vector(arguments, /*startingState=*/true,
                                        [](bool a, bool b) { return a && b; });
        case k_any_IntrinsicKind:
            return coalesce_bool_vector(arguments, /*startingState=*/false,
                                        [](bool a, bool b) { return a || b; });
        case k_greaterThan_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a > b; });

        case k_greaterThanEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a >= b; });

        case k_lessThan_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a < b; });

        case k_lessThanEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a <= b; });

        case k_equal_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a == b; });

        case k_notEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, [](auto a, auto b) { return a != b; });

        case k_abs_IntrinsicKind:
            return evaluate_intrinsic_generic1(context, arguments, [](auto a) { return abs(a); });

        case k_sign_IntrinsicKind:
            return evaluate_intrinsic_generic1(context, arguments,
                                               [](auto a) { return (a > 0) - (a < 0); });
        case k_sin_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return sin(a); });

        case k_cos_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return cos(a); });

        case k_tan_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return tan(a); });

        case k_asin_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return asin(a); });

        case k_acos_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return acos(a); });

        case k_sinh_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return sinh(a); });

        case k_cosh_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return cosh(a); });

        case k_tanh_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return tanh(a); });

        case k_ceil_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return ceil(a); });

        case k_floor_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return floor(a); });

        case k_fract_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments,
                                             [](float a) { return a - floor(a); });
        case k_trunc_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return trunc(a); });

        case k_exp_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return exp(a); });

        case k_log_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return log(a); });

        case k_exp2_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return exp2(a); });

        case k_log2_IntrinsicKind:
            return evaluate_intrinsic_float1(context, arguments, [](float a) { return log2(a); });

        default:
            return nullptr;
    }
}

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

    if (context.fConfig->fSettings.fOptimize) {
        // We might be able to optimize built-in intrinsics.
        if (function.isIntrinsic() && has_compile_time_constant_arguments(arguments)) {
            // The function is an intrinsic and all inputs are compile-time constants. Optimize it.
            if (std::unique_ptr<Expression> expr =
                        optimize_intrinsic_call(context, function.intrinsicKind(), arguments)) {
                return expr;
            }
        }
    }

    return std::make_unique<FunctionCall>(offset, returnType, &function, std::move(arguments));
}

}  // namespace SkSL
