/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSFORM
#define SKSL_TRANSFORM

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/ir/SkSLModifierFlags.h"

#include <cstdint>
#include <memory>

namespace SkSL {

class Block;
class Context;
class Expression;
class IndexExpression;
class Position;
class ProgramUsage;
class SymbolTable;
class Variable;
enum class ProgramKind : int8_t;
struct Module;
struct Program;

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
 * Copies built-in structs from modules into the program. Relies on ProgramUsage to determine
 * which structs are necessary.
 */
void FindAndDeclareBuiltinStructs(Program& program);

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
 * Eliminates unnecessary braces in a module (e.g., single-statement child blocks). Not implemented
 * for Programs because extra braces are harmless, but they waste space in long-lived module IR.
 */
void EliminateUnnecessaryBraces(const Context& context, Module& module);

/**
 * Replaces splat-casts like `float4(myFloat)` with `myFloat.xxxx`. This should be slightly smaller
 * in textual form, and will be optimized back to the splat-cast at load time.
 */
void ReplaceSplatCastsWithSwizzles(const Context& context, Module& module);

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
 * Looks for variables inside of the top-level of switch-cases, such as:
 *
 *    case 1: int i;         // `i` is at top-level
 *    case 2: float f = 5.0; // `f` is at top-level, and has an initial-value assignment
 *    case 3: { bool b; }    // `b` is not at top-level; it has an additional scope
 *
 * If any top-level variables are found, a scoped block is created and returned which holds the
 * variable declarations from the switch-cases and into the outer scope. (Variables with additional
 * scoping are left as-is.) Then, we replace the declarations with nops or assignment statements.
 * That is, we would return a Block like this:
 *
 *    {
 *        int i;
 *        float f;
 *    }
 *
 * And we would also mutate the passed-in case statements to eliminate the variable decarations:
 *
 *    case 1: Nop;         // `i` is declared in the returned block and needs no initialization
 *    case 2: f = 5.0;     // `f` is declared in the returned block and initialized here
 *    case 3: { bool b; }  // `b` is left as-is because it has a block-scope
 *
 * This doesn't change the meaning or correctness of the code. If the switch needs to be rewriten
 * (e.g. due to the restrictions of ES2 or WGSL), this transformation prevents scoping issues with
 * variables falling out of scope between switch-cases when we fall through.
 *
 * If there are no variables at the top-level, null is returned.
 */
std::unique_ptr<Block> HoistSwitchVarDeclarationsAtTopLevel(const Context&, StatementArray& cases,
                                                            SymbolTable& symbolTable, Position pos);

} // namespace Transform
} // namespace SkSL

#endif
