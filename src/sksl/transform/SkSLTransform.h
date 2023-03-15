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
class IndexExpression;
struct Modifiers;
struct Module;
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
 * Rewrites indexed swizzles of the form `myVec.zyx[i]` by replacing the swizzle with a lookup into
 * a constant vector. e.g., the above expression would be rewritten as `myVec[vec3(2, 1, 0)[i]]`.
 * This roughly matches glslang's handling of the code.
 */
std::unique_ptr<Expression> RewriteIndexedSwizzle(const Context& context,
                                                  const IndexExpression& swizzle);

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
void EliminateUnreachableCode(Module& module, ProgramUsage* usage);
void EliminateUnreachableCode(Program& program);

/**
 * Eliminates empty statements in a module (Nops, or blocks holding only Nops). Not implemented for
 * Programs because Nops are harmless, but they waste space in long-lived module IR.
 */
void EliminateEmptyStatements(Module& module);

/**
 * Eliminates functions in a program which are never called. Returns true if any changes were made.
 */
bool EliminateDeadFunctions(const Context& context, Module& module, ProgramUsage* usage);
bool EliminateDeadFunctions(Program& program);

/**
 * Eliminates variables in a program which are never read or written (past their initializer).
 * Preserves side effects from initializers, if any. Returns true if any changes were made.
 */
bool EliminateDeadLocalVariables(const Context& context,
                                 Module& module,
                                 ProgramUsage* usage);
bool EliminateDeadLocalVariables(Program& program);
bool EliminateDeadGlobalVariables(const Context& context,
                                  Module& module,
                                  ProgramUsage* usage,
                                  bool onlyPrivateGlobals);
bool EliminateDeadGlobalVariables(Program& program);

/** Renames private functions and function-local variables to minimize code size. */
void RenamePrivateSymbols(Context& context, Module& module, ProgramUsage* usage, ProgramKind kind);

/** Replaces constant variables in a program with their equivalent values. */
void ReplaceConstVarsWithLiterals(Module& module, ProgramUsage* usage);

} // namespace Transform
} // namespace SkSL

#endif
