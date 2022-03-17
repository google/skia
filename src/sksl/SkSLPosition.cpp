/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLPosition.h"

#include "src/sksl/SkSLThreadContext.h"
#include <algorithm>

namespace SkSL {

int Position::line(std::string_view source) const {
    SkASSERT(this->valid());
    if (fEndOffset == -1) {
        return fStartOffsetOrLine;
    }
    SkASSERT(source.data());
    // we allow the offset to equal the length, because that's where TK_END_OF_FILE is reported
    SkASSERT(fStartOffsetOrLine <= (int)source.length());
    int offset = std::min(fStartOffsetOrLine, (int)source.length());
    int line = 1;
    for (int i = 0; i < offset; i++) {
        if (source[i] == '\n') {
            ++line;
        }
    }
    return line;
}

} // namespace SkSL
