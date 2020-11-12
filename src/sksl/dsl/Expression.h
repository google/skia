/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EXPRESSION
#define SKSL_DSL_EXPRESSION

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace skslcode {

class Var;

class Expression {
public:
    Expression(Expression&&) = default;

    Expression(std::unique_ptr<SkSL::Expression> expression);

    Expression(float value);

    Expression(double value)
        : Expression((float) value) {}

    Expression(int value);

    Expression(const Var& var);

    Expression operator=(Expression&& other);

    Expression operator[](Expression&& index);

    std::unique_ptr<SkSL::Expression> release() {
        return std::move(fExpression);
    }

    std::unique_ptr<SkSL::Expression> coerceAndRelease(const SkSL::Type& type);

private:
    Expression() {}

    std::unique_ptr<SkSL::Expression> fExpression;

    friend class Var;
};

Expression operator+(Expression left, Expression right);
Expression operator+=(Expression left, Expression right);
Expression operator+=(Var& left, Expression right);
Expression operator-(Expression left, Expression right);
Expression operator-=(Expression left, Expression right);
Expression operator-=(Var& left, Expression right);
Expression operator*(Expression left, Expression right);
Expression operator*=(Expression left, Expression right);
Expression operator*=(Var& left, Expression right);
Expression operator/(Expression left, Expression right);
Expression operator/=(Expression left, Expression right);
Expression operator/=(Var& left, Expression right);
Expression operator==(Expression left, Expression right);
Expression operator!=(Expression left, Expression right);
Expression operator>(Expression left, Expression right);
Expression operator<(Expression left, Expression right);
Expression operator>=(Expression left, Expression right);
Expression operator<=(Expression left, Expression right);

} // namespace skslcode

#endif
