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
void EliminateUnreachableCode(std::unique_ptr<Statement>& stmt, ProgramUsage* usage = nullptr);

} // namespace Transform
} // namespace SkSL

#endif
