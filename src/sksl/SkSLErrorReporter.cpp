/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

void ErrorReporter::error(skstd::string_view msg, PositionInfo position) {
    if (msg.contains(Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    ++fErrorCount;
    this->handleError(msg, position);
}

void ErrorReporter::error(int line, skstd::string_view msg) {
    if (msg.contains(Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    if (line == -1) {
        ++fErrorCount;
        fPendingErrors.push_back(String(msg));
    } else {
        this->error(msg, PositionInfo(/*file=*/nullptr, line));
    }
}

} // namespace SkSL
