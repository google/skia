/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLModifierFlags.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

std::string ModifierFlags::paddedDescription() const {
    // SkSL extensions
    std::string result;
    if (*this & ModifierFlag::kExport) {
        result += "$export ";
    }
    if (*this & ModifierFlag::kES3) {
        result += "$es3 ";
    }
    if (*this & ModifierFlag::kPure) {
        result += "$pure ";
    }
    if (*this & ModifierFlag::kInline) {
        result += "inline ";
    }
    if (*this & ModifierFlag::kNoInline) {
        result += "noinline ";
    }

    // Real GLSL qualifiers (must be specified in order in GLSL 4.1 and below)
    if (*this & ModifierFlag::kFlat) {
        result += "flat ";
    }
    if (*this & ModifierFlag::kNoPerspective) {
        result += "noperspective ";
    }
    if (*this & ModifierFlag::kConst) {
        result += "const ";
    }
    if (*this & ModifierFlag::kUniform) {
        result += "uniform ";
    }
    if ((*this & ModifierFlag::kIn) && (*this & ModifierFlag::kOut)) {
        result += "inout ";
    } else if (*this & ModifierFlag::kIn) {
        result += "in ";
    } else if (*this & ModifierFlag::kOut) {
        result += "out ";
    }
    if (*this & ModifierFlag::kHighp) {
        result += "highp ";
    }
    if (*this & ModifierFlag::kMediump) {
        result += "mediump ";
    }
    if (*this & ModifierFlag::kLowp) {
        result += "lowp ";
    }
    if (*this & ModifierFlag::kReadOnly) {
        result += "readonly ";
    }
    if (*this & ModifierFlag::kWriteOnly) {
        result += "writeonly ";
    }
    if (*this & ModifierFlag::kBuffer) {
        result += "buffer ";
    }

    // We're using non-GLSL names for these.
    if (*this & ModifierFlag::kPixelLocal) {
        // Roughly equivalent to `__pixel_localEXT`.
        result += "pixel_local ";
    }
    if (*this & ModifierFlag::kWorkgroup) {
        // Equivalent to `shared`.
        result += "workgroup ";
    }

    return result;
}

std::string ModifierFlags::description() const {
    std::string s = this->paddedDescription();
    if (!s.empty()) {
        s.pop_back();
    }
    return s;
}

bool ModifierFlags::checkPermittedFlags(const Context& context,
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
        { ModifierFlag::kPixelLocal,     "pixel_local" },
    };

    bool success = true;
    ModifierFlags modifierFlags = *this;
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
