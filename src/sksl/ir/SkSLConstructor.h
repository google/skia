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
    static constexpr Kind kExpressionKind = kConstructor_Kind;

    Constructor(int offset, const Type& type, std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(offset, kExpressionKind, type)
    , fArguments(std::move(arguments)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 && fArguments[0]->fKind == Expression::kIntLiteral_Kind) {
            if (fType.isFloat()) {
                // promote float(1) to 1.0
                int64_t intValue = fArguments[0]->as<IntLiteral>().fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(irGenerator.fContext,
                                                                    fOffset,
                                                                    intValue));
            } else if (fType.isInteger()) {
                // promote uint(1) to 1u
                int64_t intValue = fArguments[0]->as<IntLiteral>().fValue;
                return std::unique_ptr<Expression>(new IntLiteral(fOffset,
                                                                  intValue,
                                                                  &fType));
            }
        }
        return nullptr;
    }

    bool hasProperty(Property property) const override {
        for (const auto& arg : fArguments) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<Expression>> cloned;
        for (const auto& arg : fArguments) {
            cloned.push_back(arg->clone());
        }
        return std::unique_ptr<Expression>(new Constructor(fOffset, fType, std::move(cloned)));
    }

    String description() const override {
        String result = fType.description() + "(";
        String separator;
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isCompileTimeConstant() const override {
        for (size_t i = 0; i < fArguments.size(); i++) {
            if (!fArguments[i]->isCompileTimeConstant()) {
                return false;
            }
        }
        return true;
    }

    bool isConstantOrUniform() const override {
        for (size_t i = 0; i < fArguments.size(); i++) {
            if (!fArguments[i]->isConstantOrUniform()) {
                return false;
            }
        }
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        SkASSERT(other.fKind == Expression::kConstructor_Kind && other.fType == fType);
        Constructor& c = (Constructor&) other;
        if (c.fType.kind() == Type::kVector_Kind) {
            bool isFloat = c.fType.columns() > 1 ? c.fType.componentType().isFloat()
                                                 : c.fType.isFloat();
            for (int i = 0; i < fType.columns(); i++) {
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
        SkASSERT(fType.kind() == Type::kMatrix_Kind);
        for (int col = 0; col < fType.columns(); col++) {
            for (int row = 0; row < fType.rows(); row++) {
                if (getMatComponent(col, row) != c.getMatComponent(col, row)) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename type>
    type getVecComponent(int index) const {
        SkASSERT(fType.kind() == Type::kVector_Kind);
        if (fArguments.size() == 1 && fArguments[0]->fType.kind() == Type::kScalar_Kind) {
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

            if (arg->fType.kind() == Type::kScalar_Kind) {
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

            switch (arg->fKind) {
                case kConstructor_Kind: {
                    const Constructor& constructor = static_cast<const Constructor&>(*arg);
                    if (current + constructor.fType.columns() > index) {
                        // We've found a constructor that overlaps the proper argument. Descend into
                        // it, honoring the type.
                        if (constructor.fType.componentType().isFloat()) {
                            return type(constructor.getVecComponent<SKSL_FLOAT>(index - current));
                        } else {
                            return type(constructor.getVecComponent<SKSL_INT>(index - current));
                        }
                    }
                    break;
                }
                case kPrefix_Kind: {
                    const PrefixExpression& prefix = static_cast<const PrefixExpression&>(*arg);
                    if (current + prefix.fType.columns() > index) {
                        // We found a prefix operator that contains the proper argument. Descend
                        // into it. We only support for constant propagation of the unary minus, so
                        // we shouldn't see any other tokens here.
                        SkASSERT(prefix.fOperator == Token::Kind::TK_MINUS);

                        // We expect the - prefix to always be attached to a constructor.
                        SkASSERT(prefix.fOperand->fKind == kConstructor_Kind);
                        const Constructor& constructor =
                                static_cast<const Constructor&>(*prefix.fOperand);

                        // Descend into this constructor, honoring the type.
                        if (constructor.fType.componentType().isFloat()) {
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

            current += arg->fType.columns();
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
        SkASSERT(this->isCompileTimeConstant());
        SkASSERT(fType.kind() == Type::kMatrix_Kind);
        SkASSERT(col < fType.columns() && row < fType.rows());
        if (fArguments.size() == 1) {
            if (fArguments[0]->fType.kind() == Type::kScalar_Kind) {
                // single scalar argument, so matrix is of the form:
                // x 0 0
                // 0 x 0
                // 0 0 x
                // return x if col == row
                return col == row ? fArguments[0]->getConstantFloat() : 0.0;
            }
            if (fArguments[0]->fType.kind() == Type::kMatrix_Kind) {
                SkASSERT(fArguments[0]->fKind == Expression::kConstructor_Kind);
                // single matrix argument. make sure we're within the argument's bounds.
                const Type& argType = ((Constructor&) *fArguments[0]).fType;
                if (col < argType.columns() && row < argType.rows()) {
                    // within bounds, defer to argument
                    return ((Constructor&) *fArguments[0]).getMatComponent(col, row);
                }
                // out of bounds
                return 0.0;
            }
        }
        int currentIndex = 0;
        int targetIndex = col * fType.rows() + row;
        for (const auto& arg : fArguments) {
            SkASSERT(targetIndex >= currentIndex);
            SkASSERT(arg->fType.rows() == 1);
            if (currentIndex + arg->fType.columns() > targetIndex) {
                if (arg->fType.columns() == 1) {
                    return arg->getConstantFloat();
                } else {
                    return arg->getFVecComponent(targetIndex - currentIndex);
                }
            }
            currentIndex += arg->fType.columns();
        }
        ABORT("can't happen, matrix component out of bounds");
    }

    std::vector<std::unique_ptr<Expression>> fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
