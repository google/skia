/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EXPRESSION
#define SKSL_DSL_EXPRESSION

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace SkSL::dsl {

class DSLType;

/**
 * Represents an expression such as 'cos(x)' or 'a + b'.
 */
class DSLExpression {
public:
    DSLExpression() = default;
    ~DSLExpression() = default;

    DSLExpression(DSLExpression&&) = default;
    DSLExpression& operator=(DSLExpression&&) = default;

    DSLExpression(const DSLExpression&) = delete;
    DSLExpression& operator=(const DSLExpression&) = delete;

    // If expression is null, returns Poison.
    explicit DSLExpression(std::unique_ptr<SkSL::Expression> expression, Position pos = {});

    static DSLExpression Poison(Position pos = {});

    DSLType type() const;

    std::string description() const;

    Position position() const;

    void setPosition(Position pos);

    /**
     * Returns true if this object contains an expression. DSLExpressions which were created with
     * the empty constructor or which have already been release()ed do not have a value.
     * DSLExpressions created with errors are still considered to have a value (but contain poison).
     */
    bool hasValue() const {
        return fExpression != nullptr;
    }

    /**
     * Returns true if this object contains an expression which is not poison.
     */
    bool isValid() const;

    /**
     * Invalidates this object and returns the SkSL expression it represents. It is an error to call
     * this on an invalid DSLExpression.
     */
    std::unique_ptr<SkSL::Expression> release() {
        SkASSERT(this->hasValue());
        return std::move(fExpression);
    }

    /**
     * Calls release if this expression has a value, otherwise returns null.
     */
    std::unique_ptr<SkSL::Expression> releaseIfPossible() {
        return std::move(fExpression);
    }

private:
    std::unique_ptr<SkSL::Expression> fExpression;
};

}  // namespace SkSL::dsl

template <typename T> struct sk_is_trivially_relocatable;

template <>
struct sk_is_trivially_relocatable<SkSL::dsl::DSLExpression> : std::true_type {};

#endif
