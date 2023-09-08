/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace SkSL {

class Context;
enum class OperatorPrecedence : uint8_t;

/**
 * Base class representing a constructor with unknown arguments.
 */
class AnyConstructor : public Expression {
public:
    AnyConstructor(Position pos, Kind kind, const Type* type)
            : INHERITED(pos, kind, type) {}

    virtual SkSpan<std::unique_ptr<Expression>> argumentSpan() = 0;
    virtual SkSpan<const std::unique_ptr<Expression>> argumentSpan() const = 0;

    std::string description(OperatorPrecedence) const override;

    const Type& componentType() const {
        return this->type().componentType();
    }

    bool supportsConstantValues() const override { return true; }
    std::optional<double> getConstantValue(int n) const override;

    ComparisonResult compareConstant(const Expression& other) const override;

private:
    using INHERITED = Expression;
};

/**
 * Base class representing a constructor that takes a single argument.
 */
class SingleArgumentConstructor : public AnyConstructor {
public:
    SingleArgumentConstructor(Position pos, Kind kind, const Type* type,
                              std::unique_ptr<Expression> argument)
            : INHERITED(pos, kind, type)
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
    MultiArgumentConstructor(Position pos, Kind kind, const Type* type,
            ExpressionArray arguments)
        : INHERITED(pos, kind, type)
        , fArguments(std::move(arguments)) {}

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
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
 * Converts any GLSL constructor, such as `float2(x, y)` or `mat3x3(otherMat)` or `int[2](0, i)`, to
 * an SkSL expression.
 *
 * Vector constructors must always consist of either exactly 1 scalar, or a collection of vectors
 * and scalars totaling exactly the right number of scalar components.
 *
 * Matrix constructors must always consist of either exactly 1 scalar, exactly 1 matrix, or a
 * collection of vectors and scalars totaling exactly the right number of scalar components.
 *
 * Array constructors must always contain the proper number of array elements (matching the Type).
 */
namespace Constructor {
    // Creates, typechecks and simplifies constructor expressions. Reports errors via the
    // ErrorReporter. This can return null on error, so be careful. There are several different
    // Constructor expression types; this class chooses the proper one based on context, e.g.
    // `ConstructorCompound`, `ConstructorScalarCast`, or `ConstructorMatrixResize`.
    std::unique_ptr<Expression> Convert(const Context& context,
                                        Position pos,
                                        const Type& type,
                                        ExpressionArray args);
}

}  // namespace SkSL

#endif
