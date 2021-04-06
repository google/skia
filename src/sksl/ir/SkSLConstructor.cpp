/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructor.h"

#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLConstructorVectorCast.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> Constructor::Convert(const Context& context,
                                                 int offset,
                                                 const Type& type,
                                                 ExpressionArray args) {
    // FIXME: add support for structs
    if (args.size() == 1 && args[0]->type() == type && !type.componentType().isOpaque()) {
        // Don't generate redundant casts; if the expression is already of the correct type, just
        // return it as-is.
        return std::move(args[0]);
    }
    if (type.isScalar()) {
        return ConstructorScalarCast::Convert(context, offset, type, std::move(args));
    }
    if (type.isVector() || type.isMatrix()) {
        return MakeCompoundConstructor(context, offset, type, std::move(args));
    }
    if (type.isArray() && type.columns() > 0) {
        return ConstructorArray::Convert(context, offset, type, std::move(args));
    }

    context.fErrors.error(offset, "cannot construct '" + type.displayName() + "'");
    return nullptr;
}

std::unique_ptr<Expression> Constructor::MakeCompoundConstructor(const Context& context,
                                                                 int offset,
                                                                 const Type& type,
                                                                 ExpressionArray args) {
    SkASSERT(type.isVector() || type.isMatrix());

    // The meaning of a compound constructor containing a single argument varies significantly in
    // GLSL/SkSL, depending on the argument type.
    if (args.size() == 1) {
        std::unique_ptr<Expression>& argument = args.front();
        if (argument->type().isScalar()) {
            // A constructor containing a single scalar is a splat (for vectors) or diagonal matrix
            // (for matrices). It's legal regardless of the scalar's type, so synthesize an explicit
            // conversion to the proper type. (This cast is a no-op if it's unnecessary.)
            std::unique_ptr<Expression> typecast = ConstructorScalarCast::Make(
                    context, offset, type.componentType(), std::move(argument));

            // Matrix-from-scalar creates a diagonal matrix; vector-from-scalar creates a splat.
            return type.isMatrix()
                       ? ConstructorDiagonalMatrix::Make(context, offset, type, std::move(typecast))
                       : ConstructorSplat::Make(context, offset, type, std::move(typecast));
        } else if (argument->type().isVector()) {
            // A vector constructor containing a single vector with the same number of columns is a
            // cast (e.g. float3 -> int3).
            if (type.isVector() && argument->type().columns() == type.columns()) {
                return ConstructorVectorCast::Make(context, offset, type, std::move(argument));
            }
        } else if (argument->type().isMatrix()) {
            // A matrix constructor containing a single matrix can be a resize, typecast, or both.
            // GLSL lumps these into one category, but internally SkSL keeps them distinct.
            if (type.isMatrix()) {
                // First, handle type conversion. If the component types differ, synthesize the
                // destination type with the argument's rows/columns. If not, leave it as-is.
                std::unique_ptr<Expression> typecast;
                if (type.componentType() != argument->type().componentType()) {
                    const Type& typecastType = type.componentType().toCompound(
                            context,
                            argument->type().columns(),
                            argument->type().rows());
                    typecast = std::make_unique<Constructor>(offset, typecastType, std::move(args));
                    SkASSERT(typecast);
                } else {
                    typecast = std::move(argument);
                }

                // Next, wrap the typecasted expression in a matrix-resize constructor if the sizes
                // differ. If not, return the typecasted expression as-is.
                return ConstructorMatrixResize::Make(context, offset, type, std::move(typecast));
            }
        }
    }

    // For more complex cases, we walk the argument list and fix up the arguments as needed.
    int expected = type.rows() * type.columns();
    int actual = 0;
    for (std::unique_ptr<Expression>& arg : args) {
        if (!arg->type().isScalar() && !arg->type().isVector()) {
            context.fErrors.error(offset, "'" + arg->type().displayName() +
                                          "' is not a valid parameter to '" +
                                          type.displayName() + "' constructor");
            return nullptr;
        }

        // Rely on Constructor::Convert to force this subexpression to the proper type. If it's a
        // literal, this will make sure it's the right type of literal. If an expression of matching
        // type, the expression will be returned as-is. If it's an expression of mismatched type,
        // this adds a cast.
        int offset = arg->fOffset;
        const Type& ctorType = type.componentType().toCompound(context, arg->type().columns(),
                                                               /*rows=*/1);
        ExpressionArray ctorArg;
        ctorArg.push_back(std::move(arg));
        arg = Constructor::Convert(context, offset, ctorType, std::move(ctorArg));
        if (!arg) {
            return nullptr;
        }
        actual += ctorType.columns();
    }

    if (actual != expected) {
        context.fErrors.error(offset, "invalid arguments to '" + type.displayName() +
                                      "' constructor (expected " + to_string(expected) +
                                      " scalars, but found " + to_string(actual) + ")");
        return nullptr;
    }

    if (context.fConfig->fSettings.fOptimize) {
        // Find constructors embedded inside constructors and flatten them out where possible.
        //   -  float4(float2(1, 2), 3, 4)                -->  float4(1, 2, 3, 4)
        //   -  float4(w, float3(sin(x), cos(y), tan(z))) -->  float4(w, sin(x), cos(y), tan(z))

        // Inspect each constructor argument to see if it's a candidate for flattening.
        // Remember matched arguments in a bitfield, "argsToOptimize".
        int argsToOptimize = 0;
        int currBit = 1;
        for (const std::unique_ptr<Expression>& arg : args) {
            if (arg->isAnyConstructor()) {
                AnyConstructor& inner = arg->asAnyConstructor();
                if (inner.argumentSpan().size() > 1 &&
                    inner.type().componentType() == type.componentType()) {
                    argsToOptimize |= currBit;
                }
            }
            currBit <<= 1;
        }

        if (argsToOptimize) {
            // We found at least one argument that could be flattened out. Re-walk the constructor
            // args and flatten the candidates we found during our initial pass.
            ExpressionArray flattened;
            flattened.reserve_back(type.columns());
            currBit = 1;
            for (std::unique_ptr<Expression>& arg : args) {
                if (argsToOptimize & currBit) {
                    AnyConstructor& inner = arg->asAnyConstructor();
                    for (std::unique_ptr<Expression>& innerArg : inner.argumentSpan()) {
                        flattened.push_back(std::move(innerArg));
                    }
                } else {
                    flattened.push_back(std::move(arg));
                }
                currBit <<= 1;
            }
            args = std::move(flattened);
        }
    }

    return std::make_unique<Constructor>(offset, type, std::move(args));
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
    ComparisonResult result = ComparisonResult::kEqual;
    SkASSERT(this->type().slotCount() == other.type().slotCount());

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
