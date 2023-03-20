/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLTransform.h"

namespace SkSL {

class Expression;

const Modifiers* Transform::AddConstToVarModifiers(const Context& context,
                                                   const Variable& var,
                                                   const Expression* initialValue,
                                                   const ProgramUsage* usage) {
    // If the variable is already marked as `const`, keep our existing modifiers.
    const Modifiers* modifiers = &var.modifiers();
    if (modifiers->fFlags & Modifiers::kConst_Flag) {
        return modifiers;
    }
    // If the variable doesn't have a compile-time-constant initial value, we can't `const` it.
    if (!initialValue || !Analysis::IsCompileTimeConstant(*initialValue)) {
        return modifiers;
    }
    // This only works for variables that are written-to a single time.
    ProgramUsage::VariableCounts counts = usage->get(var);
    if (counts.fWrite != 1) {
        return modifiers;
    }
    // Add `const` to our variable's modifiers, making it eligible for constant-folding.
    Modifiers constModifiers = *modifiers;
    constModifiers.fFlags |= Modifiers::kConst_Flag;
    return context.fModifiersPool->add(constModifiers);
}

}  // namespace SkSL
