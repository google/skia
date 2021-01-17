/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTANT_FOLDER
#define SKSL_CONSTANT_FOLDER

#include <memory>

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
    /** Simplifies the binary expression `left OP right`. Returns null if it can't be simplified. */
    static std::unique_ptr<Expression> Simplify(const Context& context,
                                                const Expression& left,
                                                Token::Kind op,
                                                const Expression& right);
};

}  // namespace SkSL

#endif  // SKSL_CONSTANT_FOLDER
