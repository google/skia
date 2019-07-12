/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariableReference.h"

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLSetting.h"

namespace SkSL {

VariableReference::VariableReference(IRGenerator* irGenerator, int offset, IRNode::ID variable,
                                     RefKind refKind)
: INHERITED(irGenerator, offset, kVariableReference_Kind, ((Variable&) variable.node()).fType)
, fVariable(variable)
, fRefKind(refKind) {}

void VariableReference::setRefKind(RefKind refKind) {
    fRefKind = refKind;
}

IRNode::ID VariableReference::CopyConstant(IRGenerator* irGenerator,
                                           const Expression& expr) {
    SkASSERT(expr.isConstant());
    switch (expr.fKind) {
        case Expression::kIntLiteral_Kind:
            return irGenerator->createNode(new IntLiteral(irGenerator, -1,
                                                          ((IntLiteral&) expr).fValue));
        case Expression::kFloatLiteral_Kind:
            return irGenerator->createNode(new FloatLiteral(irGenerator, -1,
                                                            ((FloatLiteral&) expr).fValue));
        case Expression::kBoolLiteral_Kind:
            return irGenerator->createNode(new BoolLiteral(irGenerator, -1,
                                                           ((BoolLiteral&) expr).fValue));
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) expr;
            std::vector<IRNode::ID> args;
            for (const auto& arg : c.fArguments) {
                args.push_back(CopyConstant(irGenerator, arg.expressionNode()));
            }
            return irGenerator->createNode(new Constructor(irGenerator, -1, c.fType,
                                                           std::move(args)));
        }
        case Expression::kSetting_Kind: {
            const Setting& s = (const Setting&) expr;
            return irGenerator->createNode(new Setting(irGenerator, -1, s.fName,
                          CopyConstant(irGenerator, s.fValue.expressionNode())));
        }
        default:
            ABORT("unsupported constant\n");
    }
}

IRNode::ID VariableReference::constantPropagate(const DefinitionMap& definitions) {
    if (fRefKind != kRead_RefKind) {
        return IRNode::ID();
    }
    Variable& var = (Variable&) fVariable.node();
    if (fIRGenerator->fKind == Program::kPipelineStage_Kind &&
        var.fStorage == Variable::kGlobal_Storage &&
        (var.fModifiers.fFlags & Modifiers::kIn_Flag) &&
        !(var.fModifiers.fFlags & Modifiers::kUniform_Flag)) {
        return fIRGenerator->getArg(fOffset, ((Variable&) fVariable.node()).fName);
    }
    if ((var.fModifiers.fFlags & Modifiers::kConst_Flag) && var.fInitialValue &&
        var.fInitialValue.expressionNode().isConstant()) {
        return CopyConstant(fIRGenerator, var.fInitialValue.expressionNode());
    }
    auto exprIter = definitions.find(fVariable);
    if (exprIter != definitions.end() && exprIter->second) {
        Expression& expr = exprIter->second.expressionNode();
        if (expr.isConstant()) {
            return CopyConstant(fIRGenerator, expr);
        }
    }
    return IRNode::ID();
}

} // namespace
