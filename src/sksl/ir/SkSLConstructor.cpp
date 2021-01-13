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

namespace SkSL {

std::unique_ptr<Expression> Constructor::constantPropagate(const IRGenerator& irGenerator,
                                                           const DefinitionMap& definitions) {
    // Handle conversion constructors of literal values.
    if (this->arguments().size() == 1) {
        return SimplifyConversion(this->type(), *this->arguments().front());
    }
    return nullptr;
}

std::unique_ptr<Expression> Constructor::SimplifyConversion(const Type& constructorType,
                                                            const Expression& expr) {
    if (expr.is<IntLiteral>()) {
        SKSL_INT value = expr.as<IntLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1) to 1.0
            return std::make_unique<FloatLiteral>(expr.fOffset, (SKSL_FLOAT)value,
                                                  &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1) to 1u
            return std::make_unique<IntLiteral>(expr.fOffset, value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1) to true/false
            return std::make_unique<BoolLiteral>(expr.fOffset, value != 0, &constructorType);
        }
    } else if (expr.is<FloatLiteral>()) {
        float value = expr.as<FloatLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1.23) to 1.23
            return std::make_unique<FloatLiteral>(expr.fOffset, value, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1.23) to 1u
            return std::make_unique<IntLiteral>(expr.fOffset, (SKSL_INT)value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1.23) to true/false
            return std::make_unique<BoolLiteral>(expr.fOffset, value != 0.0f, &constructorType);
        }
    } else if (expr.is<BoolLiteral>()) {
        bool value = expr.as<BoolLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(true) to 1.0
            return std::make_unique<FloatLiteral>(expr.fOffset, value ? 1.0f : 0.0f,
                                                  &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(true) to 1u
            return std::make_unique<IntLiteral>(expr.fOffset, value ? 1 : 0, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(true) to true/false
            return std::make_unique<BoolLiteral>(expr.fOffset, value, &constructorType);
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
    ABORT("can't happen, matrix component out of bounds");
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
