/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_MODIFIERS
#define SKSL_DSL_MODIFIERS

#include "include/private/SkSLModifiers.h"
#include "include/private/SkTArray.h"
#include "include/sksl/DSLLayout.h"

namespace SkSL {

namespace dsl {

class DSLField;
class DSLType;

enum Modifier {
    kNo_Modifier            =       0,
    kConst_Modifier         = 1 <<  0,
    kIn_Modifier            = 1 <<  1,
    kOut_Modifier           = 1 <<  2,
    kInOut_Modifier         = kIn_Modifier | kOut_Modifier,
    kUniform_Modifier       = 1 <<  3,
    kFlat_Modifier          = 1 <<  4,
    kNoPerspective_Modifier = 1 <<  5,
};

class DSLModifiers {
public:
    DSLModifiers(int flags = 0)
        : DSLModifiers(DSLLayout(), flags) {}

    DSLModifiers(DSLLayout layout, int flags = 0)
        : fModifiers(layout.fSkSLLayout, flags) {}

    int flags() const {
        return fModifiers.fFlags;
    }

private:
    SkSL::Modifiers fModifiers;

    friend DSLType Struct(skstd::string_view name, SkTArray<DSLField> fields);
    friend class DSLFunction;
    friend class DSLVar;
    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL

#endif
