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

VariableReference::VariableReference(int offset, const Variable& variable, RefKind refKind)
: INHERITED(offset, kExpressionKind, &variable.type())
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
    SkASSERT(expr->isCompileTimeConstant());
    switch (expr->kind()) {
        case Expression::Kind::kIntLiteral:
            return std::make_unique<IntLiteral>(irGenerator.fContext,
                                                expr->fOffset,
                                                expr->as<IntLiteral>().fValue);
        case Expression::Kind::kFloatLiteral:
            return std::make_unique<FloatLiteral>(irGenerator.fContext,
                                                  expr->fOffset,
                                                  expr->as<FloatLiteral>().fValue);
        case Expression::Kind::kBoolLiteral:
            return std::make_unique<BoolLiteral>(irGenerator.fContext,
                                                 expr->fOffset,
                                                 expr->as<BoolLiteral>().fValue);
        case Expression::Kind::kPrefix: {
            const PrefixExpression& prefix = expr->as<PrefixExpression>();
            return std::make_unique<PrefixExpression>(
                    prefix.fOperator, copy_constant(irGenerator, prefix.fOperand.get()));
        }
        case Expression::Kind::kConstructor: {
            const Constructor& c = expr->as<Constructor>();
            std::vector<std::unique_ptr<Expression>> args;
            args.reserve(c.fArguments.size());
            for (const auto& arg : c.fArguments) {
                args.push_back(copy_constant(irGenerator, arg.get()));
            }
            return std::make_unique<Constructor>(c.fOffset, &c.type(), std::move(args));
        }
        case Expression::Kind::kSetting: {
            const Setting& s = expr->as<Setting>();
            return std::make_unique<Setting>(s.fOffset, s.fName,
                                             copy_constant(irGenerator, s.fValue.get()));
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
    if ((fVariable.fModifiers.fFlags & Modifiers::kConst_Flag) && fVariable.fInitialValue &&
        fVariable.fInitialValue->isCompileTimeConstant() &&
        this->type().typeKind() != Type::TypeKind::kArray) {
        return copy_constant(irGenerator, fVariable.fInitialValue);
    }
    auto exprIter = definitions.find(&fVariable);
    if (exprIter != definitions.end() && exprIter->second &&
        (*exprIter->second)->isCompileTimeConstant()) {
        return copy_constant(irGenerator, exprIter->second->get());
    }
    return nullptr;
}

}  // namespace SkSL
