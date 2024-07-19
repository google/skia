/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

namespace SkSL {

static bool dead_function_predicate(const ProgramElement* element, ProgramUsage* usage) {
    if (!element->is<FunctionDefinition>()) {
        return false;
    }
    const FunctionDefinition& fn = element->as<FunctionDefinition>();
    if (fn.declaration().isMain() || usage->get(fn.declaration()) > 0) {
        return false;
    }
    // This function is about to be eliminated by remove_if; update ProgramUsage accordingly.
    usage->remove(*element);
    return true;
}

bool Transform::EliminateDeadFunctions(Program& program) {
    ProgramUsage* usage = program.fUsage.get();

    size_t numOwnedElements = program.fOwnedElements.size();
    size_t numSharedElements = program.fSharedElements.size();

    if (program.fConfig->fSettings.fRemoveDeadFunctions) {
        program.fOwnedElements.erase(std::remove_if(program.fOwnedElements.begin(),
                                                    program.fOwnedElements.end(),
                                                    [&](const std::unique_ptr<ProgramElement>& pe) {
                                                        return dead_function_predicate(pe.get(),
                                                                                       usage);
                                                    }),
                                     program.fOwnedElements.end());
        program.fSharedElements.erase(std::remove_if(program.fSharedElements.begin(),
                                                     program.fSharedElements.end(),
                                                     [&](const ProgramElement* pe) {
                                                         return dead_function_predicate(pe, usage);
                                                     }),
                                      program.fSharedElements.end());
    }
    return program.fOwnedElements.size() < numOwnedElements ||
           program.fSharedElements.size() < numSharedElements;
}

bool Transform::EliminateDeadFunctions(const Context& context,
                                       Module& module,
                                       ProgramUsage* usage) {
    size_t numElements = module.fElements.size();

    if (context.fConfig->fSettings.fRemoveDeadFunctions) {
        module.fElements.erase(std::remove_if(module.fElements.begin(),
                                              module.fElements.end(),
                                              [&](const std::unique_ptr<ProgramElement>& pe) {
                                                  return dead_function_predicate(pe.get(), usage);
                                              }),
                               module.fElements.end());
    }
    return module.fElements.size() < numElements;
}

}  // namespace SkSL
