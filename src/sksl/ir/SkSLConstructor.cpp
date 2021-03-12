/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructor.h"

#include "src/sksl/ir/SkSLBoolLiteral.h"
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
        return MakeScalarConstructor(context, offset, type.scalarTypeForLiteral(), std::move(args));
    }
    if (type.isVector() || type.isMatrix()) {
        return MakeCompoundConstructor(context, offset, type, std::move(args));
    }
    if (type.isArray() && type.columns() > 0) {
        return MakeArrayConstructor(context, offset, type, std::move(args));
    }

    context.fErrors.error(offset, "cannot construct '" + type.displayName() + "'");
    return nullptr;
}

std::unique_ptr<Expression> Constructor::MakeScalarConstructor(const Context& context,
                                                               int offset,
                                                               const Type& type,
                                                               ExpressionArray args) {
    SkASSERT(type.isScalar());
    if (args.size() != 1) {
        context.fErrors.error(offset, "invalid arguments to '" + type.displayName() +
                                      "' constructor, (expected exactly 1 argument, but found " +
                                      to_string((uint64_t)args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.isScalar()) {
        context.fErrors.error(offset, "invalid argument to '" + type.displayName() +
                                      "' constructor (expected a number or bool, but found '" +
                                      argType.displayName() + "')");
        return nullptr;
    }

    if (std::unique_ptr<Expression> converted = SimplifyConversion(type, *args[0])) {
        return converted;
    }

    return std::make_unique<Constructor>(offset, type, std::move(args));
}

std::unique_ptr<Expression> Constructor::MakeCompoundConstructor(const Context& context,
                                                                 int offset,
                                                                 const Type& type,
                                                                 ExpressionArray args) {
    SkASSERT(type.isVector() || type.isMatrix());
    if (type.isMatrix() && args.size() == 1 && args[0]->type().isMatrix()) {
        // Matrix-from-matrix is always legal.
        return std::make_unique<Constructor>(offset, type, std::move(args));
    }

    if (args.size() == 1 && args[0]->type().isScalar()) {
        // A constructor containing a single scalar is a splat (for vectors) or diagonal matrix (for
        // matrices). In either event, it's legal regardless of the scalar's type. Synthesize an
        // explicit conversion to the proper type (this is a no-op if it's unnecessary).
        ExpressionArray castArgs;
        castArgs.push_back(Constructor::Convert(context, offset, type.componentType(),
                                                std::move(args)));
        SkASSERT(castArgs.front());
        return std::make_unique<Constructor>(offset, type, std::move(castArgs));
    }

    int expected = type.rows() * type.columns();

    if (type.isVector() && args.size() == 1 && args[0]->type().isVector() &&
        args[0]->type().columns() == expected) {
        // A vector constructor containing a single vector with the same number of columns is a
        // cast (e.g. float3 -> int3).
        return std::make_unique<Constructor>(offset, type, std::move(args));
    }

    // For more complex cases, we walk the argument list and fix up the arguments as needed.
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
            if (arg->is<Constructor>()) {
                Constructor& inner = arg->as<Constructor>();
                if (inner.arguments().size() > 1 &&
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
                    Constructor& inner = arg->as<Constructor>();
                    for (std::unique_ptr<Expression>& innerArg : inner.arguments()) {
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

std::unique_ptr<Expression> Constructor::MakeArrayConstructor(const Context& context,
                                                              int offset,
                                                              const Type& type,
                                                              ExpressionArray args) {
    SkASSERTF(type.isArray() && type.columns() > 0, "%s", type.description().c_str());

    // ES2 doesn't support first-class array types.
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(offset, "construction of array type '" + type.displayName() +
                                      "' is not supported");
        return nullptr;
    }

    // Check that the number of constructor arguments matches the array size.
    if (type.columns() != args.count()) {
        context.fErrors.error(offset, String::printf("invalid arguments to '%s' constructor "
                                                     "(expected %d elements, but found %d)",
                                                     type.displayName().c_str(), type.columns(),
                                                     args.count()));
        return nullptr;
    }

    // Convert each constructor argument to the array's component type.
    const Type& baseType = type.componentType();
    for (std::unique_ptr<Expression>& argument : args) {
        argument = baseType.coerceExpression(std::move(argument), context);
        if (!argument) {
            return nullptr;
        }
    }
    return std::make_unique<Constructor>(offset, type, std::move(args));
}

std::unique_ptr<Expression> Constructor::SimplifyConversion(const Type& constructorType,
                                                            const Expression& expr) {
    if (expr.is<IntLiteral>()) {
        SKSL_INT value = expr.as<IntLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1) to 1.0
            return FloatLiteral::Make(expr.fOffset, (SKSL_FLOAT)value, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1) to 1u
            return IntLiteral::Make(expr.fOffset, value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1) to true/false
            return BoolLiteral::Make(expr.fOffset, value != 0, &constructorType);
        }
    } else if (expr.is<FloatLiteral>()) {
        float value = expr.as<FloatLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1.23) to 1.23
            return FloatLiteral::Make(expr.fOffset, value, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1.23) to 1u
            return IntLiteral::Make(expr.fOffset, (SKSL_INT)value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1.23) to true/false
            return BoolLiteral::Make(expr.fOffset, value != 0.0f, &constructorType);
        }
    } else if (expr.is<BoolLiteral>()) {
        bool value = expr.as<BoolLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(true) to 1.0
            return FloatLiteral::Make(expr.fOffset, value ? 1.0f : 0.0f, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(true) to 1u
            return IntLiteral::Make(expr.fOffset, value ? 1 : 0, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(true) to true/false
            return BoolLiteral::Make(expr.fOffset, value, &constructorType);
        }
    }
    return nullptr;
}

Expression::ComparisonResult Constructor::compareConstant(const Expression& other) const {
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
            SkASSERT(this->arguments()[0]->is<Constructor>());
            // single matrix argument. make sure we're within the argument's bounds.
            if (col < argType.columns() && row < argType.rows()) {
                // within bounds, defer to argument
                return this->arguments()[0]->as<Constructor>().getMatComponent(col, row);
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

}  // namespace SkSL
