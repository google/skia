/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPosition.h"

#include <algorithm>

namespace SkSL {

int Position::line(std::string_view source) const {
    SkASSERT(this->valid());
    if (fStartOffset == -1) {
        return -1;
    }
    if (!source.data()) {
        return -1;
    }
    // we allow the offset to equal the length, because that's where TK_END_OF_FILE is reported
    SkASSERT(fStartOffset <= (int)source.length());
    int offset = std::min(fStartOffset, (int)source.length());
    int line = 1;
    for (int i = 0; i < offset; i++) {
        if (source[i] == '\n') {
            ++line;
        }
    }
    return line;
}

} // namespace SkSL
