/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CORE
#define SKSL_DSL_CORE

#include "src/sksl/SkSLPosition.h"  // IWYU pragma: keep

#include <cstdint>
#include <memory>
#include <string>

namespace SkSL {

enum class ProgramKind : int8_t;

class Compiler;
class ErrorReporter;
struct Module;
struct Program;
struct ProgramSettings;

namespace dsl {

/**
 * Starts compilation on the current thread using the specified compiler.
 */

void Start(SkSL::Compiler* compiler, SkSL::ProgramKind kind, const SkSL::ProgramSettings& settings);

/**
 * Starts compilation of an SkSL module (SkSL include files).
 */
void StartModule(SkSL::Compiler* compiler,
                 SkSL::ProgramKind kind,
                 const SkSL::ProgramSettings& settings,
                 const SkSL::Module* parent);

/**
 * Signals the end of compilation. This must be called sometime between a call to Start() and the
 * termination of the thread.
 */
void End();

/**
 * Returns all global elements (functions and global variables) as a self-contained Program. The
 * optional source string is retained as the program's source.
 */
std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source = nullptr);

/**
 * Returns the ErrorReporter which will be notified of any errors that occur during compilation. The
 * default error reporter aborts on any error.
 */
ErrorReporter& GetErrorReporter();

/**
 * Installs an ErrorReporter which will be notified of any errors that occur during compilation.
 */
void SetErrorReporter(ErrorReporter* errorReporter);

}  // namespace dsl
}  // namespace SkSL

#endif
