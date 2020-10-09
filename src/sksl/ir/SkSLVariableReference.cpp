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
        : INHERITED(offset, VariableReferenceData{variable, refKind}) {
    SkASSERT(this->variable());
    this->variable()->referenceCreated(refKind);
}

VariableReference::~VariableReference() {
    this->variable()->referenceDestroyed(this->refKind());
}

const Type& VariableReference::type() const {
    return this->variableReferenceData().fVariable->type();
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
    this->variable()->referenceDestroyed(this->refKind());
    this->variableReferenceData().fRefKind = refKind;
    this->variable()->referenceCreated(this->refKind());
}

void VariableReference::setVariable(const Variable* variable) {
    this->variable()->referenceDestroyed(this->refKind());
    this->variableReferenceData().fVariable = variable;
    this->variable()->referenceCreated(this->refKind());
}

std::unique_ptr<Expression> VariableReference::constantPropagate(const IRGenerator& irGenerator,
                                                                 const DefinitionMap& definitions) {
    if (this->refKind() != RefKind::kRead) {
        return nullptr;
    }
    const Expression* initialValue = this->variable()->initialValue();
    if ((this->variable()->modifiers().fFlags & Modifiers::kConst_Flag) && initialValue &&
        initialValue->isCompileTimeConstant() &&
        this->type().typeKind() != Type::TypeKind::kArray) {
        return initialValue->clone();
    }
    std::unique_ptr<Expression>** exprPPtr = definitions.find(this->variable());
    if (exprPPtr && *exprPPtr && (**exprPPtr)->isCompileTimeConstant()) {
        return (**exprPPtr)->clone();
    }
    return nullptr;
}

}  // namespace SkSL
