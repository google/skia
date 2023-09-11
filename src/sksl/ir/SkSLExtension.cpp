/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLExtension.h"

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"

namespace SkSL {

std::unique_ptr<Extension> Extension::Convert(const Context& context,
                                              Position pos,
                                              std::string_view name,
                                              std::string_view behaviorText) {
    if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        // Runtime Effects do not allow any #extensions.
        context.fErrors->error(pos, "unsupported directive '#extension'");
        return nullptr;
    }
    if (behaviorText == "disable") {
        // We allow `#extension <name> : disable`, but it is a no-op.
        return nullptr;
    }
    if (behaviorText != "require" && behaviorText != "enable" && behaviorText != "warn") {
        context.fErrors->error(pos, "expected 'require', 'enable', 'warn', or 'disable'");
        return nullptr;
    }
    // We don't currently do anything different between `require`, `enable`, and `warn`.
    return Extension::Make(context, pos, name);
}

std::unique_ptr<Extension> Extension::Make(const Context& context,
                                           Position pos,
                                           std::string_view name) {
    SkASSERT(!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind));
    return std::make_unique<SkSL::Extension>(pos, name);
}

}  // namespace SkSL
