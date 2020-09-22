/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"

namespace SkSL {

/**
 * Represents the construction of a compound type, such as "float2(x, y)".
 *
 * Vector constructors will always consist of either exactly 1 scalar, or a collection of vectors
 * and scalars totalling exactly the right number of scalar components.
 *
 * Matrix constructors will always consist of either exactly 1 scalar, exactly 1 matrix, or a
 * collection of vectors and scalars totalling exactly the right number of scalar components.
 */
struct Constructor : public Expression {
    static constexpr Kind kExpressionKind = Kind::kConstructor;

    Constructor(int offset, const Type* type, std::vector<std::unique_ptr<Expression>> arguments)
            : INHERITED(offset, kExpressionKind, type)
            , fArguments(std::move(arguments)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 && fArguments[0]->is<IntLiteral>()) {
            const Context& context = irGenerator.fContext;
            const Type& type = this->type();
            int64_t intValue = fArguments[0]->as<IntLiteral>().fValue;

            if (type.isFloat()) {
                // promote float(1) to 1.0
                return std::make_unique<FloatLiteral>(context, fOffset, intValue);
            } else if (type.isInteger()) {
                // promote uint(1) to 1u
                return std::make_unique<IntLiteral>(fOffset, intValue, &type);
            } else if (&type == context.fBool_Type.get()) {
                // promote bool(k) to true/false
                return std::make_unique<BoolLiteral>(context, fOffset, intValue != 0);
            }
        }
        return nullptr;
    }

    bool hasProperty(Property property) const override {
        for (const std::unique_ptr<Expression>& arg: fArguments) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<Expression>> cloned;
        cloned.reserve(fArguments.size());
        for (const std::unique_ptr<Expression>& arg: fArguments) {
            cloned.push_back(arg->clone());
        }
        return std::make_unique<Constructor>(fOffset, &this->type(), std::move(cloned));
    }

    String description() const override {
        String result = this->type().description() + "(";
        const char* separator = "";
        for (const std::unique_ptr<Expression>& arg: fArguments) {
            result += separator;
            result += arg->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isCompileTimeConstant() const override {
        for (const std::unique_ptr<Expression>& arg: fArguments) {
            if (!arg->isCompileTimeConstant()) {
                return false;
            }
        }
        return true;
    }

    bool isConstantOrUniform() const override {
        for (const std::unique_ptr<Expression>& arg: fArguments) {
            if (!arg->isConstantOrUniform()) {
                return false;
            }
        }
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        const Constructor& c = other.as<Constructor>();
        const Type& myType = this->type();
        const Type& otherType = c.type();
        SkASSERT(myType == otherType);
        if (otherType.typeKind() == Type::TypeKind::kVector) {
            bool isFloat = otherType.columns() > 1 ? otherType.componentType().isFloat()
                                                 : otherType.isFloat();
            for (int i = 0; i < myType.columns(); i++) {
                if (isFloat) {
                    if (this->getFVecComponent(i) != c.getFVecComponent(i)) {
                        return false;
                    }
                } else if (this->getIVecComponent(i) != c.getIVecComponent(i)) {
                    return false;
                }
            }
            return true;
        }
        // shouldn't be possible to have a constant constructor that isn't a vector or matrix;
        // a constant scalar constructor should have been collapsed down to the appropriate
        // literal
        SkASSERT(myType.typeKind() == Type::TypeKind::kMatrix);
        for (int col = 0; col < myType.columns(); col++) {
            for (int row = 0; row < myType.rows(); row++) {
                if (getMatComponent(col, row) != c.getMatComponent(col, row)) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename type>
    type getVecComponent(int index) const {
        SkASSERT(this->type().typeKind() == Type::TypeKind::kVector);
        if (fArguments.size() == 1 &&
            fArguments[0]->type().typeKind() == Type::TypeKind::kScalar) {
            // This constructor just wraps a scalar. Propagate out the value.
            if (std::is_floating_point<type>::value) {
                return fArguments[0]->getConstantFloat();
            } else {
                return fArguments[0]->getConstantInt();
            }
        }

        // Walk through all the constructor arguments until we reach the index we're searching for.
        int current = 0;
        for (const std::unique_ptr<Expression>& arg : fArguments) {
            if (current > index) {
                // Somehow, we went past the argument we're looking for. Bail.
                break;
            }

            if (arg->type().typeKind() == Type::TypeKind::kScalar) {
                if (index == current) {
                    // We're on the proper argument, and it's a scalar; fetch it.
                    if (std::is_floating_point<type>::value) {
                        return arg->getConstantFloat();
                    } else {
                        return arg->getConstantInt();
                    }
                }
                current++;
                continue;
            }

            switch (arg->kind()) {
                case Kind::kConstructor: {
                    const Constructor& constructor = static_cast<const Constructor&>(*arg);
                    if (current + constructor.type().columns() > index) {
                        // We've found a constructor that overlaps the proper argument. Descend into
                        // it, honoring the type.
                        if (constructor.type().componentType().isFloat()) {
                            return type(constructor.getVecComponent<SKSL_FLOAT>(index - current));
                        } else {
                            return type(constructor.getVecComponent<SKSL_INT>(index - current));
                        }
                    }
                    break;
                }
                case Kind::kPrefix: {
                    const PrefixExpression& prefix = static_cast<const PrefixExpression&>(*arg);
                    if (current + prefix.type().columns() > index) {
                        // We found a prefix operator that contains the proper argument. Descend
                        // into it. We only support for constant propagation of the unary minus, so
                        // we shouldn't see any other tokens here.
                        SkASSERT(prefix.fOperator == Token::Kind::TK_MINUS);

                        // We expect the - prefix to always be attached to a constructor.
                        SkASSERT(prefix.fOperand->kind() == Kind::kConstructor);
                        const Constructor& constructor =
                                static_cast<const Constructor&>(*prefix.fOperand);

                        // Descend into this constructor, honoring the type.
                        if (constructor.type().componentType().isFloat()) {
                            return -type(constructor.getVecComponent<SKSL_FLOAT>(index - current));
                        } else {
                            return -type(constructor.getVecComponent<SKSL_INT>(index - current));
                        }
                    }
                    break;
                }
                default: {
                    SkDEBUGFAILF("unexpected component %d { %s } in %s\n",
                                 index, arg->description().c_str(), description().c_str());
                    break;
                }
            }

            current += arg->type().columns();
        }

        SkDEBUGFAILF("failed to find vector component %d in %s\n", index, description().c_str());
        return -1;
    }

    SKSL_FLOAT getFVecComponent(int n) const override {
        return this->getVecComponent<SKSL_FLOAT>(n);
    }

    /**
     * For a literal vector expression, return the integer value of the n'th vector component. It is
     * an error to call this method on an expression which is not a literal vector.
     */
    SKSL_INT getIVecComponent(int n) const override {
        return this->getVecComponent<SKSL_INT>(n);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override {
        SkDEBUGCODE(const Type& myType = this->type();)
        SkASSERT(this->isCompileTimeConstant());
        SkASSERT(myType.typeKind() == Type::TypeKind::kMatrix);
        SkASSERT(col < myType.columns() && row < myType.rows());
        if (fArguments.size() == 1) {
            const Type& argType = fArguments[0]->type();
            if (argType.typeKind() == Type::TypeKind::kScalar) {
                // single scalar argument, so matrix is of the form:
                // x 0 0
                // 0 x 0
                // 0 0 x
                // return x if col == row
                return col == row ? fArguments[0]->getConstantFloat() : 0.0;
            }
            if (argType.typeKind() == Type::TypeKind::kMatrix) {
                SkASSERT(fArguments[0]->kind() == Expression::Kind::kConstructor);
                // single matrix argument. make sure we're within the argument's bounds.
                if (col < argType.columns() && row < argType.rows()) {
                    // within bounds, defer to argument
                    return ((Constructor&) *fArguments[0]).getMatComponent(col, row);
                }
                // out of bounds
                return 0.0;
            }
        }
        int currentIndex = 0;
        int targetIndex = col * this->type().rows() + row;
        for (const auto& arg : fArguments) {
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

    std::vector<std::unique_ptr<Expression>> fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
