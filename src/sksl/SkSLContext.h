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
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

struct ProgramConfig;

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(ErrorReporter& errors, const ShaderCapsClass& caps);

    ~Context() {
        SkASSERT(!Pool::IsAttached());
    }

    // Returns the current error handler
    ErrorReporter& errors() const { return fErrors; }

    // The Context holds all of the built-in types.
    BuiltinTypes fTypes;

    // The Context holds a reference to our shader caps bits.
    const ShaderCapsClass& fCaps;

    // The Context holds a pointer to our pool of modifiers.
    ModifiersPool* fModifiersPool = nullptr;

    // The Context holds a pointer to the configuration of the program being compiled.
    ProgramConfig* fConfig = nullptr;

private:
    // The Context holds a reference to our error reporter.
    ErrorReporter& fErrors;
};

}  // namespace SkSL

#endif
