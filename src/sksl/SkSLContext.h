/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

namespace SkSL {

class BuiltinTypes;
class ErrorReporter;
class ModifiersPool;
struct Module;
struct ProgramConfig;
struct ShaderCaps;

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(const BuiltinTypes& types, const ShaderCaps* caps, ErrorReporter& errors);
    ~Context();

    // The Context holds a reference to all of the built-in types.
    const BuiltinTypes& fTypes;

    // The Context holds a reference to our shader caps bits.
    const ShaderCaps* fCaps;

    // The Context holds a pointer to our pool of modifiers.
    ModifiersPool* fModifiersPool = nullptr;

    // The Context holds a pointer to the configuration of the program being compiled.
    ProgramConfig* fConfig = nullptr;

    // The Context holds a pointer to our error reporter.
    ErrorReporter* fErrors;

    // The Context holds a pointer to our module with built-in declarations.
    const Module* fModule = nullptr;
};

}  // namespace SkSL

#endif
