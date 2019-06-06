/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSITION
#define SKSL_POSITION

#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

namespace SkSL {

/**
 * Represents a position in the source code. Both line and column are one-based. Column is currently
 * ignored.
 */
struct Position {
    Position()
    : fLine(-1)
    , fColumn(-1) {}

    Position(int line, int column)
    : fLine(line)
    , fColumn(column) {}

    String description() const {
        return to_string(fLine);
    }

    int fLine;
    int fColumn;
};

} // namespace

#endif
