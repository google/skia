/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLLayout.h"

#include "src/sksl/SkSLThreadContext.h"

namespace SkSL {

namespace dsl {

DSLLayout& DSLLayout::flag(SkSL::Layout::Flag mask, const char* name, PositionInfo pos) {
    if (fSkSLLayout.fFlags & mask) {
        ThreadContext::ReportError("layout qualifier '" + String(name) + "' appears more than once",
                pos);
    }
    fSkSLLayout.fFlags |= mask;
    return *this;
}

DSLLayout& DSLLayout::intValue(int* target, int value, SkSL::Layout::Flag flag, const char* name,
                               PositionInfo pos) {
    this->flag(flag, name, pos);
    *target = value;
    return *this;
}

} // namespace dsl

} // namespace SkSL
