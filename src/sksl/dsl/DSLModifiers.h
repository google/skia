/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_MODIFIERS
#define SKSL_DSL_MODIFIERS

#include "src/sksl/ir/SkSLModifiers.h"
namespace SkSL {

namespace dsl {

class DSLModifiers {
public:
    enum Flag {
        kNo_Flag             =       0,
        kConst_Flag          = 1 <<  0,
        kIn_Flag             = 1 <<  1,
        kOut_Flag            = 1 <<  2,
        kUniform_Flag        = 1 <<  3,
        kFlat_Flag           = 1 <<  4,
        kNoPerspective_Flag  = 1 <<  5,
    };

    DSLModifiers() {}

    DSLModifiers(Flag flags)
        : fModifiers(SkSL::Layout(), flags) {}

private:
    SkSL::Modifiers fModifiers;

    friend class DSLVar;
};

} // namespace dsl

} // namespace SkSL

#endif
