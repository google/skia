/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructor.h"

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLConstructorStruct.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

static std::unique_ptr<Expression> convert_compound_constructor(const Context& context,
                                                                int line,
                                                                const Type& type,
                                                                ExpressionArray args) {
    SkASSERT(type.isVector() || type.isMatrix());

    // The meaning of a compound constructor containing a single argument varies significantly in
    // GLSL/SkSL, depending on the argument type.
    if (args.size() == 1) {
        std::unique_ptr<Expression>& argument = args.front();
        if (type.isVector() && argument->type().isVector() &&
            argument->type().componentType() == type.componentType() &&
            argument->type().slotCount() > type.slotCount()) {
            // Casting a vector-type into a smaller matching vector-type is a slice in GLSL.
            // We don't allow those casts in SkSL; recommend a swizzle instead.
            // Only `.xy` and `.xyz` are valid recommendations here, because `.x` would imply a
            // scalar(vector) cast, and nothing has more slots than `.xyzw`.
            const char* swizzleHint;
            switch (type.slotCount()) {
                case 2:  swizzleHint = "; use '.xy' instead"; break;
                case 3:  swizzleHint = "; use '.xyz' instead"; break;
                default: swizzleHint = ""; SkDEBUGFAIL("unexpected slicing cast"); break;
            }

            context.fErrors->error(line, "'" + argument->type().displayName() +
                                         "' is not a valid parameter to '" + type.displayName() +
                                         "' constructor" + swizzleHint);
            return nullptr;
        }

        if (argument->type().isScalar()) {
            // A constructor containing a single scalar is a splat (for vectors) or diagonal matrix
            // (for matrices). It's legal regardless of the scalar's type, so synthesize an explicit
            // conversion to the proper type. (This cast is a no-op if it's unnecessary.)
            std::unique_ptr<Expression> typecast = ConstructorScalarCast::Make(
                    context, line, type.componentType(), std::move(argument));

            // Matrix-from-scalar creates a diagonal matrix; vector-from-scalar creates a splat.
            return type.isMatrix()
                       ? ConstructorDiagonalMatrix::Make(context, line, type, std::move(typecast))
                       : ConstructorSplat::Make(context, line, type, std::move(typecast));
        } else if (argument->type().isVector()) {
            // A vector constructor containing a single vector with the same number of columns is a
            // cast (e.g. float3 -> int3).
            if (type.isVector() && argument->type().columns() == type.columns()) {
                return ConstructorCompoundCast::Make(context, line, type, std::move(argument));
            }
        } else if (argument->type().isMatrix()) {
            // A matrix constructor containing a single matrix can be a resize, typecast, or both.
            // GLSL lumps these into one category, but internally SkSL keeps them distinct.
            if (type.isMatrix()) {
                // First, handle type conversion. If the component types differ, synthesize the
                // destination type with the argument's rows/columns. (This will be a no-op if it's
                // already the right type.)
                const Type& typecastType = type.componentType().toCompound(
                        context,
                        argument->type().columns(),
                        argument->type().rows());
                argument = ConstructorCompoundCast::Make(context, line, typecastType,
                                                         std::move(argument));

                // Casting a matrix type into another matrix type is a resize.
                return ConstructorMatrixResize::Make(context, line, type,
                                                     std::move(argument));
            }

            // A vector constructor containing a single matrix can be compound construction if the
            // matrix is 2x2 and the vector is 4-slot.
            if (type.isVector() && type.columns() == 4 && argument->type().slotCount() == 4) {
                // Casting a 2x2 matrix to a vector is a form of compound construction.
                // First, reshape the matrix into a 4-slot vector of the same type.
                const Type& vectorType = argument->type().componentType().toCompound(context,
                                                                                     /*columns=*/4,
                                                                                     /*rows=*/1);
                std::unique_ptr<Expression> vecCtor =
                        ConstructorCompound::Make(context, line, vectorType, std::move(args));

                // Then, add a typecast to the result expression to ensure the types match.
                // This will be a no-op if no typecasting is needed.
                return ConstructorCompoundCast::Make(context, line, type, std::move(vecCtor));
            }
        }
    }

    // For more complex cases, we walk the argument list and fix up the arguments as needed.
    int expected = type.rows() * type.columns();
    int actual = 0;
    for (std::unique_ptr<Expression>& arg : args) {
        if (!arg->type().isScalar() && !arg->type().isVector()) {
            context.fErrors->error(line, "'" + arg->type().displayName() +
                                         "' is not a valid parameter to '" + type.displayName() +
                                         "' constructor");
            return nullptr;
        }

        // Rely on Constructor::Convert to force this subexpression to the proper type. If it's a
        // literal, this will make sure it's the right type of literal. If an expression of matching
        // type, the expression will be returned as-is. If it's an expression of mismatched type,
        // this adds a cast.
        int ctorLine = arg->fLine;
        const Type& ctorType = type.componentType().toCompound(context, arg->type().columns(),
                                                               /*rows=*/1);
        ExpressionArray ctorArg;
        ctorArg.push_back(std::move(arg));
        arg = Constructor::Convert(context, ctorLine, ctorType, std::move(ctorArg));
        if (!arg) {
            return nullptr;
        }
        actual += ctorType.columns();
    }

    if (actual != expected) {
        context.fErrors->error(line, "invalid arguments to '" + type.displayName() +
                                     "' constructor (expected " + to_string(expected) +
                                     " scalars, but found " + to_string(actual) + ")");
        return nullptr;
    }

    return ConstructorCompound::Make(context, line, type, std::move(args));
}

std::unique_ptr<Expression> Constructor::Convert(const Context& context,
                                                 int line,
                                                 const Type& type,
                                                 ExpressionArray args) {
    if (args.size() == 1 && args[0]->type() == type && !type.componentType().isOpaque()) {
        // Don't generate redundant casts; if the expression is already of the correct type, just
        // return it as-is.
        return std::move(args[0]);
    }
    if (type.isScalar()) {
        return ConstructorScalarCast::Convert(context, line, type, std::move(args));
    }
    if (type.isVector() || type.isMatrix()) {
        return convert_compound_constructor(context, line, type, std::move(args));
    }
    if (type.isArray() && type.columns() > 0) {
        return ConstructorArray::Convert(context, line, type, std::move(args));
    }
    if (type.isStruct() && type.fields().size() > 0) {
        return ConstructorStruct::Convert(context, line, type, std::move(args));
    }

    context.fErrors->error(line, "cannot construct '" + type.displayName() + "'");
    return nullptr;
}

const Expression* AnyConstructor::getConstantSubexpression(int n) const {
    SkASSERT(n >= 0 && n < (int)this->type().slotCount());
    for (const std::unique_ptr<Expression>& arg : this->argumentSpan()) {
        int argSlots = arg->type().slotCount();
        if (n < argSlots) {
            return arg->getConstantSubexpression(n);
        }
        n -= argSlots;
    }

    SkDEBUGFAIL("argument-list slot count doesn't match constructor-type slot count");
    return nullptr;
}

Expression::ComparisonResult AnyConstructor::compareConstant(const Expression& other) const {
    SkASSERT(this->type().slotCount() == other.type().slotCount());

    if (!other.allowsConstantSubexpressions()) {
        return ComparisonResult::kUnknown;
    }

    ComparisonResult result = ComparisonResult::kEqual;
    int exprs = this->type().slotCount();
    for (int n = 0; n < exprs; ++n) {
        // Get the n'th subexpression from each side. If either one is null, return "unknown."
        const Expression* left = this->getConstantSubexpression(n);
        if (!left) {
            return ComparisonResult::kUnknown;
        }
        const Expression* right = other.getConstantSubexpression(n);
        if (!right) {
            return ComparisonResult::kUnknown;
        }
        // Recurse into the subexpressions; the literal types will perform real comparisons, and
        // most other expressions fall back on the base class Expression which returns unknown.
        result = left->compareConstant(*right);
        if (result != ComparisonResult::kEqual) {
            break;
        }
    }
    return result;
}

AnyConstructor& Expression::asAnyConstructor() {
    SkASSERT(this->isAnyConstructor());
    return static_cast<AnyConstructor&>(*this);
}

const AnyConstructor& Expression::asAnyConstructor() const {
    SkASSERT(this->isAnyConstructor());
    return static_cast<const AnyConstructor&>(*this);
}

}  // namespace SkSL
