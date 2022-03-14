/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"

#include "include/private/SkStringView.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

void ErrorReporter::error(std::string_view msg, Position position) {
    if (skstd::contains(msg, Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    ++fErrorCount;
    this->handleError(msg, position);
}

void ErrorReporter::error(Position pos, std::string_view msg) {
    if (skstd::contains(msg, Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    if (pos.valid()) {
        this->error(msg, pos);
    } else {
        ++fErrorCount;
        fPendingErrors.push_back(std::string(msg));
    }
}

void ErrorReporter::reportPendingErrors(Position pos) {
    for (const std::string& msg : fPendingErrors) {
        this->handleError(msg, pos);
    }
    fPendingErrors.clear();
}

void TestingOnly_AbortErrorReporter::handleError(std::string_view msg, Position pos) {
    SK_ABORT("%.*s", (int)msg.length(), msg.data());
}

} // namespace SkSL
