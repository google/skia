/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <memory>
#include <tuple>

namespace SkSL {

class Context;

static bool compare(const SkSL::Variable* a, const SkSL::Variable* b) {
    int setA = a->modifiers().fLayout.fSet;
    int setB = b->modifiers().fLayout.fSet;
    int bindingA = a->modifiers().fLayout.fBinding;
    int bindingB = b->modifiers().fLayout.fBinding;
    return std::tie(setA, bindingA) < std::tie(setB, bindingB);
}

SkTArray<const SkSL::Variable*> Analysis::GetComputeShaderMainParams(const Context& context,
        const Program& program) {
    SkTArray<const SkSL::Variable*> result;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& var =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fLayout.fBinding != -1 && var.modifiers().fLayout.fSet != -1) {
                result.push_back(&var);
            }
        }
    }
    std::sort(result.begin(), result.end(), compare);
    return result;
}

}  // namespace SkSL
