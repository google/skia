/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"

namespace SkSL {

std::unique_ptr<Statement> DiscardStatement::Convert(const Context& context, Position pos) {
    if (!ProgramConfig::IsFragment(context.fConfig->fKind)) {
        context.fErrors->error(pos, "discard statement is only permitted in fragment shaders");
        return nullptr;
    }
    return DiscardStatement::Make(context, pos);
}

std::unique_ptr<Statement> DiscardStatement::Make(const Context& context, Position pos) {
    SkASSERT(ProgramConfig::IsFragment(context.fConfig->fKind));
    return std::make_unique<DiscardStatement>(pos);
}

}  // namespace SkSL
