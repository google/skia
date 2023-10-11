/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSFORM
#define SKSL_TRANSFORM

#include "include/core/SkSpan.h"
#include "src/sksl/ir/SkSLModifierFlags.h"

#include <memory>
#include <vector>

namespace SkSL {

class Context;
class Expression;
class IndexExpression;
struct Module;
struct Program;
class ProgramElement;
class ProgramUsage;
class Statement;
class SwitchStatement;
class Variable;
enum class ProgramKind : int8_t;

namespace Transform {

/**
 * Checks to see if it would be safe to add `const` to the modifier flags of a variable. If so,
 * returns the modifiers with `const` applied; if not, returns the existing modifiers as-is. Adding
 * `const` allows the inliner to fold away more values and generate tighter code.
 */
ModifierFlags AddConstToVarModifiers(const Variable& var,
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

/**
 * Looks for variables inside of the top-level of a switch body, such as:
 *
 *    switch (x) {
 *        case 1: int i;         // `i` is at top-level
 *        case 2: float f = 5.0; // `f` is at top-level, and has an initial-value assignment
 *        case 3: { bool b; }    // `b` is not at top-level; it has an additional scope
 *    }
 *
 * If any top-level variables are found, a scoped block is created around the switch, and the
 * variable declarations are moved out of the switch body and into the outer scope. (Variables with
 * additional scoping are left as-is.) Then, we replace the declarations with assignment statements:
 *
 *    {
 *        int i;
 *        float f;
 *        switch (a) {
 *            case 1:              // `i` is declared above and does not need initialization
 *            case 2: f = 5.0;     // `f` is declared above and initialized here
 *            case 3: { bool b; }  // `b` is left as-is because it has a block-scope
 *        }
 *    }
 *
 * This doesn't change the meaning or correctness of the code. If the switch needs to be rewriten
 * (e.g. due to the restrictions of ES2 or WGSL), this transformation prevents scoping issues with
 * variables falling out of scope between switch-cases when we fall through.
 *
 * If there are no variables at the top-level, the switch statement is returned as-is.
 */
std::unique_ptr<Statement> HoistSwitchVarDeclarationsAtTopLevel(const Context&,
                                                                std::unique_ptr<SwitchStatement>);

} // namespace Transform
} // namespace SkSL

#endif
