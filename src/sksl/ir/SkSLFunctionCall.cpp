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
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

#include "include/private/SkFloatingPoint.h"
#include "include/sksl/DSLCore.h"
#include "src/core/SkMatrixInvert.h"

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

static double as_double(const Expression* expr) {
    if (expr) {
        if (expr->is<IntLiteral>()) {
            return (double)expr->as<IntLiteral>().value();
        }
        if (expr->is<FloatLiteral>()) {
            return (double)expr->as<FloatLiteral>().value();
        }
        if (expr->is<BoolLiteral>()) {
            return (double)expr->as<BoolLiteral>().value();
        }
        SkDEBUGFAILF("unexpected expression kind %d", (int)expr->kind());
    }
    return 0.0;
}

template <typename T>
static void type_check_expression(const Expression& expr);

template <>
void type_check_expression<float>(const Expression& expr) {
    SkASSERT(expr.type().componentType().isFloat());
}

template <>
void type_check_expression<SKSL_INT>(const Expression& expr) {
    SkASSERT(expr.type().componentType().isInteger());
}

template <>
void type_check_expression<bool>(const Expression& expr) {
    SkASSERT(expr.type().componentType().isBoolean());
}

static std::unique_ptr<Expression> make_literal(int offset, double value, const Type& type) {
    if (type.isFloat()) {
        return FloatLiteral::Make(offset, value, &type);
    }
    if (type.isInteger()) {
        return IntLiteral::Make(offset, value, &type);
    }
    if (type.isBoolean()) {
        return BoolLiteral::Make(offset, value, &type);
    }
    SkDEBUGFAILF("unexpected type %s", type.description().c_str());
    return nullptr;
}

using CoalesceFn = double (*)(double, double, double);
using FinalizeFn = double (*)(double);
using MakeLiteralFn = std::unique_ptr<Expression> (*)(int, double, const Type*);

static std::unique_ptr<Expression> coalesce_n_way_vector(const Expression* arg0,
                                                         const Expression* arg1,
                                                         double startingState,
                                                         const Type& returnType,
                                                         CoalesceFn coalesce,
                                                         FinalizeFn finalize) {
    // Takes up to two vector or scalar arguments and coalesces them in sequence:
    //     scalar = startingState;
    //     scalar = coalesce(scalar, arg0.x, arg1.x);
    //     scalar = coalesce(scalar, arg0.y, arg1.y);
    //     scalar = coalesce(scalar, arg0.z, arg1.z);
    //     scalar = coalesce(scalar, arg0.w, arg1.w);
    //     scalar = finalize(scalar);
    //
    // If an argument is null, zero is passed to the coalesce function. If the arguments are a mix
    // of scalars and vectors, the scalars is interpreted as a vector containing the same value for
    // every component.

    int offset = arg0->fOffset;

    arg0 = ConstantFolder::GetConstantValueForVariable(*arg0);
    SkASSERT(arg0);

    const Type& vecType =          arg0->type().isVector()  ? arg0->type() :
                          (arg1 && arg1->type().isVector()) ? arg1->type() :
                                                              arg0->type();
    SkASSERT(arg0->type().componentType() == vecType.componentType());

    if (arg1) {
        arg1 = ConstantFolder::GetConstantValueForVariable(*arg1);
        SkASSERT(arg1);
        SkASSERT(arg1->type().componentType() == vecType.componentType());
    }

    double value = startingState;
    int arg0Index = 0;
    int arg1Index = 0;
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

        value = coalesce(value, as_double(arg0Subexpr), as_double(arg1Subexpr));

        // If coalescing the intrinsic yields a non-finite value, do not optimize.
        if (!std::isfinite(value)) {
            return nullptr;
        }
    }

    if (finalize) {
        value = finalize(value);
    }

    return make_literal(offset, value, returnType);
}

template <typename T>
static std::unique_ptr<Expression> coalesce_vector(const ExpressionArray& arguments,
                                                   double startingState,
                                                   const Type& returnType,
                                                   CoalesceFn coalesce,
                                                   FinalizeFn finalize) {
    SkASSERT(arguments.size() == 1);
    type_check_expression<T>(*arguments[0]);

    return coalesce_n_way_vector(arguments[0].get(), /*arg1=*/nullptr,
                                 (double)startingState, returnType, coalesce, finalize);
}

template <typename T>
static std::unique_ptr<Expression> coalesce_pairwise_vectors(const ExpressionArray& arguments,
                                                             double startingState,
                                                             const Type& returnType,
                                                             CoalesceFn coalesce,
                                                             FinalizeFn finalize) {
    SkASSERT(arguments.size() == 2);
    type_check_expression<T>(*arguments[0]);
    type_check_expression<T>(*arguments[1]);

    return coalesce_n_way_vector(arguments[0].get(), arguments[1].get(),
                                 (double)startingState, returnType, coalesce, finalize);
}

using CompareFn = bool (*)(double, double);

static std::unique_ptr<Expression> optimize_comparison(const Context& context,
                                                       const ExpressionArray& arguments,
                                                       CompareFn compare) {
    SkASSERT(arguments.size() == 2);
    const Expression* left = ConstantFolder::GetConstantValueForVariable(*arguments[0]);
    const Expression* right = ConstantFolder::GetConstantValueForVariable(*arguments[1]);
    SkASSERT(left);
    SkASSERT(right);

    const Type& type = left->type();
    SkASSERT(type.isVector());
    SkASSERT(type.componentType().isNumber());
    SkASSERT(type == right->type());

    ExpressionArray array;
    array.reserve_back(type.columns());

    for (int index = 0; index < type.columns(); ++index) {
        const Expression* leftSubexpr = left->getConstantSubexpression(index);
        const Expression* rightSubexpr = right->getConstantSubexpression(index);
        SkASSERT(leftSubexpr);
        SkASSERT(rightSubexpr);
        bool value = compare(as_double(leftSubexpr), as_double(rightSubexpr));
        array.push_back(BoolLiteral::Make(context, leftSubexpr->fOffset, value));
    }

    const Type& bvecType = context.fTypes.fBool->toCompound(context, type.columns(), /*rows=*/1);
    return ConstructorCompound::Make(context, left->fOffset, bvecType, std::move(array));
}

using EvaluateFn = double (*)(double, double, double);

static std::unique_ptr<Expression> evaluate_n_way_intrinsic(const Context& context,
                                                            const Expression* arg0,
                                                            const Expression* arg1,
                                                            const Expression* arg2,
                                                            const Type& returnType,
                                                            EvaluateFn eval) {
    // Takes up to three arguments and evaluates all of them, left-to-right, in tandem.
    // Equivalent to constructing a new compound value containing the results from:
    //     eval(arg0.x, arg1.x, arg2.x),
    //     eval(arg0.y, arg1.y, arg2.y),
    //     eval(arg0.z, arg1.z, arg2.z),
    //     eval(arg0.w, arg1.w, arg2.w)
    //
    // If an argument is null, zero is passed to the evaluation function. If the arguments are a mix
    // of scalars and compounds, scalars are interpreted as a compound containing the same value for
    // every component.
    arg0 = ConstantFolder::GetConstantValueForVariable(*arg0);
    SkASSERT(arg0);

    if (arg1) {
        arg1 = ConstantFolder::GetConstantValueForVariable(*arg1);
        SkASSERT(arg1);
    }

    if (arg2) {
        arg2 = ConstantFolder::GetConstantValueForVariable(*arg2);
        SkASSERT(arg2);
    }

    int slots = returnType.slotCount();
    ExpressionArray array;
    array.reserve_back(slots);

    int arg0Index = 0;
    int arg1Index = 0;
    int arg2Index = 0;
    for (int index = 0; index < slots; ++index) {
        const Expression* arg0Subexpr = arg0->getConstantSubexpression(arg0Index);
        arg0Index += arg0->type().isScalar() ? 0 : 1;
        SkASSERT(arg0Subexpr);

        const Expression* arg1Subexpr = nullptr;
        if (arg1) {
            arg1Subexpr = arg1->getConstantSubexpression(arg1Index);
            arg1Index += arg1->type().isScalar() ? 0 : 1;
            SkASSERT(arg1Subexpr);
        }

        const Expression* arg2Subexpr = nullptr;
        if (arg2) {
            arg2Subexpr = arg2->getConstantSubexpression(arg2Index);
            arg2Index += arg2->type().isScalar() ? 0 : 1;
            SkASSERT(arg2Subexpr);
        }

        double value = eval(as_double(arg0Subexpr), as_double(arg1Subexpr), as_double(arg2Subexpr));

        // If evaluation of the intrinsic yields a non-finite value, do not optimize.
        if (!std::isfinite(value)) {
            return nullptr;
        }

        array.push_back(make_literal(arg0Subexpr->fOffset, value, returnType.componentType()));
    }

    return ConstructorCompound::Make(context, arg0->fOffset, returnType, std::move(array));
}

template <typename T>
static std::unique_ptr<Expression> evaluate_intrinsic(const Context& context,
                                                      const ExpressionArray& arguments,
                                                      const Type& returnType,
                                                      EvaluateFn eval) {
    SkASSERT(arguments.size() == 1);
    type_check_expression<T>(*arguments[0]);

    return evaluate_n_way_intrinsic(context, arguments[0].get(), /*arg1=*/nullptr, /*arg2=*/nullptr,
                                    returnType, eval);
}

static std::unique_ptr<Expression> evaluate_intrinsic_numeric(const Context& context,
                                                              const ExpressionArray& arguments,
                                                              const Type& returnType,
                                                              EvaluateFn eval) {
    SkASSERT(arguments.size() == 1);
    const Type& type = arguments[0]->type().componentType();

    if (type.isFloat()) {
        return evaluate_intrinsic<float>(context, arguments, returnType, eval);
    }
    if (type.isInteger()) {
        return evaluate_intrinsic<SKSL_INT>(context, arguments, returnType, eval);
    }

    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

static std::unique_ptr<Expression> evaluate_pairwise_intrinsic(const Context& context,
                                                               const ExpressionArray& arguments,
                                                               const Type& returnType,
                                                               EvaluateFn eval) {
    SkASSERT(arguments.size() == 2);
    const Type& type = arguments[0]->type().componentType();

    if (type.isFloat()) {
        type_check_expression<float>(*arguments[0]);
        type_check_expression<float>(*arguments[1]);
    } else if (type.isInteger()) {
        type_check_expression<SKSL_INT>(*arguments[0]);
        type_check_expression<SKSL_INT>(*arguments[1]);
    } else {
        SkDEBUGFAILF("unsupported type %s", type.description().c_str());
        return nullptr;
    }

    return evaluate_n_way_intrinsic(context, arguments[0].get(), arguments[1].get(),
                                    /*arg2=*/nullptr, returnType, eval);
}

static std::unique_ptr<Expression> evaluate_3_way_intrinsic(const Context& context,
                                                            const ExpressionArray& arguments,
                                                            const Type& returnType,
                                                            EvaluateFn eval) {
    SkASSERT(arguments.size() == 3);
    const Type& type = arguments[0]->type().componentType();

    if (type.isFloat()) {
        type_check_expression<float>(*arguments[0]);
        type_check_expression<float>(*arguments[1]);
        type_check_expression<float>(*arguments[2]);
    } else if (type.isInteger()) {
        type_check_expression<SKSL_INT>(*arguments[0]);
        type_check_expression<SKSL_INT>(*arguments[1]);
        type_check_expression<SKSL_INT>(*arguments[2]);
    } else {
        SkDEBUGFAILF("unsupported type %s", type.description().c_str());
        return nullptr;
    }

    return evaluate_n_way_intrinsic(context, arguments[0].get(), arguments[1].get(),
                                    arguments[2].get(), returnType, eval);
}

template <typename T1, typename T2>
static double pun_value(double val) {
    // Interpret `val` as a value of type T1.
    static_assert(sizeof(T1) == sizeof(T2));
    T1 inputValue = (T1)val;
    // Reinterpret those bits as a value of type T2.
    T2 outputValue;
    memcpy(&outputValue, &inputValue, sizeof(T2));
    // Return the value-of-type-T2 as a double. (Non-finite values will prohibit optimization.)
    return (double)outputValue;
}

// Helper functions for optimizing all of our intrinsics.
namespace Intrinsics {
namespace {

double coalesce_length(double a, double b, double)     { return a + (b * b); }
double finalize_length(double a)                       { return std::sqrt(a); }

double coalesce_distance(double a, double b, double c) { b -= c; return a + (b * b); }
double finalize_distance(double a)                     { return std::sqrt(a); }

double coalesce_dot(double a, double b, double c)      { return a + (b * c); }
double coalesce_any(double a, double b, double)        { return a || b; }
double coalesce_all(double a, double b, double)        { return a && b; }

bool compare_lessThan(double a, double b)              { return a < b; }
bool compare_lessThanEqual(double a, double b)         { return a <= b; }
bool compare_greaterThan(double a, double b)           { return a > b; }
bool compare_greaterThanEqual(double a, double b)      { return a >= b; }
bool compare_equal(double a, double b)                 { return a == b; }
bool compare_notEqual(double a, double b)              { return a != b; }

double evaluate_radians(double a, double, double)      { return a * 0.0174532925; }
double evaluate_degrees(double a, double, double)      { return a * 57.2957795; }
double evaluate_sin(double a, double, double)          { return std::sin(a); }
double evaluate_cos(double a, double, double)          { return std::cos(a); }
double evaluate_tan(double a, double, double)          { return std::tan(a); }
double evaluate_asin(double a, double, double)         { return std::asin(a); }
double evaluate_acos(double a, double, double)         { return std::acos(a); }
double evaluate_atan(double a, double, double)         { return std::atan(a); }
double evaluate_atan2(double a, double b, double)      { return std::atan2(a, b); }
double evaluate_asinh(double a, double, double)        { return std::asinh(a); }
double evaluate_acosh(double a, double, double)        { return std::acosh(a); }
double evaluate_atanh(double a, double, double)        { return std::atanh(a); }

double evaluate_pow(double a, double b, double)        { return std::pow(a, b); }
double evaluate_exp(double a, double, double)          { return std::exp(a); }
double evaluate_log(double a, double, double)          { return std::log(a); }
double evaluate_exp2(double a, double, double)         { return std::exp2(a); }
double evaluate_log2(double a, double, double)         { return std::log2(a); }
double evaluate_sqrt(double a, double, double)         { return std::sqrt(a); }
double evaluate_inversesqrt(double a, double, double) {
    return sk_ieee_double_divide(1.0, std::sqrt(a));
}

double evaluate_abs(double a, double, double)          { return std::abs(a); }
double evaluate_sign(double a, double, double)         { return (a > 0) - (a < 0); }
double evaluate_floor(double a, double, double)        { return std::floor(a); }
double evaluate_ceil(double a, double, double)         { return std::ceil(a); }
double evaluate_fract(double a, double, double)        { return a - std::floor(a); }
double evaluate_min(double a, double b, double)        { return (a < b) ? a : b; }
double evaluate_max(double a, double b, double)        { return (a > b) ? a : b; }
double evaluate_clamp(double x, double l, double h)    { return (x < l) ? l : (x > h) ? h : x; }
double evaluate_saturate(double a, double, double)     { return (a < 0) ? 0 : (a > 1) ? 1 : a; }
double evaluate_mix(double x, double y, double a)      { return x * (1 - a) + y * a; }
double evaluate_step(double e, double x, double)       { return (x < e) ? 0 : 1; }
double evaluate_mod(double a, double b, double) {
    return a - b * std::floor(sk_ieee_double_divide(a, b));
}
double evaluate_smoothstep(double edge0, double edge1, double x) {
    double t = sk_ieee_double_divide(x - edge0, edge1 - edge0);
    t = (t < 0) ? 0 : (t > 1) ? 1 : t;
    return t * t * (3.0 - 2.0 * t);
}

double evaluate_matrixCompMult(double x, double y, double) { return x * y; }

double evaluate_not(double a, double, double)          { return !a; }
double evaluate_sinh(double a, double, double)         { return std::sinh(a); }
double evaluate_cosh(double a, double, double)         { return std::cosh(a); }
double evaluate_tanh(double a, double, double)         { return std::tanh(a); }
double evaluate_trunc(double a, double, double)        { return std::trunc(a); }
double evaluate_round(double a, double, double) {
    // The semantics of std::remainder guarantee a rounded-to-even result here, regardless of the
    // current float-rounding mode.
    return a - std::remainder(a, 1.0);
}
double evaluate_floatBitsToInt(double a, double, double)  { return pun_value<float, int32_t> (a); }
double evaluate_floatBitsToUint(double a, double, double) { return pun_value<float, uint32_t>(a); }
double evaluate_intBitsToFloat(double a, double, double)  { return pun_value<int32_t,  float>(a); }
double evaluate_uintBitsToFloat(double a, double, double) { return pun_value<uint32_t, float>(a); }

}  // namespace
}  // namespace Intrinsics

static void extract_matrix(const Expression* expr, float mat[16]) {
    size_t numSlots = expr->type().slotCount();
    for (size_t index = 0; index < numSlots; ++index) {
        mat[index] = expr->getConstantSubexpression(index)->as<FloatLiteral>().value();
    }
}

static std::unique_ptr<Expression> optimize_intrinsic_call(const Context& context,
                                                           IntrinsicKind intrinsic,
                                                           const ExpressionArray& arguments,
                                                           const Type& returnType) {
    // Helper function for accessing a matrix argument by column and row.
    const Expression* matrix = nullptr;
    auto M = [&](int c, int r) -> float {
        int index = (matrix->type().rows() * c) + r;
        return matrix->getConstantSubexpression(index)->as<FloatLiteral>().value();
    };

    using namespace SkSL::dsl;
    switch (intrinsic) {
        // 8.1 : Angle and Trigonometry Functions
        case k_radians_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_radians);
        case k_degrees_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_degrees);
        case k_sin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_sin);
        case k_cos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_cos);
        case k_tan_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_tan);
        case k_sinh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_sinh);
        case k_cosh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_cosh);
        case k_tanh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_tanh);
        case k_asin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_asin);
        case k_acos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_acos);
        case k_atan_IntrinsicKind:
            if (arguments.size() == 1) {
                return evaluate_intrinsic<float>(context, arguments, returnType,
                                                 Intrinsics::evaluate_atan);
            } else {
                return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                                   Intrinsics::evaluate_atan2);
            }
        case k_asinh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_asinh);

        case k_acosh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_acosh);
        case k_atanh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_atanh);
        // 8.2 : Exponential Functions
        case k_pow_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_pow);
        case k_exp_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_exp);
        case k_log_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_log);
        case k_exp2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_exp2);
        case k_log2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_log2);
        case k_sqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_sqrt);
        case k_inversesqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_inversesqrt);
        // 8.3 : Common Functions
        case k_abs_IntrinsicKind:
            return evaluate_intrinsic_numeric(context, arguments, returnType,
                                              Intrinsics::evaluate_abs);
        case k_sign_IntrinsicKind:
            return evaluate_intrinsic_numeric(context, arguments, returnType,
                                              Intrinsics::evaluate_sign);
        case k_floor_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_floor);
        case k_ceil_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_ceil);
        case k_fract_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_fract);
        case k_mod_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_mod);
        case k_min_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_min);
        case k_max_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_max);
        case k_clamp_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments, returnType,
                                            Intrinsics::evaluate_clamp);
        case k_saturate_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_saturate);
        case k_mix_IntrinsicKind:
            if (arguments[2]->type().componentType().isBoolean()) {
                const SkSL::Type& numericType = arguments[0]->type().componentType();

                if (numericType.isFloat()) {
                    type_check_expression<float>(*arguments[0]);
                    type_check_expression<float>(*arguments[1]);
                } else if (numericType.isInteger()) {
                    type_check_expression<SKSL_INT>(*arguments[0]);
                    type_check_expression<SKSL_INT>(*arguments[1]);
                } else if (numericType.isBoolean()) {
                    type_check_expression<bool>(*arguments[0]);
                    type_check_expression<bool>(*arguments[1]);
                } else {
                    SkDEBUGFAILF("unsupported type %s", numericType.description().c_str());
                    return nullptr;
                }
                return evaluate_n_way_intrinsic(context, arguments[0].get(), arguments[1].get(),
                                                arguments[2].get(), returnType,
                                                Intrinsics::evaluate_mix);
            } else {
                return evaluate_3_way_intrinsic(context, arguments, returnType,
                                                Intrinsics::evaluate_mix);
            }
        case k_step_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_step);
        case k_smoothstep_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments, returnType,
                                            Intrinsics::evaluate_smoothstep);
        case k_trunc_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_trunc);
        case k_round_IntrinsicKind:      // GLSL `round` documents its rounding mode as unspecified
        case k_roundEven_IntrinsicKind:  // and is allowed to behave identically to `roundEven`.
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_round);
        case k_floatBitsToInt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_floatBitsToInt);
        case k_floatBitsToUint_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments, returnType,
                                             Intrinsics::evaluate_floatBitsToUint);
        case k_intBitsToFloat_IntrinsicKind:
            return evaluate_intrinsic<SKSL_INT>(context, arguments, returnType,
                                                Intrinsics::evaluate_intBitsToFloat);
        case k_uintBitsToFloat_IntrinsicKind:
            return evaluate_intrinsic<SKSL_INT>(context, arguments, returnType,
                                                Intrinsics::evaluate_uintBitsToFloat);
        // TODO(skia:12202): 8.4 : Floating-Point Pack and Unpack Functions

        // 8.5 : Geometric Functions
        case k_length_IntrinsicKind:
            return coalesce_vector<float>(arguments, /*startingState=*/0, returnType,
                                          Intrinsics::coalesce_length,
                                          Intrinsics::finalize_length);
        case k_distance_IntrinsicKind:
            return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0, returnType,
                                                    Intrinsics::coalesce_distance,
                                                    Intrinsics::finalize_distance);
        case k_dot_IntrinsicKind:
            return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0, returnType,
                                                    Intrinsics::coalesce_dot,
                                                    /*finalize=*/nullptr);
        case k_cross_IntrinsicKind: {
            auto Value = [&](int a, int n) -> float {
                return arguments[a]->getConstantSubexpression(n)->as<FloatLiteral>().value();
            };
            auto X = [&](int n) -> float { return Value(0, n); };
            auto Y = [&](int n) -> float { return Value(1, n); };
            SkASSERT(arguments[0]->type().columns() == 3);  // the vec2 form is not a real intrinsic
            return DSLType::Construct(&arguments[0]->type(),
                                      X(1) * Y(2) - Y(1) * X(2),
                                      X(2) * Y(0) - Y(2) * X(0),
                                      X(0) * Y(1) - Y(0) * X(1)).release();
        }
        case k_normalize_IntrinsicKind: {
            auto Vec = [&] { return DSLExpression{arguments[0]->clone()}; };
            return (Vec() / Length(Vec())).release();
        }
        case k_faceforward_IntrinsicKind: {
            auto N    = [&] { return DSLExpression{arguments[0]->clone()}; };
            auto I    = [&] { return DSLExpression{arguments[1]->clone()}; };
            auto NRef = [&] { return DSLExpression{arguments[2]->clone()}; };
            return (N() * Select(Dot(NRef(), I()) < 0, 1, -1)).release();
        }
        case k_reflect_IntrinsicKind: {
            auto I    = [&] { return DSLExpression{arguments[0]->clone()}; };
            auto N    = [&] { return DSLExpression{arguments[1]->clone()}; };
            return (I() - 2.0 * Dot(N(), I()) * N()).release();
        }
        case k_refract_IntrinsicKind: {
            auto I    = [&] { return DSLExpression{arguments[0]->clone()}; };
            auto N    = [&] { return DSLExpression{arguments[1]->clone()}; };
            auto Eta  = [&] { return DSLExpression{arguments[2]->clone()}; };

            std::unique_ptr<Expression> k =
                    (1 - Pow(Eta(), 2) * (1 - Pow(Dot(N(), I()), 2))).release();
            if (!k->is<FloatLiteral>()) {
                return nullptr;
            }
            float kValue = k->as<FloatLiteral>().value();
            return ((kValue < 0) ?
                       (0 * I()) :
                       (Eta() * I() - (Eta() * Dot(N(), I()) + std::sqrt(kValue)) * N())).release();
        }

        // 8.6 : Matrix Functions
        case k_matrixCompMult_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_matrixCompMult);
        case k_transpose_IntrinsicKind: {
            matrix = ConstantFolder::GetConstantValueForVariable(*arguments[0]);
            ExpressionArray array;
            array.reserve_back(returnType.slotCount());
            for (int c = 0; c < returnType.columns(); ++c) {
                for (int r = 0; r < returnType.rows(); ++r) {
                    array.push_back(FloatLiteral::Make(matrix->fOffset, M(r, c),
                                                       &returnType.componentType()));
                }
            }
            return ConstructorCompound::Make(context, matrix->fOffset, returnType,
                                             std::move(array));
        }
        case k_determinant_IntrinsicKind: {
            matrix = ConstantFolder::GetConstantValueForVariable(*arguments[0]);
            float m[16];
            extract_matrix(matrix, m);
            float determinant;
            switch (matrix->type().slotCount()) {
                case 4:
                    determinant = SkInvert2x2Matrix(m, /*outMatrix=*/nullptr);
                    break;
                case 9:
                    determinant = SkInvert3x3Matrix(m, /*outMatrix=*/nullptr);
                    break;
                case 16:
                    determinant = SkInvert4x4Matrix(m, /*outMatrix=*/nullptr);
                    break;
                default:
                    SkDEBUGFAILF("unsupported type %s", matrix->type().description().c_str());
                    return nullptr;
            }
            return FloatLiteral::Make(matrix->fOffset, determinant, &returnType);
        }
        case k_inverse_IntrinsicKind: {
            matrix = ConstantFolder::GetConstantValueForVariable(*arguments[0]);
            float m[16];
            extract_matrix(matrix, m);
            switch (matrix->type().slotCount()) {
                case 4:
                    if (SkInvert2x2Matrix(m, m) == 0.0f) {
                        return nullptr;
                    }
                    return DSLType::Construct(&matrix->type(),
                                              m[0], m[1],
                                              m[2], m[3]).release();
                case 9:
                    if (SkInvert3x3Matrix(m, m) == 0.0f) {
                        return nullptr;
                    }
                    return DSLType::Construct(&matrix->type(),
                                              m[0], m[1], m[2],
                                              m[3], m[4], m[5],
                                              m[6], m[7], m[8]).release();
                    break;
                case 16:
                    if (SkInvert4x4Matrix(m, m) == 0.0f) {
                        return nullptr;
                    }
                    return DSLType::Construct(&matrix->type(),
                                              m[0],  m[1],  m[2],  m[3],
                                              m[4],  m[5],  m[6],  m[7],
                                              m[8],  m[9],  m[10], m[11],
                                              m[12], m[13], m[14], m[15]).release();
            }
            SkDEBUGFAILF("unsupported type %s", matrix->type().description().c_str());
            return nullptr;
        }
        // 8.7 : Vector Relational Functions
        case k_lessThan_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_lessThan);

        case k_lessThanEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_lessThanEqual);

        case k_greaterThan_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_greaterThan);

        case k_greaterThanEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_greaterThanEqual);

        case k_equal_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_equal);

        case k_notEqual_IntrinsicKind:
            return optimize_comparison(context, arguments, Intrinsics::compare_notEqual);

        case k_any_IntrinsicKind:
            return coalesce_vector<bool>(arguments, /*startingState=*/false, returnType,
                                         Intrinsics::coalesce_any,
                                         /*finalize=*/nullptr);
        case k_all_IntrinsicKind:
            return coalesce_vector<bool>(arguments, /*startingState=*/true, returnType,
                                         Intrinsics::coalesce_all,
                                         /*finalize=*/nullptr);
        case k_not_IntrinsicKind:
            return evaluate_intrinsic<bool>(context, arguments, returnType,
                                            Intrinsics::evaluate_not);
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
    // Reject ES3 function calls in strict ES2 mode.
    if (context.fConfig->strictES2Mode() && (function.modifiers().fFlags & Modifiers::kES3_Flag)) {
        context.fErrors->error(offset, "call to '" + function.description() + "' is not supported");
        return nullptr;
    }

    // Reject function calls with the wrong number of arguments.
    if (function.parameters().size() != arguments.size()) {
        String msg = "call to '" + function.name() + "' expected " +
                     to_string((int)function.parameters().size()) + " argument";
        if (function.parameters().size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string(arguments.count());
        context.fErrors->error(offset, msg);
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
        context.fErrors->error(offset, msg);
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
            if (!Analysis::UpdateVariableRefKind(arguments[i].get(), refKind, context.fErrors)) {
                return nullptr;
            }
        }
    }

    switch (function.intrinsicKind()) {
        case k_sample_IntrinsicKind: {
            if (arguments.size() >= 1 && arguments[0]->type().isEffectChild()) {
                // Translate these intrinsic calls into a ChildCall, which simplifies handling in
                // the generators and analysis code
                const Variable& child = *arguments[0]->as<VariableReference>().variable();
                std::rotate(arguments.begin(), arguments.begin() + 1, arguments.end());
                arguments.pop_back();
                return ChildCall::Make(context, offset, returnType, child, std::move(arguments));
            }
            break;
        }
        case k_eval_IntrinsicKind: {
            // Similar, but this is a method call, so the argument ordering is different
            const Variable& child = *arguments.back()->as<VariableReference>().variable();
            arguments.pop_back();
            return ChildCall::Make(context, offset, returnType, child, std::move(arguments));
        }
        default:
            break;
    }

    return Make(context, offset, returnType, function, std::move(arguments));
}

std::unique_ptr<Expression> FunctionCall::Make(const Context& context,
                                               int offset,
                                               const Type* returnType,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments) {
    SkASSERT(function.parameters().size() == arguments.size());

    if (context.fConfig->fSettings.fOptimize) {
        // We might be able to optimize built-in intrinsics.
        if (function.isIntrinsic() && has_compile_time_constant_arguments(arguments)) {
            // The function is an intrinsic and all inputs are compile-time constants. Optimize it.
            if (std::unique_ptr<Expression> expr = optimize_intrinsic_call(context,
                                                                           function.intrinsicKind(),
                                                                           arguments,
                                                                           *returnType)) {
                return expr;
            }
        }
    }

    return std::make_unique<FunctionCall>(offset, returnType, &function, std::move(arguments));
}

}  // namespace SkSL
