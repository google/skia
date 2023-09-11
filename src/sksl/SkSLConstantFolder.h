/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTANT_FOLDER
#define SKSL_CONSTANT_FOLDER

#include <memory>

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLOperator.h"

namespace SkSL {

class Context;
class Expression;
class Position;
class Type;

/**
 * Performs constant folding on IR expressions. This simplifies expressions containing
 * compile-time constants, such as replacing `Literal(2) + Literal(2)` with `Literal(4)`.
 */
class ConstantFolder {
public:
    /**
     * If `value` is an int literal or const int variable with a known value, returns true and
     * stores the value in `out`. Otherwise, returns false.
     */
    static bool GetConstantInt(const Expression& value, SKSL_INT* out);

    /**
     * If `value` is a literal or const scalar variable with a known value, returns true and stores
     * the value in `out`. Otherwise, returns false.
     */
    static bool GetConstantValue(const Expression& value, double* out);

    /**
     * If the expression is a const variable with a known compile-time-constant value, returns that
     * value. If not, returns the original expression as-is.
     */
    static const Expression* GetConstantValueForVariable(const Expression& value);

    /**
     * If the expression can be replaced by a compile-time-constant value, returns that value.
     * If not, returns null.
     */
    static const Expression* GetConstantValueOrNull(const Expression& value);

    /** Returns true if the expression contains `value` in every slot. */
    static bool IsConstantSplat(const Expression& expr, double value);

    /**
     * If the expression is a const variable with a known compile-time-constant value, returns a
     * clone of that value. If not, returns the original expression as-is.
     */
    static std::unique_ptr<Expression> MakeConstantValueForVariable(Position pos,
            std::unique_ptr<Expression> expr);

    /** Simplifies the binary expression `left OP right`. Returns null if it can't be simplified. */
    static std::unique_ptr<Expression> Simplify(const Context& context,
                                                Position pos,
                                                const Expression& left,
                                                Operator op,
                                                const Expression& right,
                                                const Type& resultType);
};

}  // namespace SkSL

#endif  // SKSL_CONSTANT_FOLDER
