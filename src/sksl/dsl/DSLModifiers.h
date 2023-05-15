/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_MODIFIERS
#define SKSL_DSL_MODIFIERS

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifiers.h"

namespace SkSL {

class Parser;

namespace dsl {

class DSLType;

enum Modifier {
    kNo_Modifier            = SkSL::Modifiers::kNo_Flag,
    kConst_Modifier         = SkSL::Modifiers::kConst_Flag,
    kIn_Modifier            = SkSL::Modifiers::kIn_Flag,
    kOut_Modifier           = SkSL::Modifiers::kOut_Flag,
    kInOut_Modifier         = SkSL::Modifiers::kIn_Flag | SkSL::Modifiers::kOut_Flag,
    kUniform_Modifier       = SkSL::Modifiers::kUniform_Flag,
    kFlat_Modifier          = SkSL::Modifiers::kFlat_Flag,
    kNoPerspective_Modifier = SkSL::Modifiers::kNoPerspective_Flag,
};

class DSLModifiers {
public:
    DSLModifiers(int flags = 0, Position pos = {})
        : DSLModifiers(SkSL::Layout(), flags, pos) {}

    DSLModifiers(SkSL::Layout layout, int flags = 0, Position pos = {})
        : fModifiers(layout, flags)
        , fPosition(pos) {}

    int& flags() {
        return fModifiers.fFlags;
    }

    const int& flags() const {
        return fModifiers.fFlags;
    }

    const SkSL::Layout& layout() const {
        return fModifiers.fLayout;
    }

    const SkSL::Modifiers* pool() const {
        return ThreadContext::Modifiers(fModifiers);
    }

private:
    SkSL::Modifiers fModifiers;
    Position fPosition;

    friend class DSLFunction;
    friend class DSLType;
    friend class DSLWriter;
    friend class ::SkSL::Parser;
};

} // namespace dsl

} // namespace SkSL

#endif
