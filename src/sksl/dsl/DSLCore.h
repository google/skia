/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CORE
#define SKSL_DSL_CORE

#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLVar.h"  // IWYU pragma: keep

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Compiler;
class ErrorReporter;
struct Field;
class Position;
struct Program;
struct ProgramSettings;

namespace dsl {

class DSLModifiers;

/**
 * Starts DSL output on the current thread using the specified compiler. This must be called
 * prior to any other DSL functions.
 */
void Start(SkSL::Compiler* compiler, SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment);

void Start(SkSL::Compiler* compiler, SkSL::ProgramKind kind, const SkSL::ProgramSettings& settings);

/**
 * Signals the end of DSL output. This must be called sometime between a call to Start() and the
 * termination of the thread.
 */
void End();

/**
 * Returns all global elements (functions and global variables) as a self-contained Program. The
 * optional source string is retained as the program's source. DSL programs do not normally have
 * sources, but when a DSL program is produced from parsed program text (as in Parser), it may be
 * important to retain it so that any std::string_views derived from it remain valid.
 */
std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source = nullptr);

/**
 * Returns the ErrorReporter which will be notified of any errors that occur during DSL calls. The
 * default error reporter aborts on any error.
 */
ErrorReporter& GetErrorReporter();

/**
 * Installs an ErrorReporter which will be notified of any errors that occur during DSL calls.
 */
void SetErrorReporter(ErrorReporter* errorReporter);

/**
 * Declares a global variable.
 */
void Declare(DSLGlobalVar& var, Position pos = {});

DSLExpression InterfaceBlock(const DSLModifiers& modifiers,  std::string_view typeName,
                             skia_private::TArray<Field> fields, std::string_view varName = "",
                             int arraySize = 0, Position pos = {});

} // namespace dsl

} // namespace SkSL

#endif
