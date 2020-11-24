/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "include/private/SkTArray.h"
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
class Constructor final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructor;

    Constructor(int offset, const Type* type, ExpressionArray arguments)
        : INHERITED(offset, kExpressionKind, type)
        , fArguments(std::move(arguments)) {}

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    bool hasProperty(Property property) const override {
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        ExpressionArray cloned;
        cloned.reserve_back(this->arguments().size());
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            cloned.push_back(arg->clone());
        }
        return std::make_unique<Constructor>(fOffset, &this->type(), std::move(cloned));
    }

    String description() const override {
        String result = this->type().description() + "(";
        const char* separator = "";
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            result += separator;
            result += arg->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    const Type& componentType() const {
        // Returns `float` for constructors of type `float(...)` or `floatN(...)`.
        const Type& type = this->type();
        return type.columns() == 1 ? type : type.componentType();
    }

    bool isCompileTimeConstant() const override {
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            if (!arg->isCompileTimeConstant()) {
                return false;
            }
        }
        return true;
    }

    bool isConstantOrUniform() const override {
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            if (!arg->isConstantOrUniform()) {
                return false;
            }
        }
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override;

    template <typename resultType>
    resultType getVecComponent(int index) const;

    /**
     * For a literal vector expression, return the float value of the n'th vector component. It is
     * an error to call this method on an expression which is not a vector of FloatLiterals.
     */
    SKSL_FLOAT getFVecComponent(int n) const override {
        return this->getVecComponent<SKSL_FLOAT>(n);
    }

    /**
     * For a literal vector expression, return the integer value of the n'th vector component. It is
     * an error to call this method on an expression which is not a vector of IntLiterals.
     */
    SKSL_INT getIVecComponent(int n) const override {
        return this->getVecComponent<SKSL_INT>(n);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override;

    int64_t getConstantInt() const override;

    SKSL_FLOAT getConstantFloat() const override;

private:
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
