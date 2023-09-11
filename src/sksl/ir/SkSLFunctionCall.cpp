/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFunctionCall.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkHalf.h"
#include "src/core/SkMatrixInvert.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLMethodReference.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string_view>

namespace SkSL {

using IntrinsicArguments = std::array<const Expression*, 3>;

static bool has_compile_time_constant_arguments(const ExpressionArray& arguments) {
    for (const std::unique_ptr<Expression>& arg : arguments) {
        const Expression* expr = ConstantFolder::GetConstantValueForVariable(*arg);
        if (!Analysis::IsCompileTimeConstant(*expr)) {
            return false;
        }
    }
    return true;
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

using CoalesceFn = double (*)(double, double, double);
using FinalizeFn = double (*)(double);

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
    // of scalars and vectors, the scalars are interpreted as a vector containing the same value for
    // every component.

    Position pos = arg0->fPosition;
    double minimumValue = returnType.componentType().minimumValue();
    double maximumValue = returnType.componentType().maximumValue();

    const Type& vecType =          arg0->type().isVector()  ? arg0->type() :
                          (arg1 && arg1->type().isVector()) ? arg1->type() :
                                                              arg0->type();
    SkASSERT(         arg0->type().componentType().matches(vecType.componentType()));
    SkASSERT(!arg1 || arg1->type().componentType().matches(vecType.componentType()));

    double value = startingState;
    int arg0Index = 0;
    int arg1Index = 0;
    for (int index = 0; index < vecType.columns(); ++index) {
        std::optional<double> arg0Value = arg0->getConstantValue(arg0Index);
        arg0Index += arg0->type().isVector() ? 1 : 0;
        SkASSERT(arg0Value.has_value());

        std::optional<double> arg1Value = 0.0;
        if (arg1) {
            arg1Value = arg1->getConstantValue(arg1Index);
            arg1Index += arg1->type().isVector() ? 1 : 0;
            SkASSERT(arg1Value.has_value());
        }

        value = coalesce(value, *arg0Value, *arg1Value);

        if (value >= minimumValue && value <= maximumValue) {
            // This result will fit inside the return type.
        } else {
            // The value is outside the float range or is NaN (all if-checks fail); do not optimize.
            return nullptr;
        }
    }

    if (finalize) {
        value = finalize(value);
    }

    return Literal::Make(pos, value, &returnType);
}

template <typename T>
static std::unique_ptr<Expression> coalesce_vector(const IntrinsicArguments& arguments,
                                                   double startingState,
                                                   const Type& returnType,
                                                   CoalesceFn coalesce,
                                                   FinalizeFn finalize) {
    SkASSERT(arguments[0]);
    SkASSERT(!arguments[1]);
    type_check_expression<T>(*arguments[0]);

    return coalesce_n_way_vector(arguments[0], /*arg1=*/nullptr,
                                 startingState, returnType, coalesce, finalize);
}

template <typename T>
static std::unique_ptr<Expression> coalesce_pairwise_vectors(const IntrinsicArguments& arguments,
                                                             double startingState,
                                                             const Type& returnType,
                                                             CoalesceFn coalesce,
                                                             FinalizeFn finalize) {
    SkASSERT(arguments[0]);
    SkASSERT(arguments[1]);
    SkASSERT(!arguments[2]);
    type_check_expression<T>(*arguments[0]);
    type_check_expression<T>(*arguments[1]);

    return coalesce_n_way_vector(arguments[0], arguments[1],
                                 startingState, returnType, coalesce, finalize);
}

using CompareFn = bool (*)(double, double);

static std::unique_ptr<Expression> optimize_comparison(const Context& context,
                                                       const IntrinsicArguments& arguments,
                                                       CompareFn compare) {
    const Expression* left = arguments[0];
    const Expression* right = arguments[1];
    SkASSERT(left);
    SkASSERT(right);
    SkASSERT(!arguments[2]);

    const Type& type = left->type();
    SkASSERT(type.isVector());
    SkASSERT(type.componentType().isScalar());
    SkASSERT(type.matches(right->type()));

    double array[4];

    for (int index = 0; index < type.columns(); ++index) {
        std::optional<double> leftValue = left->getConstantValue(index);
        std::optional<double> rightValue = right->getConstantValue(index);
        SkASSERT(leftValue.has_value());
        SkASSERT(rightValue.has_value());
        array[index] = compare(*leftValue, *rightValue) ? 1.0 : 0.0;
    }

    const Type& bvecType = context.fTypes.fBool->toCompound(context, type.columns(), /*rows=*/1);
    return ConstructorCompound::MakeFromConstants(context, left->fPosition, bvecType, array);
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

    double minimumValue = returnType.componentType().minimumValue();
    double maximumValue = returnType.componentType().maximumValue();
    int slots = returnType.slotCount();
    double array[16];

    int arg0Index = 0;
    int arg1Index = 0;
    int arg2Index = 0;
    for (int index = 0; index < slots; ++index) {
        std::optional<double> arg0Value = arg0->getConstantValue(arg0Index);
        arg0Index += arg0->type().isScalar() ? 0 : 1;
        SkASSERT(arg0Value.has_value());

        std::optional<double> arg1Value = 0.0;
        if (arg1) {
            arg1Value = arg1->getConstantValue(arg1Index);
            arg1Index += arg1->type().isScalar() ? 0 : 1;
            SkASSERT(arg1Value.has_value());
        }

        std::optional<double> arg2Value = 0.0;
        if (arg2) {
            arg2Value = arg2->getConstantValue(arg2Index);
            arg2Index += arg2->type().isScalar() ? 0 : 1;
            SkASSERT(arg2Value.has_value());
        }

        array[index] = eval(*arg0Value, *arg1Value, *arg2Value);

        if (array[index] >= minimumValue && array[index] <= maximumValue) {
            // This result will fit inside the return type.
        } else {
            // The value is outside the float range or is NaN (all if-checks fail); do not optimize.
            return nullptr;
        }
    }

    return ConstructorCompound::MakeFromConstants(context, arg0->fPosition, returnType, array);
}

template <typename T>
static std::unique_ptr<Expression> evaluate_intrinsic(const Context& context,
                                                      const IntrinsicArguments& arguments,
                                                      const Type& returnType,
                                                      EvaluateFn eval) {
    SkASSERT(arguments[0]);
    SkASSERT(!arguments[1]);
    type_check_expression<T>(*arguments[0]);

    return evaluate_n_way_intrinsic(context, arguments[0], /*arg1=*/nullptr, /*arg2=*/nullptr,
                                    returnType, eval);
}

static std::unique_ptr<Expression> evaluate_intrinsic_numeric(const Context& context,
                                                              const IntrinsicArguments& arguments,
                                                              const Type& returnType,
                                                              EvaluateFn eval) {
    SkASSERT(arguments[0]);
    SkASSERT(!arguments[1]);
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
                                                               const IntrinsicArguments& arguments,
                                                               const Type& returnType,
                                                               EvaluateFn eval) {
    SkASSERT(arguments[0]);
    SkASSERT(arguments[1]);
    SkASSERT(!arguments[2]);
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

    return evaluate_n_way_intrinsic(context, arguments[0], arguments[1], /*arg2=*/nullptr,
                                    returnType, eval);
}

static std::unique_ptr<Expression> evaluate_3_way_intrinsic(const Context& context,
                                                            const IntrinsicArguments& arguments,
                                                            const Type& returnType,
                                                            EvaluateFn eval) {
    SkASSERT(arguments[0]);
    SkASSERT(arguments[1]);
    SkASSERT(arguments[2]);
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

    return evaluate_n_way_intrinsic(context, arguments[0], arguments[1], arguments[2],
                                    returnType, eval);
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

double evaluate_add(double a, double b, double)        { return a + b; }
double evaluate_sub(double a, double b, double)        { return a - b; }
double evaluate_mul(double a, double b, double)        { return a * b; }
double evaluate_div(double a, double b, double)        { return sk_ieee_double_divide(a, b); }
double evaluate_abs(double a, double, double)          { return std::abs(a); }
double evaluate_sign(double a, double, double)         { return (a > 0) - (a < 0); }
double evaluate_opposite_sign(double a,double, double) { return (a < 0) - (a > 0); }
double evaluate_floor(double a, double, double)        { return std::floor(a); }
double evaluate_ceil(double a, double, double)         { return std::ceil(a); }
double evaluate_fract(double a, double, double)        { return a - std::floor(a); }
double evaluate_min(double a, double b, double)        { return (a < b) ? a : b; }
double evaluate_max(double a, double b, double)        { return (a > b) ? a : b; }
double evaluate_clamp(double x, double l, double h)    { return (x < l) ? l : (x > h) ? h : x; }
double evaluate_fma(double a, double b, double c)      { return a * b + c; }
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

std::unique_ptr<Expression> evaluate_length(const IntrinsicArguments& arguments) {
    return coalesce_vector<float>(arguments, /*startingState=*/0,
                                  arguments[0]->type().componentType(),
                                  coalesce_length,
                                  finalize_length);
}

std::unique_ptr<Expression> evaluate_distance(const IntrinsicArguments& arguments) {
    return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0,
                                            arguments[0]->type().componentType(),
                                            coalesce_distance,
                                            finalize_distance);
}
std::unique_ptr<Expression> evaluate_dot(const IntrinsicArguments& arguments) {
    return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0,
                                            arguments[0]->type().componentType(),
                                            coalesce_dot,
                                            /*finalize=*/nullptr);
}

std::unique_ptr<Expression> evaluate_sign(const Context& context,
                                          const IntrinsicArguments& arguments) {
    return evaluate_intrinsic_numeric(context, arguments, arguments[0]->type(),
                                      evaluate_sign);
}

std::unique_ptr<Expression> evaluate_opposite_sign(const Context& context,
                                                   const IntrinsicArguments& arguments) {
    return evaluate_intrinsic_numeric(context, arguments, arguments[0]->type(),
                                      evaluate_opposite_sign);
}

std::unique_ptr<Expression> evaluate_add(const Context& context,
                                         const IntrinsicArguments& arguments) {
    return evaluate_pairwise_intrinsic(context, arguments, arguments[0]->type(),
                                       evaluate_add);
}

std::unique_ptr<Expression> evaluate_sub(const Context& context,
                                         const IntrinsicArguments& arguments) {
    return evaluate_pairwise_intrinsic(context, arguments, arguments[0]->type(),
                                       evaluate_sub);
}

std::unique_ptr<Expression> evaluate_mul(const Context& context,
                                         const IntrinsicArguments& arguments) {
    return evaluate_pairwise_intrinsic(context, arguments, arguments[0]->type(),
                                       evaluate_mul);
}

std::unique_ptr<Expression> evaluate_div(const Context& context,
                                         const IntrinsicArguments& arguments) {
    return evaluate_pairwise_intrinsic(context, arguments, arguments[0]->type(),
                                       evaluate_div);
}

std::unique_ptr<Expression> evaluate_normalize(const Context& context,
                                               const IntrinsicArguments& arguments) {
    // normalize(v): v / length(v)
    std::unique_ptr<Expression> length = Intrinsics::evaluate_length(arguments);
    if (!length) { return nullptr; }

    const IntrinsicArguments divArgs = {arguments[0], length.get(), nullptr};
    return Intrinsics::evaluate_div(context, divArgs);
}

std::unique_ptr<Expression> evaluate_faceforward(const Context& context,
                                                 const IntrinsicArguments& arguments) {
    const Expression* N = arguments[0];     // vector
    const Expression* I = arguments[1];     // vector
    const Expression* NRef = arguments[2];  // vector

    // faceforward(N,I,NRef): N * -sign(dot(I, NRef))
    const IntrinsicArguments dotArgs = {I, NRef, nullptr};
    std::unique_ptr<Expression> dotExpr = Intrinsics::evaluate_dot(dotArgs);
    if (!dotExpr) { return nullptr; }

    const IntrinsicArguments signArgs = {dotExpr.get(), nullptr, nullptr};
    std::unique_ptr<Expression> signExpr = Intrinsics::evaluate_opposite_sign(context, signArgs);
    if (!signExpr) { return nullptr; }

    const IntrinsicArguments mulArgs = {N, signExpr.get(), nullptr};
    return Intrinsics::evaluate_mul(context, mulArgs);
}

std::unique_ptr<Expression> evaluate_reflect(const Context& context,
                                             const IntrinsicArguments& arguments) {
    const Expression* I = arguments[0];  // vector
    const Expression* N = arguments[1];  // vector

    // reflect(I,N): temp = (N * dot(N, I)); reflect = I - (temp + temp)
    const IntrinsicArguments dotArgs = {N, I, nullptr};
    std::unique_ptr<Expression> dotExpr = Intrinsics::evaluate_dot(dotArgs);
    if (!dotExpr) { return nullptr; }

    const IntrinsicArguments mulArgs = {N, dotExpr.get(), nullptr};
    std::unique_ptr<Expression> mulExpr = Intrinsics::evaluate_mul(context, mulArgs);
    if (!mulExpr) { return nullptr; }

    const IntrinsicArguments addArgs = {mulExpr.get(), mulExpr.get(), nullptr};
    std::unique_ptr<Expression> addExpr = Intrinsics::evaluate_add(context, addArgs);
    if (!addExpr) { return nullptr; }

    const IntrinsicArguments subArgs = {I, addExpr.get(), nullptr};
    return Intrinsics::evaluate_sub(context, subArgs);
}

std::unique_ptr<Expression> evaluate_refract(const Context& context,
                                             const IntrinsicArguments& arguments) {
    const Expression* I = arguments[0];    // vector
    const Expression* N = arguments[1];    // vector
    const Expression* Eta = arguments[2];  // scalar

    // K = 1.0 - Eta^2 * (1.0 - Dot(N, I)^2);

    // DotNI = Dot(N, I)
    const IntrinsicArguments DotNIArgs = {N, I, nullptr};
    std::unique_ptr<Expression> DotNIExpr = Intrinsics::evaluate_dot(DotNIArgs);
    if (!DotNIExpr) { return nullptr; }

    // DotNI2 = DotNI * DotNI
    const IntrinsicArguments DotNI2Args = {DotNIExpr.get(), DotNIExpr.get(), nullptr};
    std::unique_ptr<Expression> DotNI2Expr = Intrinsics::evaluate_mul(context, DotNI2Args);
    if (!DotNI2Expr) { return nullptr; }

    // OneMinusDot = 1 - DotNI2
    Literal oneLiteral{Position{}, 1.0, &DotNI2Expr->type()};
    const IntrinsicArguments OneMinusDotArgs = {&oneLiteral, DotNI2Expr.get(), nullptr};
    std::unique_ptr<Expression> OneMinusDotExpr= Intrinsics::evaluate_sub(context, OneMinusDotArgs);
    if (!OneMinusDotExpr) { return nullptr; }

    // Eta2 = Eta * Eta
    const IntrinsicArguments Eta2Args = {Eta, Eta, nullptr};
    std::unique_ptr<Expression> Eta2Expr = Intrinsics::evaluate_mul(context, Eta2Args);
    if (!Eta2Expr) { return nullptr; }

    // Eta2xDot = Eta2 * OneMinusDot
    const IntrinsicArguments Eta2xDotArgs = {Eta2Expr.get(), OneMinusDotExpr.get(), nullptr};
    std::unique_ptr<Expression> Eta2xDotExpr = Intrinsics::evaluate_mul(context, Eta2xDotArgs);
    if (!Eta2xDotExpr) { return nullptr; }

    // K = 1.0 - Eta2xDot
    const IntrinsicArguments KArgs = {&oneLiteral, Eta2xDotExpr.get(), nullptr};
    std::unique_ptr<Expression> KExpr = Intrinsics::evaluate_sub(context, KArgs);
    if (!KExpr || !KExpr->is<Literal>()) { return nullptr; }

    // When K < 0, Refract(I, N, Eta) = vec(0)
    double kValue = KExpr->as<Literal>().value();
    if (kValue < 0) {
        constexpr double kZero[4] = {};
        return ConstructorCompound::MakeFromConstants(context, I->fPosition, I->type(), kZero);
    }

    // When K ≥ 0, Refract(I, N, Eta) = (I * Eta) - N * (Eta * Dot(N,I) + Sqrt(K))

    // EtaDot = Eta * DotNI
    const IntrinsicArguments EtaDotArgs = {Eta, DotNIExpr.get(), nullptr};
    std::unique_ptr<Expression> EtaDotExpr = Intrinsics::evaluate_mul(context, EtaDotArgs);
    if (!EtaDotExpr) { return nullptr; }

    // EtaDotSqrt = EtaDot + Sqrt(K)
    Literal sqrtKLiteral{Position{}, std::sqrt(kValue), &Eta->type()};
    const IntrinsicArguments EtaDotSqrtArgs = {EtaDotExpr.get(), &sqrtKLiteral, nullptr};
    std::unique_ptr<Expression> EtaDotSqrtExpr = Intrinsics::evaluate_add(context, EtaDotSqrtArgs);
    if (!EtaDotSqrtExpr) { return nullptr; }

    // NxEDS = N * EtaDotSqrt
    const IntrinsicArguments NxEDSArgs = {N, EtaDotSqrtExpr.get(), nullptr};
    std::unique_ptr<Expression> NxEDSExpr = Intrinsics::evaluate_mul(context, NxEDSArgs);
    if (!NxEDSExpr) { return nullptr; }

    // IEta = I * Eta
    const IntrinsicArguments IEtaArgs = {I, Eta, nullptr};
    std::unique_ptr<Expression> IEtaExpr = Intrinsics::evaluate_mul(context, IEtaArgs);
    if (!IEtaExpr) { return nullptr; }

    // Refract = IEta - NxEDS
    const IntrinsicArguments RefractArgs = {IEtaExpr.get(), NxEDSExpr.get(), nullptr};
    return Intrinsics::evaluate_sub(context, RefractArgs);
}

}  // namespace
}  // namespace Intrinsics

static void extract_matrix(const Expression* expr, float mat[16]) {
    size_t numSlots = expr->type().slotCount();
    for (size_t index = 0; index < numSlots; ++index) {
        mat[index] = *expr->getConstantValue(index);
    }
}

static std::unique_ptr<Expression> optimize_intrinsic_call(const Context& context,
                                                           Position pos,
                                                           IntrinsicKind intrinsic,
                                                           const ExpressionArray& argArray,
                                                           const Type& returnType) {
    // Replace constant variables with their literal values.
    IntrinsicArguments arguments = {};
    SkASSERT(SkToSizeT(argArray.size()) <= arguments.size());
    for (int index = 0; index < argArray.size(); ++index) {
        arguments[index] = ConstantFolder::GetConstantValueForVariable(*argArray[index]);
    }

    auto Get = [&](int idx, int col) -> float {
        return *arguments[idx]->getConstantValue(col);
    };

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
            if (argArray.size() == 1) {
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
            return Intrinsics::evaluate_sign(context, arguments);

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
        case k_fma_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments, returnType,
                                            Intrinsics::evaluate_fma);
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
                return evaluate_n_way_intrinsic(context, arguments[0], arguments[1], arguments[2],
                                                returnType, Intrinsics::evaluate_mix);
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
        // 8.4 : Floating-Point Pack and Unpack Functions
        case k_packUnorm2x16_IntrinsicKind: {
            auto Pack = [&](int n) -> unsigned int {
                float x = Get(0, n);
                return (int)std::round(Intrinsics::evaluate_clamp(x, 0.0, 1.0) * 65535.0);
            };
            const double packed = ((Pack(0) << 0)  & 0x0000FFFF) |
                                  ((Pack(1) << 16) & 0xFFFF0000);
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fUInt, &packed);
        }
        case k_packSnorm2x16_IntrinsicKind: {
            auto Pack = [&](int n) -> unsigned int {
                float x = Get(0, n);
                return (int)std::round(Intrinsics::evaluate_clamp(x, -1.0, 1.0) * 32767.0);
            };
            const double packed = ((Pack(0) << 0)  & 0x0000FFFF) |
                                  ((Pack(1) << 16) & 0xFFFF0000);
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fUInt, &packed);
        }
        case k_packHalf2x16_IntrinsicKind: {
            auto Pack = [&](int n) -> unsigned int {
                return SkFloatToHalf(Get(0, n));
            };
            const double packed = ((Pack(0) << 0)  & 0x0000FFFF) |
                                  ((Pack(1) << 16) & 0xFFFF0000);
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fUInt, &packed);
        }
        case k_unpackUnorm2x16_IntrinsicKind: {
            SKSL_INT x = *arguments[0]->getConstantValue(0);
            uint16_t a = ((x >> 0)  & 0x0000FFFF);
            uint16_t b = ((x >> 16) & 0x0000FFFF);
            const double unpacked[2] = {double(a) / 65535.0,
                                        double(b) / 65535.0};
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fFloat2, unpacked);
        }
        case k_unpackSnorm2x16_IntrinsicKind: {
            SKSL_INT x = *arguments[0]->getConstantValue(0);
            int16_t a = ((x >> 0)  & 0x0000FFFF);
            int16_t b = ((x >> 16) & 0x0000FFFF);
            const double unpacked[2] = {Intrinsics::evaluate_clamp(double(a) / 32767.0, -1.0, 1.0),
                                        Intrinsics::evaluate_clamp(double(b) / 32767.0, -1.0, 1.0)};
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fFloat2, unpacked);
        }
        case k_unpackHalf2x16_IntrinsicKind: {
            SKSL_INT x = *arguments[0]->getConstantValue(0);
            uint16_t a = ((x >> 0)  & 0x0000FFFF);
            uint16_t b = ((x >> 16) & 0x0000FFFF);
            const double unpacked[2] = {SkHalfToFloat(a),
                                        SkHalfToFloat(b)};
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          *context.fTypes.fFloat2, unpacked);
        }
        // 8.5 : Geometric Functions
        case k_length_IntrinsicKind:
            return Intrinsics::evaluate_length(arguments);

        case k_distance_IntrinsicKind:
            return Intrinsics::evaluate_distance(arguments);

        case k_dot_IntrinsicKind:
            return Intrinsics::evaluate_dot(arguments);

        case k_cross_IntrinsicKind: {
            auto X = [&](int n) -> float { return Get(0, n); };
            auto Y = [&](int n) -> float { return Get(1, n); };
            SkASSERT(arguments[0]->type().columns() == 3);  // the vec2 form is not a real intrinsic

            double vec[3] = {X(1) * Y(2) - Y(1) * X(2),
                             X(2) * Y(0) - Y(2) * X(0),
                             X(0) * Y(1) - Y(0) * X(1)};
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          returnType, vec);
        }
        case k_normalize_IntrinsicKind:
            return Intrinsics::evaluate_normalize(context, arguments);

        case k_faceforward_IntrinsicKind:
            return Intrinsics::evaluate_faceforward(context, arguments);

        case k_reflect_IntrinsicKind:
            return Intrinsics::evaluate_reflect(context, arguments);

        case k_refract_IntrinsicKind:
            return Intrinsics::evaluate_refract(context, arguments);

        // 8.6 : Matrix Functions
        case k_matrixCompMult_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments, returnType,
                                               Intrinsics::evaluate_matrixCompMult);
        case k_transpose_IntrinsicKind: {
            double mat[16];
            int index = 0;
            for (int c = 0; c < returnType.columns(); ++c) {
                for (int r = 0; r < returnType.rows(); ++r) {
                    mat[index++] = Get(0, (returnType.columns() * r) + c);
                }
            }
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          returnType, mat);
        }
        case k_outerProduct_IntrinsicKind: {
            double mat[16];
            int index = 0;
            for (int c = 0; c < returnType.columns(); ++c) {
                for (int r = 0; r < returnType.rows(); ++r) {
                    mat[index++] = Get(0, r) * Get(1, c);
                }
            }
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                          returnType, mat);
        }
        case k_determinant_IntrinsicKind: {
            float mat[16];
            extract_matrix(arguments[0], mat);
            float determinant;
            switch (arguments[0]->type().slotCount()) {
                case 4:
                    determinant = SkInvert2x2Matrix(mat, /*outMatrix=*/nullptr);
                    break;
                case 9:
                    determinant = SkInvert3x3Matrix(mat, /*outMatrix=*/nullptr);
                    break;
                case 16:
                    determinant = SkInvert4x4Matrix(mat, /*outMatrix=*/nullptr);
                    break;
                default:
                    SkDEBUGFAILF("unsupported type %s", arguments[0]->type().description().c_str());
                    return nullptr;
            }
            return Literal::MakeFloat(arguments[0]->fPosition, determinant, &returnType);
        }
        case k_inverse_IntrinsicKind: {
            float mat[16] = {};
            extract_matrix(arguments[0], mat);
            switch (arguments[0]->type().slotCount()) {
                case 4:
                    if (SkInvert2x2Matrix(mat, mat) == 0.0f) {
                        return nullptr;
                    }
                    break;
                case 9:
                    if (SkInvert3x3Matrix(mat, mat) == 0.0f) {
                        return nullptr;
                    }
                    break;
                case 16:
                    if (SkInvert4x4Matrix(mat, mat) == 0.0f) {
                        return nullptr;
                    }
                    break;
                default:
                    SkDEBUGFAILF("unsupported type %s", arguments[0]->type().description().c_str());
                    return nullptr;
            }

            double dmat[16];
            std::copy(mat, mat + std::size(mat), dmat);
            return ConstructorCompound::MakeFromConstants(context, arguments[0]->fPosition,
                                                         returnType, dmat);
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

std::unique_ptr<Expression> FunctionCall::clone(Position pos) const {
    return std::make_unique<FunctionCall>(pos, &this->type(), &this->function(),
                                          this->arguments().clone());
}

std::string FunctionCall::description(OperatorPrecedence) const {
    std::string result = std::string(this->function().name()) + "(";
    auto separator = SkSL::String::Separator();
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += separator();
        result += arg->description(OperatorPrecedence::kSequence);
    }
    result += ")";
    return result;
}

static bool argument_and_parameter_flags_match(const Expression& argument,
                                               const Variable& parameter) {
    // If the function parameter has a pixel format, the argument being passed in must have a
    // matching pixel format.
    LayoutFlags paramPixelFormat = parameter.layout().fFlags & LayoutFlag::kAllPixelFormats;
    if (paramPixelFormat != LayoutFlag::kNone) {
        // The only SkSL type that supports pixel-format qualifiers is a storage texture.
        if (parameter.type().isStorageTexture()) {
            // Storage textures are opaquely typed, so there's no way to specify one other than by
            // directly accessing a variable.
            if (!argument.is<VariableReference>()) {
                return false;
            }

            // The variable's pixel-format flags must match. (Only one pixel-format bit can be set.)
            const Variable& var = *argument.as<VariableReference>().variable();
            if ((var.layout().fFlags & LayoutFlag::kAllPixelFormats) != paramPixelFormat) {
                return false;
            }
        }
    }

    // The only other supported parameter flags are `const` and `in/out`, which do not allow
    // multiple overloads.
    return true;
}

/**
 * Used to determine the best overload for a function call by calculating the cost of coercing the
 * arguments of the function to the required types. Cost has no particular meaning other than "lower
 * costs are preferred". Returns CoercionCost::Impossible() if the call is not valid. This is never
 * called for functions with only one definition.
 */
static CoercionCost call_cost(const Context& context,
                              const FunctionDeclaration& function,
                              const ExpressionArray& arguments) {
    // Strict-ES2 programs can never call an `$es3` function.
    if (context.fConfig->strictES2Mode() && function.modifierFlags().isES3()) {
        return CoercionCost::Impossible();
    }
    // Functions with the wrong number of parameters are never a match.
    if (function.parameters().size() != SkToSizeT(arguments.size())) {
        return CoercionCost::Impossible();
    }
    // If the arguments cannot be coerced to the parameter types, the function is never a match.
    FunctionDeclaration::ParamTypes types;
    const Type* ignored;
    if (!function.determineFinalTypes(arguments, &types, &ignored)) {
        return CoercionCost::Impossible();
    }
    // If the arguments do not match the parameter types due to mismatched modifiers, the function
    // is never a match.
    for (int i = 0; i < arguments.size(); i++) {
        const Expression& arg = *arguments[i];
        const Variable& param = *function.parameters()[i];
        if (!argument_and_parameter_flags_match(arg, param)) {
            return CoercionCost::Impossible();
        }
    }
    // Return the sum of coercion costs of each argument.
    CoercionCost total = CoercionCost::Free();
    for (int i = 0; i < arguments.size(); i++) {
        total = total + arguments[i]->coercionCost(*types[i]);
    }
    return total;
}

const FunctionDeclaration* FunctionCall::FindBestFunctionForCall(
        const Context& context,
        const FunctionDeclaration* overloadChain,
        const ExpressionArray& arguments) {
    if (!overloadChain->nextOverload()) {
        return overloadChain;
    }
    CoercionCost bestCost = CoercionCost::Impossible();
    const FunctionDeclaration* best = nullptr;
    for (const FunctionDeclaration* f = overloadChain; f != nullptr; f = f->nextOverload()) {
        CoercionCost cost = call_cost(context, *f, arguments);
        if (cost <= bestCost) {
            bestCost = cost;
            best = f;
        }
    }
    return bestCost.fImpossible ? nullptr : best;
}

static std::string build_argument_type_list(SkSpan<const std::unique_ptr<Expression>> arguments) {
    std::string result = "(";
    auto separator = SkSL::String::Separator();
    for (const std::unique_ptr<Expression>& arg : arguments) {
        result += separator();
        result += arg->type().displayName();
    }
    return result + ")";
}

std::unique_ptr<Expression> FunctionCall::Convert(const Context& context,
                                                  Position pos,
                                                  std::unique_ptr<Expression> functionValue,
                                                  ExpressionArray arguments) {
    switch (functionValue->kind()) {
        case Expression::Kind::kTypeReference:
            return Constructor::Convert(context,
                                        pos,
                                        functionValue->as<TypeReference>().value(),
                                        std::move(arguments));
        case Expression::Kind::kFunctionReference: {
            const FunctionReference& ref = functionValue->as<FunctionReference>();
            const FunctionDeclaration* best = FindBestFunctionForCall(context, ref.overloadChain(),
                                                                      arguments);
            if (best) {
                return FunctionCall::Convert(context, pos, *best, std::move(arguments));
            }
            std::string msg = "no match for " + std::string(ref.overloadChain()->name()) +
                              build_argument_type_list(arguments);
            context.fErrors->error(pos, msg);
            return nullptr;
        }
        case Expression::Kind::kMethodReference: {
            MethodReference& ref = functionValue->as<MethodReference>();
            arguments.push_back(std::move(ref.self()));

            const FunctionDeclaration* best = FindBestFunctionForCall(context, ref.overloadChain(),
                                                                      arguments);
            if (best) {
                return FunctionCall::Convert(context, pos, *best, std::move(arguments));
            }
            std::string msg =
                    "no match for " + arguments.back()->type().displayName() +
                    "::" + std::string(ref.overloadChain()->name().substr(1)) +
                    build_argument_type_list(SkSpan(arguments).first(arguments.size() - 1));
            context.fErrors->error(pos, msg);
            return nullptr;
        }
        case Expression::Kind::kPoison:
            functionValue->fPosition = pos;
            return functionValue;
        default:
            context.fErrors->error(pos, "not a function");
            return nullptr;
    }
}

std::unique_ptr<Expression> FunctionCall::Convert(const Context& context,
                                                  Position pos,
                                                  const FunctionDeclaration& function,
                                                  ExpressionArray arguments) {
    // Reject ES3 function calls in strict ES2 mode.
    if (context.fConfig->strictES2Mode() && function.modifierFlags().isES3()) {
        context.fErrors->error(pos, "call to '" + function.description() + "' is not supported");
        return nullptr;
    }

    // Reject function calls with the wrong number of arguments.
    if (function.parameters().size() != SkToSizeT(arguments.size())) {
        std::string msg = "call to '" + std::string(function.name()) + "' expected " +
                          std::to_string(function.parameters().size()) + " argument";
        if (function.parameters().size() != 1) {
            msg += "s";
        }
        msg += ", but found " + std::to_string(arguments.size());
        context.fErrors->error(pos, msg);
        return nullptr;
    }

    // If the arguments do not match the parameter types due to mismatched modifiers, reject the
    // function call.
    for (int i = 0; i < arguments.size(); i++) {
        const Expression& arg = *arguments[i];
        const Variable& param = *function.parameters()[i];
        if (!argument_and_parameter_flags_match(arg, param)) {
            context.fErrors->error(arg.position(), "expected argument of type '" +
                                                   param.layout().paddedDescription() +
                                                   param.modifierFlags().paddedDescription() +
                                                   param.type().description() + "'");
            return nullptr;
        }
    }

    // Resolve generic types.
    FunctionDeclaration::ParamTypes types;
    const Type* returnType;
    if (!function.determineFinalTypes(arguments, &types, &returnType)) {
        std::string msg = "no match for " + std::string(function.name()) +
                          build_argument_type_list(arguments);
        context.fErrors->error(pos, msg);
        return nullptr;
    }

    for (int i = 0; i < arguments.size(); i++) {
        // Coerce each argument to the proper type.
        arguments[i] = types[i]->coerceExpression(std::move(arguments[i]), context);
        if (!arguments[i]) {
            return nullptr;
        }
        // Update the refKind on out-parameters, and ensure that they are actually assignable.
        ModifierFlags paramFlags = function.parameters()[i]->modifierFlags();
        if (paramFlags & ModifierFlag::kOut) {
            const VariableRefKind refKind = (paramFlags & ModifierFlag::kIn)
                                                    ? VariableReference::RefKind::kReadWrite
                                                    : VariableReference::RefKind::kPointer;
            if (!Analysis::UpdateVariableRefKind(arguments[i].get(), refKind, context.fErrors)) {
                return nullptr;
            }
        }
    }

    if (function.isMain()) {
        context.fErrors->error(pos, "call to 'main' is not allowed");
        return nullptr;
    }

    if (function.intrinsicKind() == k_eval_IntrinsicKind) {
        // This is a method call on an effect child. Translate it into a ChildCall, which simplifies
        // handling in the generators and analysis code.
        const Variable& child = *arguments.back()->as<VariableReference>().variable();
        arguments.pop_back();
        return ChildCall::Make(context, pos, returnType, child, std::move(arguments));
    }

    return Make(context, pos, returnType, function, std::move(arguments));
}

std::unique_ptr<Expression> FunctionCall::Make(const Context& context,
                                               Position pos,
                                               const Type* returnType,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments) {
    SkASSERT(function.parameters().size() == SkToSizeT(arguments.size()));

    // We might be able to optimize built-in intrinsics.
    if (function.isIntrinsic() && has_compile_time_constant_arguments(arguments)) {
        // The function is an intrinsic and all inputs are compile-time constants. Optimize it.
        if (std::unique_ptr<Expression> expr = optimize_intrinsic_call(context,
                                                                       pos,
                                                                       function.intrinsicKind(),
                                                                       arguments,
                                                                       *returnType)) {
            expr->fPosition = pos;
            return expr;
        }
    }

    return std::make_unique<FunctionCall>(pos, returnType, &function, std::move(arguments));
}

}  // namespace SkSL
