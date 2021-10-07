/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>

namespace SkSL {

bool Transform::EliminateDeadGlobalVariables(Program& program, ProgramUsage* usage) {
    bool madeChanges = false;

    if (program.fConfig->fSettings.fRemoveDeadVariables) {
        auto isDeadVariable = [&](const ProgramElement* element) {
            if (!element->is<GlobalVarDeclaration>()) {
                return false;
            }
            const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
            const VarDeclaration& varDecl = global.declaration()->as<VarDeclaration>();
            if (!usage->isDead(varDecl.var())) {
                return false;
            }
            madeChanges = true;
            return true;
        };

        program.fOwnedElements.erase(std::remove_if(program.fOwnedElements.begin(),
                                                    program.fOwnedElements.end(),
                                                    [&](const std::unique_ptr<ProgramElement>& pe) {
                                                        return isDeadVariable(pe.get());
                                                    }),
                                     program.fOwnedElements.end());
        program.fSharedElements.erase(std::remove_if(program.fSharedElements.begin(),
                                                     program.fSharedElements.end(),
                                                     isDeadVariable),
                                      program.fSharedElements.end());
    }
    return madeChanges;
}

}  // namespace SkSL
