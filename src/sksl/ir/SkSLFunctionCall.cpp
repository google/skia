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
[[maybe_unused]] void type_check_expression<SKSL_INT>(const Expression& expr) {
    SkASSERT(expr.type().componentType().isInteger());
}

template <>
void type_check_expression<bool>(const Expression& expr) {
    SkASSERT(expr.type().componentType().isBoolean());
}

template <typename T>
static std::unique_ptr<Expression> make_literal(int offset, double value, const Type* type) {
    return Literal<T>::Make(offset, (T)value, type);
}

using CoalesceFn = double (*)(double, double, double);
using FinalizeFn = double (*)(double);
using MakeLiteralFn = std::unique_ptr<Expression> (*)(int, double, const Type*);

static std::unique_ptr<Expression> coalesce_n_way_vector(const Expression* arg0,
                                                         const Expression* arg1,
                                                         double startingState,
                                                         CoalesceFn coalesce,
                                                         FinalizeFn finalize,
                                                         MakeLiteralFn makeLiteral) {
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

    return makeLiteral(offset, value, &arg0->type().componentType());
}

template <typename T>
static std::unique_ptr<Expression> coalesce_vector(const ExpressionArray& arguments,
                                                   double startingState,
                                                   CoalesceFn coalesce,
                                                   FinalizeFn finalize) {
    SkASSERT(arguments.size() == 1);
    type_check_expression<T>(*arguments[0]);

    return coalesce_n_way_vector(arguments[0].get(), /*arg1=*/nullptr,
                                 (double)startingState, coalesce, finalize, make_literal<T>);
}

template <typename T>
static std::unique_ptr<Expression> coalesce_pairwise_vectors(const ExpressionArray& arguments,
                                                             double startingState,
                                                             CoalesceFn coalesce,
                                                             FinalizeFn finalize) {
    SkASSERT(arguments.size() == 2);
    type_check_expression<T>(*arguments[0]);
    type_check_expression<T>(*arguments[1]);

    return coalesce_n_way_vector(arguments[0].get(), arguments[1].get(),
                                 (double)startingState, coalesce, finalize, make_literal<T>);
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

template <typename T0, typename T1 = T0, typename T2 = T0>
static std::unique_ptr<Expression> evaluate_n_way_intrinsic_of_type(
        const Context& context,
        const Expression* arg0,
        const Expression* arg1,
        const Expression* arg2,
        const std::function<T0(T0, T1, T2)>& eval) {
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

    const Type& compoundType =          !arg0->type().isScalar()  ? arg0->type() :
                               (arg1 && !arg1->type().isScalar()) ? arg1->type() :
                               (arg2 && !arg2->type().isScalar()) ? arg2->type() :
                                                                    arg0->type();
    const Type& type = compoundType.componentType();
    SkASSERT(arg0->type().componentType() == type);

    if (arg1) {
        arg1 = ConstantFolder::GetConstantValueForVariable(*arg1);
        SkASSERT(arg1);
    }

    if (arg2) {
        arg2 = ConstantFolder::GetConstantValueForVariable(*arg2);
        SkASSERT(arg2);
    }

    ExpressionArray array;
    array.reserve_back(compoundType.columns());

    int arg0Index = 0;
    int arg1Index = 0;
    int arg2Index = 0;
    int slots = compoundType.slotCount();
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

        T0 value = eval(arg0Subexpr->as<Literal<T0>>().value(),
                        arg1Subexpr ? arg1Subexpr->as<Literal<T1>>().value() : T1{},
                        arg2Subexpr ? arg2Subexpr->as<Literal<T2>>().value() : T2{});

        if constexpr (std::is_floating_point<T0>::value) {
            // If evaluation of the intrinsic yields a non-finite value, do not optimize.
            if (!std::isfinite(value)) {
                return nullptr;
            }
        }

        array.push_back(Literal<T0>::Make(arg0Subexpr->fOffset, value, &type));
    }

    return ConstructorCompound::Make(context, arg0->fOffset, compoundType, std::move(array));
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

    return evaluate_n_way_intrinsic_of_type<T, T, T>(
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
        return evaluate_n_way_intrinsic_of_type<float, float, float>(
                context, arguments[0].get(), arguments[1].get(), /*arg2=*/nullptr,
                [&eval](float a, float b, float) { return eval(a, b); });
    }
    if (type.isInteger()) {
        return evaluate_n_way_intrinsic_of_type<SKSL_INT, SKSL_INT, SKSL_INT>(
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
        return evaluate_n_way_intrinsic_of_type<float, float, float>(
                context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
    }
    if (type.isInteger()) {
        return evaluate_n_way_intrinsic_of_type<SKSL_INT, SKSL_INT, SKSL_INT>(
                context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
    }

    SkDEBUGFAILF("unsupported type %s", type.description().c_str());
    return nullptr;
}

// Helper functions for optimizing all of our intrinsics.
namespace Intrinsics {

double coalesce_length(double a, double b, double)     { return a + (b * b); }
double finalize_length(double a)                       { return std::sqrt(a); }

double coalesce_distance(double a, double b, double c) { b -= c; return a + (b * b); }
double finalize_distance(double a)                     { return std::sqrt(a); }

double coalesce_dot(double a, double b, double c)      { return a + (b * c); }
double coalesce_any(double a, double b, double)        { return a || b; }
double coalesce_all(double a, double b, double)        { return a && b; }

bool compare_lessThan(double a, double b)         { return a < b; }
bool compare_lessThanEqual(double a, double b)    { return a <= b; }
bool compare_greaterThan(double a, double b)      { return a > b; }
bool compare_greaterThanEqual(double a, double b) { return a >= b; }
bool compare_equal(double a, double b)            { return a == b; }
bool compare_notEqual(double a, double b)         { return a != b; }

}

static std::unique_ptr<Expression> optimize_intrinsic_call(const Context& context,
                                                           IntrinsicKind intrinsic,
                                                           const ExpressionArray& arguments) {
    using namespace SkSL::dsl;
    switch (intrinsic) {
        // 8.1 : Angle and Trigonometry Functions
        case k_radians_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a * 0.0174532925; });
        case k_degrees_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a * 57.2957795; });
        case k_sin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::sin(a); });
        case k_cos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::cos(a); });
        case k_tan_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::tan(a); });
        case k_asin_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::asin(a); });
        case k_acos_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::acos(a); });
        case k_atan_IntrinsicKind:
            if (arguments.size() == 1) {
                return evaluate_intrinsic<float>(
                        context, arguments, [](float a) { return std::atan(a); });
            } else {
                SkASSERT(arguments.size() == 2);
                return evaluate_pairwise_intrinsic(
                        context, arguments, [](auto a, auto b) { return std::atan2(a, b); });
            }

        // 8.2 : Exponential Functions
        case k_pow_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto x, auto y) { return std::pow(x, y); });
        case k_exp_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::exp(a); });
        case k_log_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::log(a); });
        case k_exp2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::exp2(a); });
        case k_log2_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::log2(a); });
        case k_sqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::sqrt(a); });
        case k_inversesqrt_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return 1 / std::sqrt(a); });

        // 8.3 : Common Functions
        case k_abs_IntrinsicKind:
            return evaluate_intrinsic_numeric(context, arguments,
                                              [](auto a) { return std::abs(a); });
        case k_sign_IntrinsicKind:
            return evaluate_intrinsic_numeric(context, arguments,
                                              [](auto a) { return (a > 0) - (a < 0); });
        case k_floor_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::floor(a); });
        case k_ceil_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::ceil(a); });
        case k_fract_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return a - std::floor(a); });
        case k_mod_IntrinsicKind:
            return evaluate_pairwise_intrinsic(
                    context, arguments, [](auto x, auto y) { return x - y * std::floor(x / y); });

        case k_min_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto a, auto b) { return (a < b) ? a : b; });
        case k_max_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto a, auto b) { return (a > b) ? a : b; });
        case k_clamp_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments,
                    [](auto x, auto l, auto h) { return (x < l) ? l : (x > h) ? h : x; });
        case k_saturate_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return (a < 0) ? 0 : (a > 1) ? 1 : a; });
        case k_mix_IntrinsicKind:
            if (arguments[2]->type().componentType().isBoolean()) {
                const SkSL::Type& numericType = arguments[0]->type().componentType();
                const auto eval = [](auto x, auto y, bool a) { return a ? y : x; };

                if (numericType.isFloat()) {
                    return evaluate_n_way_intrinsic_of_type<float, float, bool>(
                        context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
                } else if (numericType.isInteger()) {
                    return evaluate_n_way_intrinsic_of_type<SKSL_INT, SKSL_INT, bool>(
                        context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
                } else if (numericType.isBoolean()) {
                    return evaluate_n_way_intrinsic_of_type<bool, bool, bool>(
                        context, arguments[0].get(), arguments[1].get(), arguments[2].get(), eval);
                }

                SkDEBUGFAILF("unsupported type %s", numericType.description().c_str());
                return nullptr;
            } else {
                return evaluate_3_way_intrinsic(context, arguments,
                                        [](auto x, auto y, auto a) { return x * (1 - a) + y * a; });
            }
        case k_step_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto e, auto x) { return (x < e) ? 0 : 1; });
        case k_smoothstep_IntrinsicKind:
            return evaluate_3_way_intrinsic(context, arguments, [](auto edge0, auto edge1, auto x) {
                auto t = (x - edge0) / (edge1 - edge0);
                t = (t < 0) ? 0 : (t > 1) ? 1 : t;
                return t * t * (3.0 - 2.0 * t);
            });

        // 8.4 : Geometric Functions
        case k_length_IntrinsicKind:
            return coalesce_vector<float>(arguments, /*startingState=*/0,
                                          Intrinsics::coalesce_length,
                                          Intrinsics::finalize_length);
        case k_distance_IntrinsicKind:
            return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0,
                                                    Intrinsics::coalesce_distance,
                                                    Intrinsics::finalize_distance);
        case k_dot_IntrinsicKind:
            return coalesce_pairwise_vectors<float>(arguments, /*startingState=*/0,
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

        // 8.5 : Matrix Functions
        case k_matrixCompMult_IntrinsicKind:
            return evaluate_pairwise_intrinsic(context, arguments,
                                               [](auto x, auto y) { return x * y; });

        // Not supported until GLSL 1.40. Poly-filled by SkSL:
        case k_inverse_IntrinsicKind: {
            auto M = [&](int c, int r) -> float {
                int index = (arguments[0]->type().rows() * c) + r;
                return arguments[0]->getConstantSubexpression(index)->as<FloatLiteral>().value();
            };
            // Our matrix inverse is adapted from the logic in GLSLCodeGenerator::writeInverseHack.
            switch (arguments[0]->type().slotCount()) {
                case 4: {
                    float mat2[4] = {M(0, 0), M(0, 1),
                                     M(1, 0), M(1, 1)};
                    if (!SkInvert2x2Matrix(mat2, mat2)) {
                        return nullptr;
                    }
                    return DSLType::Construct(&arguments[0]->type(),
                                              mat2[0], mat2[1],
                                              mat2[2], mat2[3]).release();
                }
                case 9: {
                    float mat3[9] = {M(0, 0), M(0, 1), M(0, 2),
                                     M(1, 0), M(1, 1), M(1, 2),
                                     M(2, 0), M(2, 1), M(2, 2)};
                    if (!SkInvert3x3Matrix(mat3, mat3)) {
                        return nullptr;
                    }
                    return DSLType::Construct(&arguments[0]->type(),
                                              mat3[0], mat3[1], mat3[2],
                                              mat3[3], mat3[4], mat3[5],
                                              mat3[6], mat3[7], mat3[8]).release();
                }
                case 16: {
                    float mat4[16] = {M(0, 0), M(0, 1), M(0, 2), M(0, 3),
                                      M(1, 0), M(1, 1), M(1, 2), M(1, 3),
                                      M(2, 0), M(2, 1), M(2, 2), M(2, 3),
                                      M(3, 0), M(3, 1), M(3, 2), M(3, 3)};
                    if (!SkInvert4x4Matrix(mat4, mat4)) {
                        return nullptr;
                    }
                    return DSLType::Construct(&arguments[0]->type(),
                                              mat4[0],  mat4[1],  mat4[2],  mat4[3],
                                              mat4[4],  mat4[5],  mat4[6],  mat4[7],
                                              mat4[8],  mat4[9],  mat4[10], mat4[11],
                                              mat4[12], mat4[13], mat4[14], mat4[15]).release();
                }
            }
            SkDEBUGFAILF("unsupported type %s", arguments[0]->type().description().c_str());
            return nullptr;
        }

        // 8.6 : Vector Relational Functions
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
            return coalesce_vector<bool>(arguments, /*startingState=*/false,
                                         Intrinsics::coalesce_any,
                                         /*finalize=*/nullptr);
        case k_all_IntrinsicKind:
            return coalesce_vector<bool>(arguments, /*startingState=*/true,
                                         Intrinsics::coalesce_all,
                                         /*finalize=*/nullptr);
        case k_not_IntrinsicKind:
            return evaluate_intrinsic<bool>(context, arguments, [](bool a) { return !a; });

        // Additional intrinsics not required by GLSL ES2:
        case k_sinh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::sinh(a); });
        case k_cosh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::cosh(a); });
        case k_tanh_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::tanh(a); });
        case k_trunc_IntrinsicKind:
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::trunc(a); });
        case k_round_IntrinsicKind:      // GLSL `round` documents its rounding mode as unspecified
        case k_roundEven_IntrinsicKind:  // and is allowed to behave identically to `roundEven`.
            return evaluate_intrinsic<float>(context, arguments,
                                             [](float a) { return std::round(a / 2) * 2; });
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
