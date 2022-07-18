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
 * Checks to see if it would be safe to add `const` to the modifiers of a variable. If so, returns
 * the modifiers with `const` applied; if not, returns the existing modifiers as-is. Adding `const`
 * allows the inliner to fold away more values and generate tighter code.
 */
const Modifiers* AddConstToVarModifiers(const Context& context,
                                        const Variable& var,
                                        const Expression* initialValue,
                                        const ProgramUsage* usage);

/**
 * Scans the finished program for built-in variables like `sk_FragColor` and adds them to the
 * program's shared elements.
 */
void FindAndDeclareBuiltinVariables(const Context& context, ProgramKind programKind,
                                    std::vector<const ProgramElement*>& sharedElements);

/**
 * Eliminates statements in a block which cannot be reached; for example, a statement
 * immediately after a `return` or `continue` can safely be eliminated.
 */
void EliminateUnreachableCode(LoadedModule& module, ProgramUsage* usage);
void EliminateUnreachableCode(Program& program, ProgramUsage* usage);

/**
 * Eliminates empty statements in a module (Nops, or blocks holding only Nops). Not implemented for
 * Programs because Nops are harmless, but they waste space in the dehydrated IR.
 */
void EliminateEmptyStatements(LoadedModule& module);

/**
 * Eliminates functions in a program which are never called. Returns true if any changes were made.
 */
bool EliminateDeadFunctions(Program& program, ProgramUsage* usage);

/**
 * Eliminates variables in a program which are never read or written (past their initializer).
 * Preserves side effects from initializers, if any. Returns true if any changes were made.
 */
bool EliminateDeadLocalVariables(const Context& context, LoadedModule& module, ProgramUsage* usage);
bool EliminateDeadLocalVariables(Program& program, ProgramUsage* usage);
bool EliminateDeadGlobalVariables(Program& program, ProgramUsage* usage);

} // namespace Transform
} // namespace SkSL

#endif
