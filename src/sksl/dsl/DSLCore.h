/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CORE
#define SKSL_DSL_CORE

#include "include/private/SkSLDefines.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/dsl/DSLVar.h"  // IWYU pragma: keep

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Compiler;
class ErrorReporter;
class Position;
struct Program;
struct ProgramSettings;

namespace dsl {

class DSLField;
class DSLModifiers;

// When users import the DSL namespace via `using namespace SkSL::dsl`, we want the SwizzleComponent
// Type enum to come into scope as well, so `Swizzle(var, X, Y, ONE)` can work as expected.
// `namespace SkSL::SwizzleComponent` contains only an `enum Type`; this `using namespace` directive
// shouldn't pollute the SkSL::dsl namespace with anything else.
using namespace SkSL::SwizzleComponent;

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
 * #extension <name> : enable
 */
void AddExtension(std::string_view name, Position pos = {});

/**
 * Creates a local variable declaration statement.
 */
DSLStatement Declare(DSLVar& var, Position pos = {});

/**
 * Creates a local variable declaration statement containing multiple variables.
 */
DSLStatement Declare(skia_private::TArray<DSLVar>& vars, Position pos = {});

/**
 * Declares a global variable.
 */
void Declare(DSLGlobalVar& var, Position pos = {});

/**
 * Declares a set of global variables.
 */
void Declare(skia_private::TArray<DSLGlobalVar>& vars, Position pos = {});

DSLExpression InterfaceBlock(const DSLModifiers& modifiers,  std::string_view typeName,
                             skia_private::TArray<DSLField> fields, std::string_view varName = "",
                             int arraySize = 0, Position pos = {});

/**
 * test ? ifTrue : ifFalse
 */
DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
                     Position  = {});

/**
 * expression.xyz1
 */
DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      Position pos = {},
                      Position maskPos = {});

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      Position pos = {},
                      Position maskPos = {});

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      Position pos = {},
                      Position maskPos = {});

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      SkSL::SwizzleComponent::Type d,
                      Position pos = {},
                      Position maskPos = {});

} // namespace dsl

} // namespace SkSL

#endif
