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
    SkASSERT(fVariable);
    fVariable->referenceCreated(fRefKind);
}

VariableReference::~VariableReference() {
    fVariable->referenceDestroyed(fRefKind);
}

bool VariableReference::hasProperty(Property property) const {
    switch (property) {
        case Property::kSideEffects:      return false;
        case Property::kContainsRTAdjust: return fVariable->name() == "sk_RTAdjust";
        default:
            SkASSERT(false);
            return false;
    }
}

bool VariableReference::isConstantOrUniform() const {
    return (fVariable->modifiers().fFlags & Modifiers::kUniform_Flag) != 0;
}

String VariableReference::description() const {
    return fVariable->name();
}

void VariableReference::setRefKind(RefKind refKind) {
    fVariable->referenceDestroyed(fRefKind);
    fRefKind = refKind;
    fVariable->referenceCreated(fRefKind);
}

void VariableReference::setVariable(const Variable* variable) {
    fVariable->referenceDestroyed(fRefKind);
    fVariable = variable;
    fVariable->referenceCreated(fRefKind);
}

std::unique_ptr<Expression> VariableReference::constantPropagate(const IRGenerator& irGenerator,
                                                                 const DefinitionMap& definitions) {
    if (fRefKind != kRead_RefKind) {
        return nullptr;
    }
    const Expression* initialValue = fVariable->initialValue();
    if ((fVariable->modifiers().fFlags & Modifiers::kConst_Flag) && initialValue &&
        initialValue->isCompileTimeConstant() &&
        this->type().typeKind() != Type::TypeKind::kArray) {
        return initialValue->clone();
    }
    auto exprIter = definitions.find(fVariable);
    if (exprIter != definitions.end() && exprIter->second &&
        (*exprIter->second)->isCompileTimeConstant()) {
        return (*exprIter->second)->clone();
    }
    return nullptr;
}

}  // namespace SkSL
