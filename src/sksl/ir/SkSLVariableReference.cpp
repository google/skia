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
    this->incrementRefs();
}

VariableReference::~VariableReference() {
    this->decrementRefs();
}

void VariableReference::incrementRefs() const {
    if (fRefKind != kRead_RefKind) {
        fVariable->fWriteCount++;
    }
    if (fRefKind != kWrite_RefKind) {
        fVariable->fReadCount++;
    }
}

void VariableReference::decrementRefs() const {
    if (fRefKind != kRead_RefKind) {
        fVariable->fWriteCount--;
    }
    if (fRefKind != kWrite_RefKind) {
        fVariable->fReadCount--;
    }
}

void VariableReference::setRefKind(RefKind refKind) {
    this->decrementRefs();
    fRefKind = refKind;
    this->incrementRefs();
}

std::unique_ptr<Expression> VariableReference::constantPropagate(const IRGenerator& irGenerator,
                                                                 const DefinitionMap& definitions) {
    if (fRefKind != kRead_RefKind) {
        return nullptr;
    }
    if ((fVariable->fModifiers.fFlags & Modifiers::kConst_Flag) && fVariable->fInitialValue &&
        fVariable->fInitialValue->isCompileTimeConstant() &&
        this->type().typeKind() != Type::TypeKind::kArray) {
        return fVariable->fInitialValue->clone();
    }
    auto exprIter = definitions.find(fVariable);
    if (exprIter != definitions.end() && exprIter->second &&
        (*exprIter->second)->isCompileTimeConstant()) {
        return (*exprIter->second)->clone();
    }
    return nullptr;
}

}  // namespace SkSL
