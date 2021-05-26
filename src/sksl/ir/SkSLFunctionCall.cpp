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

template <typename T>
static std::unique_ptr<Expression> coalesce_vector(const ExpressionArray& arguments,
                                                   T startingState,
                                                   const std::function<T(T, T)>& coalesce,
                                                   const std::function<T(T)>& finalize) {
    SkASSERT(arguments.size() == 1);
    const Expression* arg = ConstantFolder::GetConstantValueForVariable(*arguments.front());
    SkASSERT(arg);
    const Type& vecType = arg->type();

    T value = startingState;
    for (int index = 0; index < vecType.columns(); ++index) {
        const Expression* subexpression = arg->getConstantSubexpression(index);
        SkASSERT(subexpression);
        value = coalesce(value, subexpression->as<Literal<T>>().value());
    }

    if (finalize) {
        value = finalize(value);
    }

    return Literal<T>::Make(arg->fOffset, value, &vecType.componentType());
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

    ExpressionArray array;
    array.reserve_back(type.columns());

    for (int index = 0; index < type.columns(); ++index) {
        const Expression* leftSubexpr = left.getConstantSubexpression(index);
        const Expression* rightSubexpr = right.getConstantSubexpression(index);
        SkASSERT(leftSubexpr);
        SkASSERT(rightSubexpr);
        bool value = compare(leftSubexpr->as<LITERAL>().value(),
                             rightSubexpr->as<LITERAL>().value());
        array.push_back(BoolLiteral::Make(context, leftSubexpr->fOffset, value));
    }

    const Type& bvecType = context.fTypes.fBool->toCompound(context, type.columns(), /*rows=*/1);
    return ConstructorCompound::Make(context, left.fOffset, bvecType, std::move(array));
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

template <typename T>
static std::unique_ptr<Expression> evaluate_n_way_intrinsic_of_type(
                                                            const Context& context,
                                                            const Expression* arg0,
                                                            const Expression* arg1,
                                                            const Expression* arg2,
                                                            const std::function<T(T, T, T)>& eval) {
    // Takes up to three arguments and evaluates them in tandem, equivalent to constructing a new
    // vector containing the results from:
    //     eval(arg0.x, arg1.x, arg2.x),
    //     eval(arg0.y, arg1.y, arg2.y),
    //     eval(arg0.z, arg1.z, arg2.z),
    //     eval(arg0.w, arg1.w, arg2.w)
    //
    // If an argument is null, zero is passed to the evaluation function. If the arguments are a mix
    // of scalars and vectors, scalars are interpreted as a vector containing the same value for
    // every component.
    arg0 = ConstantFolder::GetConstantValueForVariable(*arg0);
    SkASSERT(arg0);

    const Type& vecType =          arg0->type().isVector()  ? arg0->type() :
                          (arg1 && arg1->type().isVector()) ? arg1->type() :
                          (arg2 && arg2->type().isVector()) ? arg2->type() :
                                                              arg0->type();
    const Type& type = vecType.componentType();
    SkASSERT(arg0->type().componentType() == type);

    if (arg1) {
        arg1 = ConstantFolder::GetConstantValueForVariable(*arg1);
        SkASSERT(arg1);
        SkASSERT(arg1->type().componentType() == type);
    }

    if (arg2) {
        arg2 = ConstantFolder::GetConstantValueForVariable(*arg2);
        SkASSERT(arg2);
        SkASSERT(arg2->type().componentType() == type);
    }

    ExpressionArray array;
    array.reserve_back(vecType.columns());

    int arg0Index = 0;
    int arg1Index = 0;
    int arg2Index = 0;
    for (int index = 0; index < vecType.columns(); ++index) {
        const Expression* arg0Subexpr = arg0->getConstantSubexpression(arg0Index);
        arg0Index += arg0->type().isVector() ? 1 : 0;
        SkASSERT(arg0Subexpr);

        const Expression* arg1Subexpr = nullptr;
        if (arg1) {
            arg1Subexpr = arg1->getConstantSubexpression(arg1Index);
            arg1Index += arg1->type().isVector() ? 1 : 0;
            SkASSERT(arg1Subexpr);
        }

        const Expression* arg2Subexpr = nullptr;
        if (arg2) {
            arg2Subexpr = arg2->getConstantSubexpression(arg2Index);
            arg2Index += arg2->type().isVector() ? 1 : 0;
            SkASSERT(arg2Subexpr);
        }

        T value = eval(arg0Subexpr->as<Literal<T>>().value(),
                       arg1Subexpr ? arg1Subexpr->as<Literal<T>>().value() : T{},
                       arg2Subexpr ? arg2Subexpr->as<Literal<T>>().value() : T{});

        if constexpr (std::is_floating_point<T>::value) {
            // If evaluation of the intrinsic yields a non-finite value, do not optimize.
            if (!isfinite(value)) {
                return nullptr;
            }
        }

        array.push_back(Literal<T>::Make(arg0Subexpr->fOffset, value, &type));
    }

    return ConstructorCompound::Make(context, arg0->fOffset, vecType, std::move(array));
}

template <typename T>
static std::unique_ptr<Expression> evaluate_intrinsic(const Context& context,
                                                      const ExpressionArray& arguments,
                                                      const std::function<T(T)>& eval) {
    SkASSERT(arguments.size() == 1);

    if constexpr (std::is_same<T, bool>::value) {
        SkASSERT(arguments.front()->type().componentType().isBoolean());
    }
    if constexpr (std::is_same<T, float>::value) {
        SkASSERT(arguments.front()->type().componentType().isFloat());
    }
    if constexpr (std::is_same<T, SKSL_INT>::value) {
        SkASSERT(arguments.front()->type().componentType().isInteger());
    }

    return evaluate_n_way_intrinsic_of_type<T>(
            context, arguments.front().get(), /*arg1=*/nullptr, /*arg2=*/nullptr,
            [&eval](T a, T, T) { return eval(a); });
}

template <typename FN>
static std::unique_ptr<Expression> evaluate_intrinsic_numeric(const Context& context,
                                                              const ExpressionArray& arguments,
                                                              const FN& eval) {
    SkASSERT(arguments.size() == 1);
    const Type& type = arguments.front()->type().componentType();

    if (type.isFloat()) {
        return evaluate_intrinsic<float>(context, arguments, eval);
    }
    if (type.isInteger()) {
        return evaluate_intrinsic<SKSL_INT>(context, arguments, eval);
    }

    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

template <typename FN>
static std::unique_ptr<Expression> evaluate_pairwise_intrinsic(const Context& context,
                                                               const ExpressionArray& arguments,
                                                               const FN& eval) {
    SkASSERT(arguments.size() == 2);
    const Type& type = arguments.front()->type().componentType();

    if (type.isFloat()) {
        return evaluate_n_way_intrinsic_of_type<float>(
                context, arguments[0].get(), arguments[1].get(), /*arg2=*/nullptr,
                [&eval](float a, float b, float) { return eval(a, b); });
    }
    if (type.isInteger()) {
        return evaluate_n_way_intrinsic_of_type<SKSL_INT>(
                context, arguments[0].get(), arguments[1].get(), /*arg2=*/nullptr,
                [&eval](SKSL_INT a, SKSL_INT b, SKSL_INT) { return eval(a, b); });
    }

    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

template <typename FN>
static std::unique_ptr<Expression> evaluate_3_way_intrinsic(const Context& context,
                                                            const ExpressionArray& arguments,
                                                            const FN& eval) {
    SkASSERT(arguments.size() == 3);
    const Type& type = arguments.front()->type().componentType();

    if (type.isFloat()) {
        return evaluate_n_way_intrinsic_of_type<float>(
                context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
    }
    if (type.isInteger()) {
        return evaluate_n_way_intrinsic_of_type<SKSL_INT>(
                context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
    }

    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

static std::unique_ptr<Expression> optimize_intrinsic_call(const Context& context,
                                                           IntrinsicKind intrinsic,
                                                           const ExpressionArray& arguments) {
    switch (intrinsic) {
        case k_all_IntrinsicKind:
            return coalesce_vector<bool>(arguments, /*startingState=*/true,
                                         [](bool a, bool b) { return a && b; },
                                         /*finalize=*/nullptr);
        case k_any_IntrinsicKind:
            return coalesce_vector<bool>(arguments, /*startingState=*/false,
                                         [](bool a, bool b) { return a || b; },
                                         /*finalize=*/nullptr);
        case k_not_IntrinsicKind:
            return evaluate_intrinsic<bool>(context, arguments, [](bool a) { return !a; });

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
            return evaluate_intrinsic_numeric(context, arguments, [](auto a) { return abs(a); });

        case k_sign_IntrinsicKind:
            return evaluate_intrinsic_numeric(context, arguments,
                                              [](auto a) { return (a > 0) - (a < 0); });
        case k_sin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return sin(a); });

        case k_cos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return cos(a); });

        case k_tan_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return tan(a); });

        case k_asin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return asin(a); });

        case k_acos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return acos(a); });

        case k_sinh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return sinh(a); });

        case k_cosh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return cosh(a); });

        case k_tanh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return tanh(a); });

        case k_ceil_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return ceil(a); });

        case k_floor_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return floor(a); });

        case k_fract_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a - floor(a); });
        case k_trunc_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return trunc(a); });

        case k_mod_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto x, auto y) { return x - y * floor(x / y); });
        case k_exp_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return exp(a); });

        case k_log_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return log(a); });

        case k_exp2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return exp2(a); });

        case k_log2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return log2(a); });

        case k_sqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, [](float a) { return sqrt(a); });

        case k_saturate_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return (a < 0) ? 0 : (a > 1) ? 1 : a; });
        case k_round_IntrinsicKind:      // GLSL `round` documents its rounding mode as unspecified
        case k_roundEven_IntrinsicKind:  // and is allowed to behave identically to `roundEven`.
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return round(a / 2) * 2; });
        case k_inversesqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return 1 / sqrt(a); });
        case k_radians_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a * 0.0174532925; });
        case k_degrees_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a * 57.2957795; });
        case k_min_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto a, auto b) { return (a < b) ? a : b; });
        case k_max_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto a, auto b) { return (a > b) ? a : b; });
        case k_clamp_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments,
                    [](auto x, auto l, auto h) { return (x < l) ? l : (x > h) ? h : x; });
        case k_step_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto e, auto x) { return (x < e) ? 0 : 1; });
        case k_smoothstep_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments, [](auto edge0, auto edge1, auto x) {
                auto t = (x - edge0) / (edge1 - edge0);
                t = (t < 0) ? 0 : (t > 1) ? 1 : t;
                return t * t * (3.0 - 2.0 * t);
            });
        case k_length_IntrinsicKind:
            return coalesce_vector<float>(arguments, /*startingState=*/0,
                                         [](float a, float b) { return a + (b * b); },
                                         [](float a) { return sqrt(a); });
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
