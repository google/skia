/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLVariableReference.h"

#include "SkSLConstructor.h"
#include "SkSLFloatLiteral.h"
#include "SkSLIRGenerator.h"
#include "SkSLSetting.h"

namespace SkSL {

VariableReference::VariableReference(int offset, const Variable& variable, RefKind refKind)
: INHERITED(offset, kVariableReference_Kind, variable.fType)
, fVariable(variable)
, fRefKind(refKind) {
    if (refKind != kRead_RefKind) {
        fVariable.fWriteCount++;
    }
    if (refKind != kWrite_RefKind) {
        fVariable.fReadCount++;
    }
}

VariableReference::~VariableReference() {
    if (fRefKind != kRead_RefKind) {
        fVariable.fWriteCount--;
    }
    if (fRefKind != kWrite_RefKind) {
        fVariable.fReadCount--;
    }
}

void VariableReference::setRefKind(RefKind refKind) {
    if (fRefKind != kRead_RefKind) {
        fVariable.fWriteCount--;
    }
    if (fRefKind != kWrite_RefKind) {
        fVariable.fReadCount--;
    }
    if (refKind != kRead_RefKind) {
        fVariable.fWriteCount++;
    }
    if (refKind != kWrite_RefKind) {
        fVariable.fReadCount++;
    }
    fRefKind = refKind;
}

std::unique_ptr<Expression> VariableReference::copy_constant(const IRGenerator& irGenerator,
                                                             const Expression* expr) {
    SkASSERT(expr->isConstant());
    switch (expr->fKind) {
        case Expression::kIntLiteral_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(irGenerator.fContext,
                                                              -1,
                                                              ((IntLiteral*) expr)->fValue));
        case Expression::kFloatLiteral_Kind:
            return std::unique_ptr<Expression>(new FloatLiteral(
                                                               irGenerator.fContext,
                                                               -1,
                                                               ((FloatLiteral*) expr)->fValue));
        case Expression::kBoolLiteral_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(irGenerator.fContext,
                                                               -1,
                                                               ((BoolLiteral*) expr)->fValue));
        case Expression::kConstructor_Kind: {
            const Constructor* c = (const Constructor*) expr;
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : c->fArguments) {
                args.push_back(copy_constant(irGenerator, arg.get()));
            }
            return std::unique_ptr<Expression>(new Constructor(-1, c->fType,
                                                               std::move(args)));
        }
        case Expression::kSetting_Kind: {
            const Setting* s = (const Setting*) expr;
            return std::unique_ptr<Expression>(new Setting(-1, s->fName,
                                                           copy_constant(irGenerator,
                                                                         s->fValue.get())));
        }
        default:
            ABORT("unsupported constant\n");
    }
}

std::unique_ptr<Expression> VariableReference::constantPropagate(const IRGenerator& irGenerator,
                                                                 const DefinitionMap& definitions) {
    if (fRefKind != kRead_RefKind) {
        return nullptr;
    }
    if (irGenerator.fKind == Program::kPipelineStage_Kind &&
        fVariable.fStorage == Variable::kGlobal_Storage &&
        (fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) &&
        !(fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag)) {
        return irGenerator.getArg(fOffset, fVariable.fName);
    }
    if ((fVariable.fModifiers.fFlags & Modifiers::kConst_Flag) && fVariable.fInitialValue &&
        fVariable.fInitialValue->isConstant()) {
        return copy_constant(irGenerator, fVariable.fInitialValue);
    }
    auto exprIter = definitions.find(&fVariable);
    if (exprIter != definitions.end() && exprIter->second &&
        (*exprIter->second)->isConstant()) {
        return copy_constant(irGenerator, exprIter->second->get());
    }
    return nullptr;
}

} // namespace
