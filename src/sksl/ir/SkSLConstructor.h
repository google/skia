/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "include/private/SkTArray.h"
#include "src/core/SkSpan.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <algorithm>

namespace SkSL {

/**
 * Base class representing a constructor with unknown arguments.
 */
class AnyConstructor : public Expression {
public:
    AnyConstructor(int offset, Kind kind, const Type* type)
            : INHERITED(offset, kind, type) {}

    virtual SkSpan<std::unique_ptr<Expression>> argumentSpan() = 0;
    virtual SkSpan<const std::unique_ptr<Expression>> argumentSpan() const = 0;

    bool hasProperty(Property property) const override {
        for (const std::unique_ptr<Expression>& arg : this->argumentSpan()) {
            if (arg->hasProperty(property)) {
                return true;
            }
        }
        return false;
    }

    String description() const override {
        String result = this->type().description() + "(";
        const char* separator = "";
        for (const std::unique_ptr<Expression>& arg : this->argumentSpan()) {
            result += separator;
            result += arg->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    const Type& componentType() const {
        return this->type().componentType();
    }

    bool isCompileTimeConstant() const override {
        for (const std::unique_ptr<Expression>& arg : this->argumentSpan()) {
            if (!arg->isCompileTimeConstant()) {
                return false;
            }
        }
        return true;
    }

    bool isConstantOrUniform() const override {
        for (const std::unique_ptr<Expression>& arg : this->argumentSpan()) {
            if (!arg->isConstantOrUniform()) {
                return false;
            }
        }
        return true;
    }

private:
    std::unique_ptr<Expression> fArgument;

    using INHERITED = Expression;
};

/**
 * Base class representing a constructor that takes a single argument.
 */
class SingleArgumentConstructor : public AnyConstructor {
public:
    SingleArgumentConstructor(int offset, Kind kind, const Type* type,
                              std::unique_ptr<Expression> argument)
            : INHERITED(offset, kind, type)
            , fArgument(std::move(argument)) {}

    std::unique_ptr<Expression>& argument() {
        return fArgument;
    }

    const std::unique_ptr<Expression>& argument() const {
        return fArgument;
    }

    SkSpan<std::unique_ptr<Expression>> argumentSpan() final {
        return {&fArgument, 1};
    }

    SkSpan<const std::unique_ptr<Expression>> argumentSpan() const final {
        return {&fArgument, 1};
    }

private:
    std::unique_ptr<Expression> fArgument;

    using INHERITED = AnyConstructor;
};

/**
 * Base class representing a constructor that takes an array of arguments.
 */
class MultiArgumentConstructor : public AnyConstructor {
public:
    MultiArgumentConstructor(int offset, Kind kind, const Type* type, ExpressionArray arguments)
            : INHERITED(offset, kind, type)
            , fArguments(std::move(arguments)) {}

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    ExpressionArray cloneArguments() const {
        ExpressionArray clonedArgs;
        clonedArgs.reserve_back(this->arguments().size());
        for (const std::unique_ptr<Expression>& arg: this->arguments()) {
            clonedArgs.push_back(arg->clone());
        }
        return clonedArgs;
    }

    SkSpan<std::unique_ptr<Expression>> argumentSpan() final {
        return {&fArguments.front(), fArguments.size()};
    }

    SkSpan<const std::unique_ptr<Expression>> argumentSpan() const final {
        return {&fArguments.front(), fArguments.size()};
    }

private:
    ExpressionArray fArguments;

    using INHERITED = AnyConstructor;
};

/**
 * Represents any GLSL constructor, such as `float2(x, y)` or `mat3x3(otherMat)` or `int[2](0, i)`.
 *
 * Vector constructors will always consist of either exactly 1 scalar, or a collection of vectors
 * and scalars totaling exactly the right number of scalar components.
 *
 * Matrix constructors will always consist of either exactly 1 scalar, exactly 1 matrix, or a
 * collection of vectors and scalars totaling exactly the right number of scalar components.
 *
 * Array constructors will always contain the proper number of array elements (matching the Type).
 *
 * TODO(skia:11032): this class will be replaced by several single-purpose Constructor objects.
 */
class Constructor final : public MultiArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructor;

    Constructor(int offset, const Type& type, ExpressionArray arguments)
        : INHERITED(offset, kExpressionKind, &type, std::move(arguments)) {}

    // Use Constructor::Convert to create, typecheck and simplify constructor expressions.
    // Reports errors via the ErrorReporter. This can return null on error, so be careful.
    // TODO(skia:11032): Unlike most Expressions, there isn't a failsafe Constructor::Make which
    // always returns an IRNode, because Constructor creation is currently quite complex and
    // duplicating big chunks of its logic isn't worth it. Splitting up Constructor would help.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int offset,
                                               const Type& type,
                                               ExpressionArray args);

    // If the passed-in expression is a literal, performs a constructor-conversion of the literal
    // value to the constructor's type and returns that converted value as a new literal. e.g., the
    // constructor expression `short(3.14)` would be represented as `FloatLiteral(3.14)` along with
    // type `Short`, and this would result in `IntLiteral(3, type=Short)`. Returns nullptr if the
    // expression is not a literal or the conversion cannot be made.
    static std::unique_ptr<Expression> SimplifyConversion(const Type& constructorType,
                                                          const Expression& expr);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Constructor>(fOffset, this->type(), this->cloneArguments());
    }

    ComparisonResult compareConstant(const Expression& other) const override;

    template <typename ResultType>
    ResultType getVecComponent(int index) const;

    /**
     * For a literal vector expression, return the float value of the n'th vector component. It is
     * an error to call this method on an expression which is not a compile-time constant vector of
     * floating-point type.
     */
    SKSL_FLOAT getFVecComponent(int n) const override {
        return this->getVecComponent<SKSL_FLOAT>(n);
    }

    /**
     * For a literal vector expression, return the integer value of the n'th vector component. It is
     * an error to call this method on an expression which is not a compile-time constant vector of
     * integer type.
     */
    SKSL_INT getIVecComponent(int n) const override {
        return this->getVecComponent<SKSL_INT>(n);
    }

    /**
     * For a literal vector expression, return the boolean value of the n'th vector component. It is
     * an error to call this method on an expression which is not a compile-time constant vector of
     * Boolean type.
     */
    bool getBVecComponent(int n) const override {
        return this->getVecComponent<bool>(n);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override;

    SKSL_INT getConstantInt() const override;

    SKSL_FLOAT getConstantFloat() const override;

    bool getConstantBool() const override;

private:
    static std::unique_ptr<Expression> MakeScalarConstructor(const Context& context,
                                                             int offset,
                                                             const Type& type,
                                                             ExpressionArray args);

    static std::unique_ptr<Expression> MakeCompoundConstructor(const Context& context,
                                                               int offset,
                                                               const Type& type,
                                                               ExpressionArray args);

    template <typename ResultType> ResultType getConstantValue(const Expression& expr) const;

    template <typename ResultType>
    ResultType getInnerVecComponent(const Expression& expr, int position) const;

    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
