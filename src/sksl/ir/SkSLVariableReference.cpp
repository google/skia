/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariableReference.h"

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLSetting.h"

namespace SkSL {

VariableReference::VariableReference(int offset, const Variable* variable, RefKind refKind)
    : INHERITED(offset, kExpressionKind, &variable->type())
    , fVariable(variable)
    , fRefKind(refKind) {
    SkASSERT(this->variable());
}

bool VariableReference::hasProperty(Property property) const {
    switch (property) {
        case Property::kSideEffects:      return false;
        case Property::kContainsRTAdjust: return this->variable()->name() == "sk_RTAdjust";
        default:
            SkASSERT(false);
            return false;
    }
}

bool VariableReference::isConstantOrUniform() const {
    return (this->variable()->modifiers().fFlags & Modifiers::kUniform_Flag) != 0;
}

String VariableReference::description() const {
    return this->variable()->name();
}

void VariableReference::setRefKind(RefKind refKind) {
    fRefKind = refKind;
}

void VariableReference::setVariable(const Variable* variable) {
    fVariable = variable;
}

}  // namespace SkSL
