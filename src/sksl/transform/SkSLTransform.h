/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSFORM
#define SKSL_TRANSFORM

#include <memory>
#include <vector>

namespace SkSL {

class Context;
struct Program;
class ProgramElement;
class ProgramUsage;
class Statement;
enum class ProgramKind : int8_t;

namespace Transform {

void FindAndDeclareBuiltinVariables(const Context& context, ProgramKind programKind,
                                    std::vector<const ProgramElement*>& sharedElements);

/**
 * Eliminates statements in a block which cannot be reached; for example, a statement
 * immediately after a `return` or `continue` can safely be eliminated.
 */
void EliminateUnreachableCode(Program& program, ProgramUsage* usage = nullptr);

/**
 * Eliminates functions in a program which are never called. Returns true if any changes were made.
 */
bool EliminateDeadFunctions(Program& program, ProgramUsage* usage);

/**
 * Eliminates variables in a program which are never read or written (past their initializer).
 * Preserves side effects from initializers, if any. Returns true if any changes were made.
 */
bool EliminateDeadLocalVariables(Program& program, ProgramUsage* usage);
bool EliminateDeadGlobalVariables(Program& program, ProgramUsage* usage);

} // namespace Transform
} // namespace SkSL

#endif
