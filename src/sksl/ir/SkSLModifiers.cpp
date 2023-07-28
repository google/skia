/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLModifiers.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

bool Modifiers::checkPermittedFlags(const Context& context,
                                    Position pos,
                                    ModifierFlags permittedModifierFlags) const {
    static constexpr struct { ModifierFlag flag; const char* name; } kModifierFlags[] = {
        { ModifierFlag::kConst,          "const" },
        { ModifierFlag::kIn,             "in" },
        { ModifierFlag::kOut,            "out" },
        { ModifierFlag::kUniform,        "uniform" },
        { ModifierFlag::kFlat,           "flat" },
        { ModifierFlag::kNoPerspective,  "noperspective" },
        { ModifierFlag::kPure,           "$pure" },
        { ModifierFlag::kInline,         "inline" },
        { ModifierFlag::kNoInline,       "noinline" },
        { ModifierFlag::kHighp,          "highp" },
        { ModifierFlag::kMediump,        "mediump" },
        { ModifierFlag::kLowp,           "lowp" },
        { ModifierFlag::kExport,         "$export" },
        { ModifierFlag::kES3,            "$es3" },
        { ModifierFlag::kWorkgroup,      "workgroup" },
        { ModifierFlag::kReadOnly,       "readonly" },
        { ModifierFlag::kWriteOnly,      "writeonly" },
        { ModifierFlag::kBuffer,         "buffer" },
    };

    bool success = true;
    ModifierFlags modifierFlags = fFlags;
    for (const auto& f : kModifierFlags) {
        if (modifierFlags & f.flag) {
            if (!(permittedModifierFlags & f.flag)) {
                context.fErrors->error(pos, "'" + std::string(f.name) + "' is not permitted here");
                success = false;
            }
            modifierFlags &= ~f.flag;
        }
    }
    SkASSERT(modifierFlags == ModifierFlag::kNone);

    return success;
}

} // namespace SkSL
