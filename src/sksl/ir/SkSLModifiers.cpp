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
                               int permittedLayoutFlags) const {
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

    int backendFlags = fLayout.fFlags & Layout::kAllBackendFlagsMask;
    if (SkPopCount(backendFlags) > 1) {
        context.fErrors->error(pos, "only one backend qualifier can be used");
        success = false;
    }

    static constexpr struct { Layout::Flag flag; const char* name; } kLayoutFlags[] = {
        { Layout::kOriginUpperLeft_Flag,          "origin_upper_left"},
        { Layout::kPushConstant_Flag,             "push_constant"},
        { Layout::kBlendSupportAllEquations_Flag, "blend_support_all_equations"},
        { Layout::kColor_Flag,                    "color"},
        { Layout::kLocation_Flag,                 "location"},
        { Layout::kOffset_Flag,                   "offset"},
        { Layout::kBinding_Flag,                  "binding"},
        { Layout::kTexture_Flag,                  "texture"},
        { Layout::kSampler_Flag,                  "sampler"},
        { Layout::kIndex_Flag,                    "index"},
        { Layout::kSet_Flag,                      "set"},
        { Layout::kBuiltin_Flag,                  "builtin"},
        { Layout::kInputAttachmentIndex_Flag,     "input_attachment_index"},
        { Layout::kSPIRV_Flag,                    "spirv"},
        { Layout::kMetal_Flag,                    "metal"},
        { Layout::kGL_Flag,                       "gl"},
        { Layout::kWGSL_Flag,                     "wgsl"},
    };

    int layoutFlags = fLayout.fFlags;
    if ((layoutFlags & (Layout::kTexture_Flag | Layout::kSampler_Flag)) &&
        layoutFlags & Layout::kBinding_Flag) {
        context.fErrors->error(pos, "'binding' modifier cannot coexist with 'texture'/'sampler'");
        success = false;
    }
    // The `texture` and `sampler` flags are only allowed when explicitly targeting Metal and WGSL
    if (!(layoutFlags & (Layout::kMetal_Flag | Layout::kWGSL_Flag))) {
        permittedLayoutFlags &= ~Layout::kTexture_Flag;
        permittedLayoutFlags &= ~Layout::kSampler_Flag;
    }
    // The `set` flag is not allowed when explicitly targeting Metal and GLSL. It is currently
    // allowed when no backend flag is present.
    // TODO(skia:14023): Further restrict the `set` flag to SPIR-V and WGSL
    if (layoutFlags & (Layout::kMetal_Flag | Layout::kGL_Flag)) {
        permittedLayoutFlags &= ~Layout::kSet_Flag;
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
    SkASSERT(layoutFlags == 0);
    return success;
}

} // namespace SkSL
