/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLModifiers.h"

#include "include/core/SkTypes.h"
#include "src/base/SkMathPriv.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

bool Modifiers::checkPermitted(const Context& context,
                               Position pos,
                               ModifierFlags permittedModifierFlags,
                               LayoutFlags permittedLayoutFlags) const {
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

    LayoutFlags backendFlags = fLayout.fFlags & LayoutFlag::kAllBackends;
    if (SkPopCount(backendFlags.value()) > 1) {
        context.fErrors->error(pos, "only one backend qualifier can be used");
        success = false;
    }

    static constexpr struct { LayoutFlag flag; const char* name; } kLayoutFlags[] = {
        { LayoutFlag::kOriginUpperLeft,          "origin_upper_left"},
        { LayoutFlag::kPushConstant,             "push_constant"},
        { LayoutFlag::kBlendSupportAllEquations, "blend_support_all_equations"},
        { LayoutFlag::kColor,                    "color"},
        { LayoutFlag::kLocation,                 "location"},
        { LayoutFlag::kOffset,                   "offset"},
        { LayoutFlag::kBinding,                  "binding"},
        { LayoutFlag::kTexture,                  "texture"},
        { LayoutFlag::kSampler,                  "sampler"},
        { LayoutFlag::kIndex,                    "index"},
        { LayoutFlag::kSet,                      "set"},
        { LayoutFlag::kBuiltin,                  "builtin"},
        { LayoutFlag::kInputAttachmentIndex,     "input_attachment_index"},
        { LayoutFlag::kSPIRV,                    "spirv"},
        { LayoutFlag::kMetal,                    "metal"},
        { LayoutFlag::kGL,                       "gl"},
        { LayoutFlag::kWGSL,                     "wgsl"},
    };

    LayoutFlags layoutFlags = fLayout.fFlags;
    if ((layoutFlags & (LayoutFlag::kTexture | LayoutFlag::kSampler)) &&
        layoutFlags & LayoutFlag::kBinding) {
        context.fErrors->error(pos, "'binding' modifier cannot coexist with 'texture'/'sampler'");
        success = false;
    }
    // The `texture` and `sampler` flags are only allowed when explicitly targeting Metal and WGSL
    if (!(layoutFlags & (LayoutFlag::kMetal | LayoutFlag::kWGSL))) {
        permittedLayoutFlags &= ~LayoutFlag::kTexture;
        permittedLayoutFlags &= ~LayoutFlag::kSampler;
    }
    // The `set` flag is not allowed when explicitly targeting Metal and GLSL. It is currently
    // allowed when no backend flag is present.
    // TODO(skia:14023): Further restrict the `set` flag to SPIR-V and WGSL
    if (layoutFlags & (LayoutFlag::kMetal | LayoutFlag::kGL)) {
        permittedLayoutFlags &= ~LayoutFlag::kSet;
    }
    // TODO(skia:14023): Restrict the `push_constant` flag to SPIR-V and WGSL

    for (const auto& lf : kLayoutFlags) {
        if (layoutFlags & lf.flag) {
            if (!(permittedLayoutFlags & lf.flag)) {
                context.fErrors->error(pos, "layout qualifier '" + std::string(lf.name) +
                                            "' is not permitted here");
                success = false;
            }
            layoutFlags &= ~lf.flag;
        }
    }
    SkASSERT(layoutFlags == LayoutFlag::kNone);
    return success;
}

} // namespace SkSL
