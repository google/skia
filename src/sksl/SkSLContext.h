/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include <memory>

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(ErrorReporter& errors);

    // The Context holds all of the built-in types.
    BuiltinTypes fTypes;

    // The Context holds a reference to our error reporter.
    ErrorReporter& fErrors;

    // A sentinel expression used to mark that a variable has a value during dataflow analysis (when
    // it could have several different values, or the analyzer is otherwise unable to assign it a
    // specific expression)
    const std::unique_ptr<Expression> fDefined_Expression;
};

}  // namespace SkSL

#endif
