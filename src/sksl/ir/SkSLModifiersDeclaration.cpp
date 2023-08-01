/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"

#include <cstdint>

namespace SkSL {

enum class ProgramKind : int8_t;

std::unique_ptr<ModifiersDeclaration> ModifiersDeclaration::Convert(const Context& context,
                                                                    const Modifiers& modifiers) {
    SkSL::ProgramKind kind = context.fConfig->fKind;
    if (!ProgramConfig::IsFragment(kind) &&
        !ProgramConfig::IsVertex(kind)) {
        context.fErrors->error(modifiers.fPosition,
                               "layout qualifiers are not allowed in this kind of program");
        return nullptr;
    }

    return ModifiersDeclaration::Make(context, modifiers);
}

std::unique_ptr<ModifiersDeclaration> ModifiersDeclaration::Make(const Context& context,
                                                                 const Modifiers& modifiers) {
    [[maybe_unused]] SkSL::ProgramKind kind = context.fConfig->fKind;
    SkASSERT(ProgramConfig::IsFragment(kind) ||
             ProgramConfig::IsVertex(kind));

    return std::make_unique<ModifiersDeclaration>(modifiers.fPosition,
                                                  modifiers.fLayout,
                                                  modifiers.fFlags);
}

}  // namespace SkSL
