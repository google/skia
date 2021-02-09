/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EXPRESSION
#define SKSL_DSL_EXPRESSION

#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>

namespace SkSL {

class Expression;

namespace dsl {

class DSLStatement;
class DSLVar;

/**
 * Represents an expression such as 'cos(x)' or 'a + b'.
 */
class DSLExpression {
public:
    DSLExpression(const DSLExpression&) = delete;

    DSLExpression(DSLExpression&&) = default;

    DSLExpression();

    /**
     * Creates an expression representing a literal float.
     */
    DSLExpression(float value);

    /**
     * Creates an expression representing a literal float.
     */
    DSLExpression(double value)
        : DSLExpression((float) value) {}

    /**
     * Creates an expression representing a literal int.
     */
    DSLExpression(int value);

    /**
     * Creates an expression representing a literal bool.
     */
    DSLExpression(bool value);

    /**
     * Creates an expression representing a variable reference.
     */
    DSLExpression(const DSLVar& var);

    ~DSLExpression();

    /**
     * Overloads the '=' operator to create an SkSL assignment statement.
     */
    DSLExpression operator=(DSLExpression other);

    DSLExpression x();

    DSLExpression y();

    DSLExpression z();

    DSLExpression w();

    DSLExpression r();

    DSLExpression g();

    DSLExpression b();

    DSLExpression a();

    /**
     * Creates an SkSL array index expression.
     */
    DSLExpression operator[](DSLExpression index);

    /**
     * Invalidates this object and returns the SkSL expression it represents.
     */
    std::unique_ptr<SkSL::Expression> release();

private:
    DSLExpression(std::unique_ptr<SkSL::Expression> expression);

    /**
     * Invalidates this object and returns the SkSL expression it represents coerced to the
     * specified type. If the expression cannot be coerced, reports an error and returns null.
     */
    std::unique_ptr<SkSL::Expression> coerceAndRelease(const SkSL::Type& type);

    std::unique_ptr<SkSL::Expression> fExpression;

    template<class... Cases>
    friend DSLStatement Switch(DSLExpression value, Cases... cases);

    friend class DSLCore;
    friend class DSLVar;
    friend class DSLWriter;
};

DSLExpression operator+(DSLExpression left, DSLExpression right);
DSLExpression operator+=(DSLExpression left, DSLExpression right);
DSLExpression operator-(DSLExpression left, DSLExpression right);
DSLExpression operator-=(DSLExpression left, DSLExpression right);
DSLExpression operator*(DSLExpression left, DSLExpression right);
DSLExpression operator*=(DSLExpression left, DSLExpression right);
DSLExpression operator/(DSLExpression left, DSLExpression right);
DSLExpression operator/=(DSLExpression left, DSLExpression right);
DSLExpression operator%(DSLExpression left, DSLExpression right);
DSLExpression operator%=(DSLExpression left, DSLExpression right);
DSLExpression operator<<(DSLExpression left, DSLExpression right);
DSLExpression operator<<=(DSLExpression left, DSLExpression right);
DSLExpression operator>>(DSLExpression left, DSLExpression right);
DSLExpression operator>>=(DSLExpression left, DSLExpression right);
DSLExpression operator&&(DSLExpression left, DSLExpression right);
DSLExpression operator||(DSLExpression left, DSLExpression right);
DSLExpression operator&(DSLExpression left, DSLExpression right);
DSLExpression operator&=(DSLExpression left, DSLExpression right);
DSLExpression operator|(DSLExpression left, DSLExpression right);
DSLExpression operator|=(DSLExpression left, DSLExpression right);
DSLExpression operator^(DSLExpression left, DSLExpression right);
DSLExpression operator^=(DSLExpression left, DSLExpression right);
DSLExpression operator,(DSLExpression left, DSLExpression right);
DSLExpression operator==(DSLExpression left, DSLExpression right);
DSLExpression operator!=(DSLExpression left, DSLExpression right);
DSLExpression operator>(DSLExpression left, DSLExpression right);
DSLExpression operator<(DSLExpression left, DSLExpression right);
DSLExpression operator>=(DSLExpression left, DSLExpression right);
DSLExpression operator<=(DSLExpression left, DSLExpression right);
DSLExpression operator!(DSLExpression expr);
DSLExpression operator~(DSLExpression expr);
DSLExpression operator++(DSLExpression expr);
DSLExpression operator++(DSLExpression expr, int);
DSLExpression operator--(DSLExpression expr);
DSLExpression operator--(DSLExpression expr, int);

} // namespace dsl

} // namespace SkSL

#endif
