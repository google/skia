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

void ErrorReporter::error(const char* msg, PositionInfo position) {
    if (strstr(msg, Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    ++fErrorCount;
    this->handleError(msg, position);
}

void ErrorReporter::error(int offset, const char* msg) {
    if (strstr(msg, Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    if (offset == -1) {
        ++fErrorCount;
        fPendingErrors.push_back(std::string(msg));
    } else {
        this->error(msg, this->position(offset));
    }
}

PositionInfo ErrorReporter::position(int offset) const {
    if (fSource && offset >= 0) {
        int line = 1;
        for (int i = 0; i < offset; i++) {
            if (fSource[i] == '\n') {
                ++line;
            }
        }
        return PositionInfo(/*file=*/nullptr, line);
    } else {
        return PositionInfo();
    }
}

} // namespace SkSL
