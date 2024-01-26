/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <type_traits>
#include <vector>

using namespace skia_private;

namespace SkSL {

static bool contains_builtin_struct(const ProgramUsage& usage) {
    for (const auto& [symbol, count] : usage.fStructCounts) {
        const Type& type = symbol->as<Type>();
        if (type.isBuiltin()) {
            return true;
        }
    }
    return false;
}

static void get_struct_definitions_from_module(
        Program& program,
        const Module& module,
        std::vector<const ProgramElement*>* addedStructDefs) {
    // We want to start at the root module and work our way towards the Program, so that structs
    // are added to the program in the same order that they appear in the Module hierarchy.
    if (module.fParent) {
        get_struct_definitions_from_module(program, *module.fParent, addedStructDefs);
    }

    // Find StructDefinitions from this Module that are used by the program, and copy them into our
    // array of shared elements.
    for (const std::unique_ptr<ProgramElement>& elem : module.fElements) {
        if (elem->is<StructDefinition>()) {
            const StructDefinition& structDef = elem->as<StructDefinition>();
            int* structCount = program.fUsage->fStructCounts.find(&structDef.type());
            if (structCount && *structCount > 0) {
                addedStructDefs->push_back(&structDef);
            }
        }
    }
}

void Transform::FindAndDeclareBuiltinStructs(Program& program) {
    // Check if the program references any builtin structs at all.
    if (contains_builtin_struct(*program.fUsage)) {
        // Visit all of our modules to find struct definitions that were referenced by ProgramUsage.
        std::vector<const ProgramElement*> addedStructDefs;
        get_struct_definitions_from_module(program, *program.fContext->fModule, &addedStructDefs);

        // Copy the struct definitions into our shared elements, and update ProgramUsage to match.
        program.fSharedElements.insert(program.fSharedElements.begin(),
                                       addedStructDefs.begin(), addedStructDefs.end());

        for (const ProgramElement* element : addedStructDefs) {
            program.fUsage->add(*element);
        }
    }
}

}  // namespace SkSL
