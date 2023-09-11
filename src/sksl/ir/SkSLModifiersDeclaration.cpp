/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLModifiersDeclaration.h"

#include "include/private/base/SkAssert.h"
#include "src/base/SkEnumBitMask.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLModifiers.h"

#include <cstdint>

namespace SkSL {

enum class ProgramKind : int8_t;

std::unique_ptr<ModifiersDeclaration> ModifiersDeclaration::Convert(const Context& context,
                                                                    const Modifiers& modifiers) {
    SkSL::ProgramKind kind = context.fConfig->fKind;
    if (!ProgramConfig::IsFragment(kind) && !ProgramConfig::IsVertex(kind) &&
        !ProgramConfig::IsCompute(kind)) {
        context.fErrors->error(modifiers.fPosition,
                               "layout qualifiers are not allowed in this kind of program");
        return nullptr;
    }

    if (modifiers.fLayout.fLocalSizeX >= 0 ||
        modifiers.fLayout.fLocalSizeY >= 0 ||
        modifiers.fLayout.fLocalSizeZ >= 0) {
        if (modifiers.fLayout.fLocalSizeX == 0 ||
            modifiers.fLayout.fLocalSizeY == 0 ||
            modifiers.fLayout.fLocalSizeZ == 0) {
            context.fErrors->error(modifiers.fPosition, "local size qualifiers cannot be zero");
            return nullptr;
        }
        if (!ProgramConfig::IsCompute(kind)) {
            context.fErrors->error(
                    modifiers.fPosition,
                    "local size layout qualifiers are only allowed in a compute program");
            return nullptr;
        }
        if (modifiers.fFlags != ModifierFlag::kIn) {
            context.fErrors->error(
                    modifiers.fPosition,
                    "local size layout qualifiers must be defined using an 'in' declaration");
            return nullptr;
        }
    }

    return ModifiersDeclaration::Make(context, modifiers);
}

std::unique_ptr<ModifiersDeclaration> ModifiersDeclaration::Make(const Context& context,
                                                                 const Modifiers& modifiers) {
    [[maybe_unused]] SkSL::ProgramKind kind = context.fConfig->fKind;
    SkASSERT(ProgramConfig::IsFragment(kind) || ProgramConfig::IsVertex(kind) ||
             ProgramConfig::IsCompute(kind));

    return std::make_unique<ModifiersDeclaration>(modifiers.fPosition,
                                                  modifiers.fLayout,
                                                  modifiers.fFlags);
}

}  // namespace SkSL
