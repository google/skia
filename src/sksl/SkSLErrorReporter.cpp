/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLErrorReporter.h"

#include "src/base/SkStringView.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

void ErrorReporter::error(Position position, std::string_view msg) {
    if (skstd::contains(msg, Compiler::POISON_TAG)) {
        // Don't report errors on poison values.
        return;
    }
    ++fErrorCount;
    this->handleError(msg, position);
}

void TestingOnly_AbortErrorReporter::handleError(std::string_view msg, Position pos) {
    SK_ABORT("%.*s", (int)msg.length(), msg.data());
}

} // namespace SkSL
