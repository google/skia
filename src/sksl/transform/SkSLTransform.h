/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSFORM
#define SKSL_TRANSFORM

#include "include/core/SkSpan.h"
#include <memory>
#include <vector>

namespace SkSL {

class Context;
class Expression;
struct LoadedModule;
struct Modifiers;
struct Program;
class ProgramElement;
class ProgramUsage;
class Statement;
class Variable;
enum class ProgramKind : int8_t;

namespace Transform {

/**
 * Copies built-in functions from modules into the program. Relies on ProgramUsage to determine
 * which functions are necessary.
 */
void FindAndDeclareBuiltinFunctions(Program& program);

/**
 * Scans the finished program for built-in variables like `sk_FragColor` and adds them to the
 * program's shared elements.
 */
void FindAndDeclareBuiltinVariables(Program& program);

/**
 * Eliminates statements in a block which cannot be reached; for example, a statement
 * immediately after a `return` or `continue` can safely be eliminated.
 */
void EliminateUnreachableCode(LoadedModule& module, ProgramUsage* usage);
void EliminateUnreachableCode(Program& program);

/**
 * Eliminates empty statements in a module (Nops, or blocks holding only Nops). Not implemented for
 * Programs because Nops are harmless, but they waste space in the dehydrated IR.
 */
void EliminateEmptyStatements(LoadedModule& module);

/**
 * Eliminates functions in a program which are never called. Returns true if any changes were made.
 */
bool EliminateDeadFunctions(Program& program);

/**
 * Eliminates variables in a program which are never read or written (past their initializer).
 * Preserves side effects from initializers, if any. Returns true if any changes were made.
 */
bool EliminateDeadLocalVariables(const Context& context, LoadedModule& module, ProgramUsage* usage);
bool EliminateDeadLocalVariables(Program& program);
bool EliminateDeadGlobalVariables(Program& program);

} // namespace Transform
} // namespace SkSL

#endif
