/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_MODIFIERS
#define SKSL_DSL_MODIFIERS

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLModifiers.h"

namespace SkSL::dsl {

struct DSLModifiers {
    SkSL::Modifiers fModifiers;
    Position fPosition;
};

}  // namespace SkSL::dsl

#endif
