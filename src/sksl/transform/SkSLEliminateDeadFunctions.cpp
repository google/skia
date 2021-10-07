/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>

namespace SkSL {

bool Transform::EliminateDeadFunctions(Program& program, ProgramUsage* usage) {
    bool madeChanges = false;

    if (program.fConfig->fSettings.fRemoveDeadFunctions) {
        auto isDeadFunction = [&](const ProgramElement* element) {
            if (!element->is<FunctionDefinition>()) {
                return false;
            }
            const FunctionDefinition& fn = element->as<FunctionDefinition>();
            if (fn.declaration().isMain() || usage->get(fn.declaration()) > 0) {
                return false;
            }
            usage->remove(*element);
            madeChanges = true;
            return true;
        };

        program.fOwnedElements.erase(std::remove_if(program.fOwnedElements.begin(),
                                                    program.fOwnedElements.end(),
                                                    [&](const std::unique_ptr<ProgramElement>& pe) {
                                                        return isDeadFunction(pe.get());
                                                    }),
                                     program.fOwnedElements.end());
        program.fSharedElements.erase(std::remove_if(program.fSharedElements.begin(),
                                                     program.fSharedElements.end(),
                                                     isDeadFunction),
                                      program.fSharedElements.end());
    }
    return madeChanges;
}

}  // namespace SkSL
