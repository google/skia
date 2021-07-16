/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EXPRESSION
#define SKSL_DSL_EXPRESSION

#include "include/core/SkStringView.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/sksl/DSLErrorHandling.h"
#include "include/sksl/DSLWrapper.h"

#include <cstdint>
#include <memory>

namespace SkSL {

class Expression;
class Type;

namespace dsl {

class DSLPossibleExpression;
class DSLStatement;
class DSLType;
class DSLVarBase;

/**
 * Represents an expression such as 'cos(x)' or 'a + b'.
 */
class DSLExpression {
public:
    DSLExpression(const DSLExpression&) = delete;

    DSLExpression(DSLExpression&&);

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
     * Creates an expression representing a literal int.
     */
    DSLExpression(int64_t value);

    /**
     * Creates an expression representing a literal uint.
     */
    DSLExpression(unsigned int value);

    /**
     * Creates an expression representing a literal bool.
     */
    DSLExpression(bool value);

    /**
     * Creates an expression representing a variable reference.
     */
    DSLExpression(DSLVarBase& var);

    DSLExpression(DSLVarBase&& var);

    DSLExpression(DSLPossibleExpression expr, PositionInfo pos = PositionInfo());

    explicit DSLExpression(std::unique_ptr<SkSL::Expression> expression);

    ~DSLExpression();

    DSLType type();

    /**
     * Overloads the '=' operator to create an SkSL assignment statement.
     */
    DSLPossibleExpression operator=(DSLExpression other);

    DSLExpression x(PositionInfo pos = PositionInfo());

    DSLExpression y(PositionInfo pos = PositionInfo());

    DSLExpression z(PositionInfo pos = PositionInfo());

    DSLExpression w(PositionInfo pos = PositionInfo());

    DSLExpression r(PositionInfo pos = PositionInfo());

    DSLExpression g(PositionInfo pos = PositionInfo());

    DSLExpression b(PositionInfo pos = PositionInfo());

    DSLExpression a(PositionInfo pos = PositionInfo());

    /**
     * Creates an SkSL struct field access expression.
     */
    DSLExpression field(skstd::string_view name, PositionInfo pos = PositionInfo());

    /**
     * Creates an SkSL array index expression.
     */
    DSLPossibleExpression operator[](DSLExpression index);

    DSLPossibleExpression operator()(SkTArray<DSLWrapper<DSLExpression>> args);

    /**
     * Returns true if this object contains an expression. DSLExpressions which were created with
     * the empty constructor or which have already been release()ed are not valid. DSLExpressions
     * created with errors are still considered valid (but contain a poison value).
     */
    bool valid() const {
        return fExpression != nullptr;
    }

    void swap(DSLExpression& other);

    /**
     * Invalidates this object and returns the SkSL expression it represents. It is an error to call
     * this on an invalid DSLExpression.
     */
    std::unique_ptr<SkSL::Expression> release();

private:
    /**
     * Calls release if this expression is valid, otherwise returns null.
     */
    std::unique_ptr<SkSL::Expression> releaseIfValid();

    /**
     * Invalidates this object and returns the SkSL expression it represents coerced to the
     * specified type. If the expression cannot be coerced, reports an error and returns null.
     */
    std::unique_ptr<SkSL::Expression> coerceAndRelease(const SkSL::Type& type);

    std::unique_ptr<SkSL::Expression> fExpression;

    friend DSLExpression SampleChild(int index, DSLExpression coords);

    friend class DSLCore;
    friend class DSLFunction;
    friend class DSLPossibleExpression;
    friend class DSLVarBase;
    friend class DSLWriter;
    template<typename T> friend class DSLWrapper;
};

DSLPossibleExpression operator+(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator+(DSLExpression expr);
DSLPossibleExpression operator+=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator-(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator-(DSLExpression expr);
DSLPossibleExpression operator-=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator*(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator*=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator/(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator/=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator%(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator%=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator<<(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator<<=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator>>(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator>>=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator&&(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator||(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator&(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator&=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator|(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator|=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator^(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator^=(DSLExpression left, DSLExpression right);
DSLPossibleExpression LogicalXor(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator,(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator,(DSLPossibleExpression left, DSLExpression right);
DSLPossibleExpression operator,(DSLExpression left, DSLPossibleExpression right);
DSLPossibleExpression operator,(DSLPossibleExpression left, DSLPossibleExpression right);
DSLPossibleExpression operator==(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator!=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator>(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator<(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator>=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator<=(DSLExpression left, DSLExpression right);
DSLPossibleExpression operator!(DSLExpression expr);
DSLPossibleExpression operator~(DSLExpression expr);
DSLPossibleExpression operator++(DSLExpression expr);
DSLPossibleExpression operator++(DSLExpression expr, int);
DSLPossibleExpression operator--(DSLExpression expr);
DSLPossibleExpression operator--(DSLExpression expr, int);

/**
 * Represents an Expression which may have failed and/or have pending errors to report. Converting a
 * PossibleExpression into an Expression requires PositionInfo so that any pending errors can be
 * reported at the correct position.
 *
 * PossibleExpression is used instead of Expression in situations where it is not possible to
 * capture the PositionInfo at the time of Expression construction (notably in operator overloads,
 * where we cannot add default parameters).
 */
class DSLPossibleExpression {
public:
    DSLPossibleExpression(std::unique_ptr<SkSL::Expression> expression);

    DSLPossibleExpression(DSLPossibleExpression&& other);

    ~DSLPossibleExpression();

    bool valid() const {
        return fExpression != nullptr;
    }

    /**
     * Reports any pending errors at the specified position.
     */
    void reportErrors(PositionInfo pos);

    DSLType type();

    DSLExpression x(PositionInfo pos = PositionInfo());

    DSLExpression y(PositionInfo pos = PositionInfo());

    DSLExpression z(PositionInfo pos = PositionInfo());

    DSLExpression w(PositionInfo pos = PositionInfo());

    DSLExpression r(PositionInfo pos = PositionInfo());

    DSLExpression g(PositionInfo pos = PositionInfo());

    DSLExpression b(PositionInfo pos = PositionInfo());

    DSLExpression a(PositionInfo pos = PositionInfo());

    DSLExpression field(skstd::string_view name, PositionInfo pos = PositionInfo());

    DSLPossibleExpression operator=(DSLExpression expr);

    DSLPossibleExpression operator=(int expr);

    DSLPossibleExpression operator=(float expr);

    DSLPossibleExpression operator=(double expr);

    DSLPossibleExpression operator[](DSLExpression index);

    DSLPossibleExpression operator()(SkTArray<DSLWrapper<DSLExpression>> args);

    DSLPossibleExpression operator++();

    DSLPossibleExpression operator++(int);

    DSLPossibleExpression operator--();

    DSLPossibleExpression operator--(int);

    std::unique_ptr<SkSL::Expression> release(PositionInfo pos = PositionInfo());

private:
    std::unique_ptr<SkSL::Expression> fExpression;

    friend class DSLExpression;
};

} // namespace dsl

} // namespace SkSL

#endif
