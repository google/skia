/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERS
#define SKSL_MODIFIERS

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"

namespace SkSL {

struct Modifiers {
    Position fPosition;
    SkSL::Layout fLayout;
    SkSL::ModifierFlags fFlags = ModifierFlag::kNone;
};

}  // namespace SkSL

#endif
