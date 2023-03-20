/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariableReference.h"

#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

VariableReference::VariableReference(Position pos, const Variable* variable, RefKind refKind)
    : INHERITED(pos, kIRNodeKind, &variable->type())
    , fVariable(variable)
    , fRefKind(refKind) {
    SkASSERT(this->variable());
}

std::string VariableReference::description(OperatorPrecedence) const {
    return std::string(this->variable()->name());
}

void VariableReference::setRefKind(RefKind refKind) {
    fRefKind = refKind;
}

void VariableReference::setVariable(const Variable* variable) {
    fVariable = variable;
}

}  // namespace SkSL
