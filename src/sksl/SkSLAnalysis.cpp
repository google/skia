/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"

// ProgramElements
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

// Statements
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

// Expressions
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalValueReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace {

static bool is_sample_call_to_fp(const FunctionCall& fc, const Variable& fp) {
    const FunctionDeclaration& f = fc.fFunction;
    return f.fBuiltin && f.fName == "sample" && fc.fArguments.size() >= 1 &&
            fc.fArguments[0]->fKind == Expression::kVariableReference_Kind &&
            &((VariableReference&) *fc.fArguments[0]).fVariable == &fp;
}

// Visitor that determines the merged SampleUsage for a given child 'fp' in the program.
class MergeSampleUsageVisitor : public ProgramVisitor {
public:
    MergeSampleUsageVisitor(const Variable& fp) : fFP(fp) {}

    SampleUsage visit(const Program& program) {
        fUsage = SampleUsage(); // reset to none
        this->INHERITED::visit(program);
        return fUsage;
    }

protected:
    const Variable& fFP;
    SampleUsage fUsage;

    bool visitExpression(const Expression& e) override {
        // Looking for sample(fp, inColor?, ...)
        if (e.fKind == Expression::kFunctionCall_Kind) {
            const FunctionCall& fc = (const FunctionCall&) e;
            if (is_sample_call_to_fp(fc, fFP)) {
                // Determine the type of call at this site, and merge it with the accumulated state
                const Expression* lastArg = fc.fArguments.back().get();
                const Context& context = *this->program().fContext;

                if (lastArg->fType == *context.fFloat2_Type) {
                    fUsage.merge(SampleUsage::Explicit());
                } else if (lastArg->fType == *context.fFloat3x3_Type) {
                    // Determine the type of matrix for this call site
                    if (lastArg->isConstantOrUniform()) {
                        if (lastArg->fKind == Expression::Kind::kVariableReference_Kind ||
                            lastArg->fKind == Expression::Kind::kConstructor_Kind) {
                            // FIXME if this is a constant, we should parse the float3x3 constructor
                            // and determine if the resulting matrix introduces perspective.
                            fUsage.merge(SampleUsage::UniformMatrix(lastArg->description()));
                        } else {
                            // FIXME this is really to workaround a restriction of the downstream
                            // code that relies on the SampleUsage's fExpression to identify uniform
                            // names. Once they are tracked separately, any uniform expression can
                            // work, but right now this avoids issues from '0.5 * matrix' that is
                            // both a constant AND a uniform.
                            fUsage.merge(SampleUsage::VariableMatrix());
                        }
                    } else {
                        fUsage.merge(SampleUsage::VariableMatrix());
                    }
                } else {
                    // The only other signatures do pass-through sampling
                    fUsage.merge(SampleUsage::PassThrough());
                }
                // NOTE: we don't return true here just because we found a sample call. We need to
                //  process the entire program and merge across all encountered calls.
            }
        }

        return this->INHERITED::visitExpression(e);
    }

    typedef ProgramVisitor INHERITED;
};

// Visitor that searches through a main function of the program for reference to the
// sample coordinates provided by the parent FP or main program.
class SampleCoordsVisitor : public ProgramVisitor {
protected:
    // Only bother recursing through the main function for the sample coord builtin
    bool visitProgramElement(const ProgramElement& pe) override {
        if (pe.fKind == ProgramElement::kFunction_Kind) {
            // Both kFragmentProcessor and kPipelineStage types use the first argument for
            // the main coords builtin. If that isn't in the signature, there's no need to
            // recurse deeper.
            const FunctionDeclaration& func = ((const FunctionDefinition&) pe).fDeclaration;
            if (func.fName == "main" && func.fParameters.size() >= 1 &&
                func.fParameters.front()->fType == *this->program().fContext->fFloat2_Type) {
                return this->INHERITED::visitProgramElement(pe);
            }
        }
        // No recursion, but returning false will allow visitor to continue to siblings
        return false;
    }

    bool visitExpression(const Expression& e) override {
        if (e.fKind == Expression::kVariableReference_Kind) {
            const VariableReference& var = (const VariableReference&) e;
            return var.fVariable.fModifiers.fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
        }
        return this->INHERITED::visitExpression(e);
    }

    typedef ProgramVisitor INHERITED;
};

}

////////////////////////////////////////////////////////////////////////////////
// Analysis

SampleUsage Analysis::GetSampleUsage(const Program& program, const Variable& fp) {
    MergeSampleUsageVisitor visitor(fp);
    return visitor.visit(program);
}

bool Analysis::ReferencesSampleCoords(const Program& program) {
    SampleCoordsVisitor visitor;
    return visitor.visit(program);
}

////////////////////////////////////////////////////////////////////////////////
// ProgramVisitor

bool ProgramVisitor::visit(const Program& program) {
    fProgram = &program;
    bool result = false;
    for (const auto& pe : program) {
        if (this->visitProgramElement(pe)) {
            result = true;
            break;
        }
    }
    fProgram = nullptr;
    return result;
}

bool ProgramVisitor::visitExpression(const Expression& e) {
    switch(e.fKind) {
        case Expression::kBoolLiteral_Kind:
        case Expression::kDefined_Kind:
        case Expression::kExternalValue_Kind:
        case Expression::kFieldAccess_Kind:
        case Expression::kFloatLiteral_Kind:
        case Expression::kFunctionReference_Kind:
        case Expression::kIntLiteral_Kind:
        case Expression::kNullLiteral_Kind:
        case Expression::kSetting_Kind:
        case Expression::kTypeReference_Kind:
        case Expression::kVariableReference_Kind:
            // Leaf expressions return false
            return false;
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->visitExpression(*b.fLeft) || this->visitExpression(*b.fRight); }
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) e;
            for (const auto& arg : c.fArguments) {
                if (this->visitExpression(*arg)) { return true; }
            }
            return false; }
        case Expression::kExternalFunctionCall_Kind: {
            const ExternalFunctionCall& c = (const ExternalFunctionCall&) e;
            for (const auto& arg : c.fArguments) {
                if (this->visitExpression(*arg)) { return true; }
            }
            return false; }
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& c = (const FunctionCall&) e;
            for (const auto& arg : c.fArguments) {
                if (this->visitExpression(*arg)) { return true; }
            }
            return false; }
        case Expression::kIndex_Kind:{
            const IndexExpression& i = (const IndexExpression&) e;
            return this->visitExpression(*i.fBase) || this->visitExpression(*i.fIndex); }
        case Expression::kPostfix_Kind:
            return this->visitExpression(*((const PostfixExpression&) e).fOperand);
        case Expression::kPrefix_Kind:
            return this->visitExpression(*((const PrefixExpression&) e).fOperand);
        case Expression::kSwizzle_Kind:
            return this->visitExpression(*((const Swizzle&) e).fBase);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->visitExpression(*t.fTest) ||
                   this->visitExpression(*t.fIfTrue) ||
                   this->visitExpression(*t.fIfFalse); }
        default:
            SkUNREACHABLE;
    }
}

bool ProgramVisitor::visitStatement(const Statement& s) {
    switch(s.fKind) {
        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
        case Statement::kNop_Kind:
            // Leaf statements just return false
            return false;
        case Statement::kBlock_Kind:
            for (const auto& s : ((const Block&) s).fStatements) {
                if (this->visitStatement(*s)) { return true; }
            }
            return false;
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->visitExpression(*d.fTest) || this->visitStatement(*d.fStatement); }
        case Statement::kExpression_Kind:
            return this->visitExpression(*((const ExpressionStatement&) s).fExpression);
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return (f.fInitializer && this->visitStatement(*f.fInitializer)) ||
                   (f.fInitializer && this->visitExpression(*f.fTest)) ||
                   (f.fNext && this->visitExpression(*f.fNext)) ||
                   this->visitStatement(*f.fStatement); }
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->visitExpression(*i.fTest) ||
                   this->visitStatement(*i.fIfTrue) ||
                   (i.fIfFalse && this->visitStatement(*i.fIfFalse)); }
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = (const ReturnStatement&) s;
            return r.fExpression && this->visitExpression(*r.fExpression); }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& sw = (const SwitchStatement&) s;
            if (this->visitExpression(*sw.fValue)) { return true; }
            for (const auto& c : sw.fCases) {
                if (c->fValue && this->visitExpression(*c->fValue)) { return true; }
                for (const auto& st : c->fStatements) {
                    if (this->visitStatement(*st)) { return true; }
                }
            }
            return false; }
        case Statement::kVarDeclaration_Kind: {
            const VarDeclaration& v = (const VarDeclaration&) s;
            for (const auto& s : v.fSizes) {
                if (this->visitExpression(*s)) { return true; }
            }
            return v.fValue && this->visitExpression(*v.fValue); }
        case Statement::kVarDeclarations_Kind: {
            // Technically this statement points to a program element, but it's convenient
            // to have program element > statement > expression, so visit the declaration elements
            // directly without going up to visitProgramElement.
            const VarDeclarations& vars = *((const VarDeclarationsStatement&) s).fDeclaration;
            for (const auto& v : vars.fVars) {
                if (this->visitStatement(*v)) { return true; }
            }
            return false;
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->visitExpression(*w.fTest) || this->visitStatement(*w.fStatement); }
        default:
            SkUNREACHABLE;
    }
}

bool ProgramVisitor::visitProgramElement(const ProgramElement& pe) {
    switch(pe.fKind) {
        case ProgramElement::kEnum_Kind:
        case ProgramElement::kExtension_Kind:
        case ProgramElement::kModifiers_Kind:
        case ProgramElement::kSection_Kind:
            // Leaf program elements just return false by default
            return false;
        case ProgramElement::kFunction_Kind:
            return this->visitStatement(*((const FunctionDefinition&) pe).fBody);
        case ProgramElement::kInterfaceBlock_Kind:
            for (const auto& e : ((const InterfaceBlock&) pe).fSizes) {
                if (this->visitExpression(*e)) { return true; }
            }
            return false;
        case ProgramElement::kVar_Kind:
            for (const auto& v : ((const VarDeclarations&) pe).fVars) {
                if (this->visitStatement(*v)) { return true; }
            }
            return false;
        default:
            SkUNREACHABLE;
    }
}

}
