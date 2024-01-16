/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include "include/private/base/SkAssert.h"

namespace SkSL {

class BuiltinTypes;
class ErrorReporter;
struct Module;
struct ProgramConfig;
class SymbolTable;

/**
 * Contains compiler-wide objects and state.
 */
class Context {
public:
    Context(const BuiltinTypes& types, ErrorReporter& errors);
    ~Context();

    // The Context holds a reference to all of the built-in types.
    const BuiltinTypes& fTypes;

    // The Context holds a pointer to the configuration of the program being compiled.
    ProgramConfig* fConfig = nullptr;

    // The Context holds a pointer to our error reporter.
    ErrorReporter* fErrors;

    void setErrorReporter(ErrorReporter* e) {
        SkASSERT(e);
        fErrors = e;
    }

    // The Context holds a pointer to our module with built-in declarations.
    const Module* fModule = nullptr;

    // This is the current symbol table of the code we are processing, and therefore changes during
    // compilation.
    SymbolTable* fSymbolTable = nullptr;
};

}  // namespace SkSL

#endif
