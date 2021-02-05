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
#include "src/sksl/SkSLLexer.h"

namespace SkSL {

class Context;
class ErrorReporter;
class Expression;

/**
 * Performs constant folding on IR expressions. This simplifies expressions containing
 * compile-time constants, such as replacing `IntLiteral(2) + IntLiteral(2)` with `IntLiteral(4)`.
 */
class ConstantFolder {
public:
    /**
     * If value is an int literal or const int variable with a known value, returns true and stores
     * the value in out. Otherwise returns false.
     */
    static bool GetConstantInt(const Expression& value, SKSL_INT* out);

    /**
     * If value is a float literal or const float variable with a known value, returns true and
     * stores the value in out. Otherwise returns false.
     */
    static bool GetConstantFloat(const Expression& value, SKSL_FLOAT* out);

    /**
     * Reports an error and returns true if op is a division / mod operator and right is zero or
     * contains a zero element.
     */
    static bool ErrorOnDivideByZero(const Context& context, int offset, Token::Kind op,
                                    const Expression& right);

    /** Simplifies the binary expression `left OP right`. Returns null if it can't be simplified. */
    static std::unique_ptr<Expression> Simplify(const Context& context,
                                                int offset,
                                                const Expression& left,
                                                Token::Kind op,
                                                const Expression& right);
};

}  // namespace SkSL

#endif  // SKSL_CONSTANT_FOLDER
