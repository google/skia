/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLLayout.h"

#include "src/sksl/SkSLThreadContext.h"

#include <string>

namespace SkSL {

namespace dsl {

DSLLayout& DSLLayout::flag(SkSL::Layout::Flag mask, const char* name, Position pos) {
    if (fSkSLLayout.fFlags & mask) {
        ThreadContext::ReportError("layout qualifier '" + std::string(name) +
                                   "' appears more than once", pos);
    }
    fSkSLLayout.fFlags |= mask;
    return *this;
}

DSLLayout& DSLLayout::intValue(int* target, int value, SkSL::Layout::Flag flag, const char* name,
                               Position pos) {
    this->flag(flag, name, pos);
    *target = value;
    return *this;
}

} // namespace dsl

} // namespace SkSL
