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
class Statement;
class Type;

namespace dsl {

class DSLExpression;
class DSLStatement;

enum SwizzleComponent : int8_t {
    R,
    G,
    B,
    A,
    X,
    Y,
    Z,
    W,
    ZERO,
    ONE
};

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

    ~DSLExpression();

    /**
     * Overloads the '=' operator to create an SkSL assignment statement.
     */
    DSLExpression operator=(DSLExpression&& other);

    /**
     * Creates an SkSL array index expression.
     */
    DSLExpression operator[](DSLExpression&& index);

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a);

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b);

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                                 SwizzleComponent c);

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                                 SwizzleComponent c, SwizzleComponent d);

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

    template <typename... Args>
    friend DSLExpression dsl_function(const char* name, Args... args);

    friend class DSLBlock;
    friend class DSLStatement;
    friend class DSLWriter;

    friend DSLExpression operator+(DSLExpression left, DSLExpression right);
    friend DSLExpression operator+=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator-(DSLExpression left, DSLExpression right);
    friend DSLExpression operator-=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator*(DSLExpression left, DSLExpression right);
    friend DSLExpression operator*=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator/(DSLExpression left, DSLExpression right);
    friend DSLExpression operator/=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator%(DSLExpression left, DSLExpression right);
    friend DSLExpression operator%=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator<<(DSLExpression left, DSLExpression right);
    friend DSLExpression operator<<=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator>>(DSLExpression left, DSLExpression right);
    friend DSLExpression operator>>=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator&&(DSLExpression left, DSLExpression right);
    friend DSLExpression operator||(DSLExpression left, DSLExpression right);
    friend DSLExpression operator&(DSLExpression left, DSLExpression right);
    friend DSLExpression operator&=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator|(DSLExpression left, DSLExpression right);
    friend DSLExpression operator|=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator^(DSLExpression left, DSLExpression right);
    friend DSLExpression operator^=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator,(DSLExpression left, DSLExpression right);
    friend DSLExpression operator==(DSLExpression left, DSLExpression right);
    friend DSLExpression operator!=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator>(DSLExpression left, DSLExpression right);
    friend DSLExpression operator<(DSLExpression left, DSLExpression right);
    friend DSLExpression operator>=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator<=(DSLExpression left, DSLExpression right);
    friend DSLExpression operator!(DSLExpression expr);
    friend DSLExpression operator~(DSLExpression expr);
    friend DSLExpression operator++(DSLExpression expr);
    friend DSLExpression operator++(DSLExpression expr, int);
    friend DSLExpression operator--(DSLExpression expr);
    friend DSLExpression operator--(DSLExpression expr, int);
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
