/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkEnumBitMask.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLTransform.h"

namespace SkSL {

class Expression;

ModifierFlags Transform::AddConstToVarModifiers(const Variable& var,
                                                const Expression* initialValue,
                                                const ProgramUsage* usage) {
    // If the variable is already marked as `const`, keep our existing modifiers.
    ModifierFlags flags = var.modifierFlags();
    if (flags.isConst()) {
        return flags;
    }
    // If the variable doesn't have a compile-time-constant initial value, we can't `const` it.
    if (!initialValue || !Analysis::IsCompileTimeConstant(*initialValue)) {
        return flags;
    }
    // This only works for variables that are written-to a single time.
    ProgramUsage::VariableCounts counts = usage->get(var);
    if (counts.fWrite != 1) {
        return flags;
    }
    // Add `const` to our variable's modifier flags, making it eligible for constant-folding.
    return flags | ModifierFlag::kConst;
}

}  // namespace SkSL
