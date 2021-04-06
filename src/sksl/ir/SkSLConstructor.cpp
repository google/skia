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

Expression::ComparisonResult Constructor::compareConstant(const Expression& other) const {
    if (other.is<ConstructorDiagonalMatrix>()) {
        return other.compareConstant(*this);
    }
    if (other.is<ConstructorMatrixResize>()) {
        return other.compareConstant(*this);
    }
    if (other.is<ConstructorSplat>()) {
        return other.compareConstant(*this);
    }
    if (!other.is<Constructor>()) {
        return ComparisonResult::kUnknown;
    }
    const Constructor& c = other.as<Constructor>();
    const Type& myType = this->type();
    SkASSERT(myType == c.type());

    if (myType.isVector()) {
        if (myType.componentType().isFloat()) {
            for (int i = 0; i < myType.columns(); i++) {
                if (this->getFVecComponent(i) != c.getFVecComponent(i)) {
                    return ComparisonResult::kNotEqual;
                }
            }
            return ComparisonResult::kEqual;
        }
        if (myType.componentType().isInteger()) {
            for (int i = 0; i < myType.columns(); i++) {
                if (this->getIVecComponent(i) != c.getIVecComponent(i)) {
                    return ComparisonResult::kNotEqual;
                }
            }
            return ComparisonResult::kEqual;
        }
        if (myType.componentType().isBoolean()) {
            for (int i = 0; i < myType.columns(); i++) {
                if (this->getBVecComponent(i) != c.getBVecComponent(i)) {
                    return ComparisonResult::kNotEqual;
                }
            }
            return ComparisonResult::kEqual;
        }
    }

    if (myType.isMatrix()) {
        for (int col = 0; col < myType.columns(); col++) {
            for (int row = 0; row < myType.rows(); row++) {
                if (getMatComponent(col, row) != c.getMatComponent(col, row)) {
                    return ComparisonResult::kNotEqual;
                }
            }
        }
        return ComparisonResult::kEqual;
    }

    SkDEBUGFAILF("compareConstant unexpected type: %s", myType.description().c_str());
    return ComparisonResult::kUnknown;
}

template <typename ResultType>
ResultType Constructor::getConstantValue(const Expression& expr) const {
    const Type& type = expr.type();
    SkASSERT(type.isScalar());
    if (type.isFloat()) {
        return ResultType(expr.getConstantFloat());
    } else if (type.isInteger()) {
        return ResultType(expr.getConstantInt());
    } else if (type.isBoolean()) {
        return ResultType(expr.getConstantBool());
    }
    SkDEBUGFAILF("unrecognized kind of constant value: %s", expr.description().c_str());
    return ResultType(0);
}

template <typename ResultType>
ResultType Constructor::getInnerVecComponent(const Expression& expr, int position) const {
    const Type& type = expr.type().componentType();
    if (type.isFloat()) {
        return ResultType(expr.getVecComponent<SKSL_FLOAT>(position));
    } else if (type.isInteger()) {
        return ResultType(expr.getVecComponent<SKSL_INT>(position));
    } else if (type.isBoolean()) {
        return ResultType(expr.getVecComponent<bool>(position));
    }
    SkDEBUGFAILF("unrecognized type of constant: %s", expr.description().c_str());
    return ResultType(0);
};

template <typename ResultType>
ResultType Constructor::getVecComponent(int index) const {
    static_assert(std::is_same<ResultType, SKSL_FLOAT>::value ||
                  std::is_same<ResultType, SKSL_INT>::value ||
                  std::is_same<ResultType, bool>::value);

    SkASSERT(this->type().isVector());
    SkASSERT(this->isCompileTimeConstant());

    if (this->arguments().size() == 1 &&
        this->arguments()[0]->type().isScalar()) {
        // This constructor just wraps a scalar. Propagate out the value.
        return this->getConstantValue<ResultType>(*this->arguments()[0]);
    }

    // Walk through all the constructor arguments until we reach the index we're searching for.
    int current = 0;
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        if (current > index) {
            // Somehow, we went past the argument we're looking for. Bail.
            break;
        }

        if (arg->type().isScalar()) {
            if (index == current) {
                // We're on the proper argument, and it's a scalar; fetch it.
                return this->getConstantValue<ResultType>(*arg);
            }
            current++;
            continue;
        }

        if (arg->type().isVector()) {
            if (current + arg->type().columns() > index) {
                // We've found an expression that encompasses the proper argument. Descend into it.
                return this->getInnerVecComponent<ResultType>(*arg, index - current);
            }
        }

        current += arg->type().columns();
    }

    SkDEBUGFAILF("failed to find vector component %d in %s\n", index, description().c_str());
    return ResultType(0);
}

template SKSL_INT Constructor::getVecComponent(int) const;
template SKSL_FLOAT Constructor::getVecComponent(int) const;
template bool Constructor::getVecComponent(int) const;

SKSL_FLOAT Constructor::getMatComponent(int col, int row) const {
    SkDEBUGCODE(const Type& myType = this->type();)
    SkASSERT(this->isCompileTimeConstant());
    SkASSERT(myType.isMatrix());
    SkASSERT(col < myType.columns() && row < myType.rows());
    if (this->arguments().size() == 1) {
        const Type& argType = this->arguments()[0]->type();
        if (argType.isScalar()) {
            // single scalar argument, so matrix is of the form:
            // x 0 0
            // 0 x 0
            // 0 0 x
            // return x if col == row
            return col == row ? this->getConstantValue<SKSL_FLOAT>(*this->arguments()[0]) : 0.0;
        }
        if (argType.isMatrix()) {
            SkASSERT(this->arguments()[0]->isAnyConstructor());
            // single matrix argument. make sure we're within the argument's bounds.
            if (col < argType.columns() && row < argType.rows()) {
                // within bounds, defer to argument
                return this->arguments()[0]->getMatComponent(col, row);
            }
            // out of bounds
            return 0.0;
        }
    }
    int currentIndex = 0;
    int targetIndex = col * this->type().rows() + row;
    for (const auto& arg : this->arguments()) {
        const Type& argType = arg->type();
        SkASSERT(targetIndex >= currentIndex);
        SkASSERT(argType.rows() == 1);
        if (currentIndex + argType.columns() > targetIndex) {
            if (argType.columns() == 1) {
                return arg->getConstantFloat();
            } else {
                return arg->getFVecComponent(targetIndex - currentIndex);
            }
        }
        currentIndex += argType.columns();
    }
    SK_ABORT("can't happen, matrix component out of bounds");
}

SKSL_INT Constructor::getConstantInt() const {
    // We're looking for scalar integer constructors only, i.e. `int(1)`.
    SkASSERT(this->arguments().size() == 1);
    SkASSERT(this->type().columns() == 1);
    SkASSERT(this->type().isInteger());

    // This might be a cast, meaning the inner argument would actually be a different scalar type.
    const Expression& expr = *this->arguments().front();
    SkASSERT(expr.type().isInteger() || expr.type().isFloat() || expr.type().isBoolean());
    return expr.type().isInteger() ? expr.getConstantInt() :
             expr.type().isFloat() ? (SKSL_INT)expr.getConstantFloat() :
                                     (SKSL_INT)expr.getConstantBool();
}

SKSL_FLOAT Constructor::getConstantFloat() const {
    // We're looking for scalar integer constructors only, i.e. `float(1.0)`.
    SkASSERT(this->arguments().size() == 1);
    SkASSERT(this->type().columns() == 1);
    SkASSERT(this->type().isFloat());

    // This might be a cast, meaning the inner argument would actually be a different scalar type.
    const Expression& expr = *this->arguments().front();
    SkASSERT(expr.type().isInteger() || expr.type().isFloat() || expr.type().isBoolean());
    return   expr.type().isFloat() ? expr.getConstantFloat() :
           expr.type().isInteger() ? (SKSL_FLOAT)expr.getConstantInt() :
                                     (SKSL_FLOAT)expr.getConstantBool();
}

bool Constructor::getConstantBool() const {
    // We're looking for scalar Boolean constructors only, i.e. `bool(true)`.
    SkASSERT(this->arguments().size() == 1);
    SkASSERT(this->type().columns() == 1);
    SkASSERT(this->type().isBoolean());

    // This might be a cast, meaning the inner argument would actually be a different scalar type.
    const Expression& expr = *this->arguments().front();
    SkASSERT(expr.type().isInteger() || expr.type().isFloat() || expr.type().isBoolean());
    return expr.type().isBoolean() ? expr.getConstantBool() :
           expr.type().isInteger() ? (bool)expr.getConstantInt() :
                                     (bool)expr.getConstantFloat();
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
