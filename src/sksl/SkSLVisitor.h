/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAM_UTILS
#define SKSL_PROGRAM_UTILS

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

/**
 * ProgramVisitor is a utility to recurse through the various elements, statements, and expressions
 * in a parsed SkSL program. Each typed visit() function returns a bool that indicates whether or
 * not to terminate the recursion (true implies stopping). All leaf expressions and statements by
 * default just return false. All aggregate elements, statements, and expressions recurse to the
 * children in lexical order, stopping if a child returns true, which is then propogated up.
 *
 * Subclasses can extend the visit functions they are particularly interested in and can invoke the
 * super function manually to recurse to the types children. Alternatively, they can skip recursion
 * or do so manually. If a visitor needs to track complex information for the result of a program,
 * it is intended that they store it as a private member that is returned after visit(program) ends.
 */
class ProgramVisitor {
public:
    virtual ~ProgramVisitor() { SkASSERT(!fProgram); }

    bool visit(const Program& program) {
        fProgram = &program;
        for (const auto& pe : program) {
            if (this->visitProgramElement(pe)) {
                fProgram = nullptr;
                return true;
            }
        }
        fProgram = nullptr;
        return false;
    }

protected:
    const Program& program() const {
        SkASSERT(fProgram);
        return *fProgram;
    }

    bool visitProgramElement(const ProgramElement& pe) {
        switch(pe.fKind) {
            case ProgramElement::kEnum_Kind:
                return this->visitEnum((const Enum&) pe);
            case ProgramElement::kExtension_Kind:
                return this->visitExtension((const Extension&) pe);
            case ProgramElement::kFunction_Kind:
                return this->visitFunctionDefinition((const FunctionDefinition&) pe);
            case ProgramElement::kInterfaceBlock_Kind:
                return this->visitInterfaceBlock((const InterfaceBlock&) pe);
            case ProgramElement::kModifiers_Kind:
                return this->visitModifiers((const ModifiersDeclaration&) pe);
            case ProgramElement::kSection_Kind:
                return this->visitSection((const Section&) pe);
            case ProgramElement::kVar_Kind:
                return this->visitVars((const VarDeclarations&) pe);
            default:
                SkUNREACHABLE;
        }
    }

    bool visitStatement(const Statement& s) {
        switch(s.fKind) {
            case Statement::kBlock_Kind:
                return this->visitBlock((const Block&) s);
            case Statement::kBreak_Kind:
                return this->visitBreak((const BreakStatement&) s);
            case Statement::kContinue_Kind:
                return this->visitContinue((const ContinueStatement&) s);
            case Statement::kDiscard_Kind:
                return this->visitDiscard((const DiscardStatement&) s);
            case Statement::kDo_Kind:
                return this->visitDo((const DoStatement&) s);
            case Statement::kExpression_Kind:
                return this->visitExpressionStmt((const ExpressionStatement&) s);
            case Statement::kFor_Kind:
                return this->visitFor((const ForStatement&) s);
            case Statement::kIf_Kind:
                return this->visitIf((const IfStatement&) s);
            case Statement::kNop_Kind:
                return this->visitNop((const Nop&) s);
            case Statement::kReturn_Kind:
                return this->visitReturn((const ReturnStatement&) s);
            case Statement::kSwitch_Kind:
                return this->visitSwitch((const SwitchStatement&) s);
            case Statement::kVarDeclaration_Kind:
                return this->visitVarDeclaration((const VarDeclaration&) s);
            case Statement::kVarDeclarations_Kind:
                return this->visitDeclarations((const VarDeclarationsStatement&) s);
            case Statement::kWhile_Kind:
                return this->visitWhile((const WhileStatement&) s);
            default:
                SkUNREACHABLE;
        }
    }

    bool visitExpression(const Expression& e) {
        switch(e.fKind) {
            case Expression::kBinary_Kind:
                return this->visitBinary((const BinaryExpression&) e);
            case Expression::kBoolLiteral_Kind:
                return this->visitBool((const BoolLiteral&) e);
            case Expression::kConstructor_Kind:
                return this->visitConstructor((const Constructor&) e);
            case Expression::kDefined_Kind:
                return false; // defined is internal use only
            case Expression::kExternalFunctionCall_Kind:
                return this->visitExternalCall((const ExternalFunctionCall&) e);
            case Expression::kExternalValue_Kind:
                return this->visitExternalValue((const ExternalValueReference&) e);
            case Expression::kFieldAccess_Kind:
                return this->visitFieldAccess((const FieldAccess&) e);
            case Expression::kFloatLiteral_Kind:
                return this->visitFloat((const FloatLiteral&) e);
            case Expression::kFunctionCall_Kind:
                return this->visitFunctionCall((const FunctionCall&) e);
            case Expression::kFunctionReference_Kind:
                return this->visitFunctionReference((const FunctionReference&) e);
            case Expression::kIndex_Kind:
                return this->visitIndex((const IndexExpression&) e);
            case Expression::kIntLiteral_Kind:
                return this->visitInt((const IntLiteral&) e);
            case Expression::kNullLiteral_Kind:
                return this->visitNull((const NullLiteral&) e);
            case Expression::kPostfix_Kind:
                return this->visitPostfix((const PostfixExpression&) e);
            case Expression::kPrefix_Kind:
                return this->visitPrefix((const PrefixExpression&) e);
            case Expression::kSetting_Kind:
                return this->visitSetting((const Setting&) e);
            case Expression::kSwizzle_Kind:
                return this->visitSwizzle((const Swizzle&) e);
            case Expression::kTernary_Kind:
                return this->visitTernary((const TernaryExpression&) e);
            case Expression::kTypeReference_Kind:
                return this->visitTypeReference((const TypeReference&) e);
            case Expression::kVariableReference_Kind:
                return this->visitVarReference((const VariableReference&) e);
            default:
                SkUNREACHABLE;
        }
    }

    // ProgramElements
    virtual bool visitEnum(const Enum&) { return false; }
    virtual bool visitExtension(const Extension&) { return false; }
    virtual bool visitFunctionDefinition(const FunctionDefinition& func) {
        return this->visitStatement(*func.fBody);
    }
    virtual bool visitInterfaceBlock(const InterfaceBlock& interface) {
        for (const auto& e : interface.fSizes) {
            if (this->visitExpression(*e)) {
                return true;
            }
        }
        return false;
    }
    virtual bool visitModifiers(const ModifiersDeclaration&) { return false; }
    virtual bool visitSection(const Section&) { return false; }
    virtual bool visitVars(const VarDeclarations& vars) {
        for (const auto& v : vars.fVars) {
            if (this->visitStatement(*v)) {
                return true;
            }
        }
        return false;
    }

    // Statements
    virtual bool visitBlock(const Block& b) {
        for (const auto& s : b.fStatements) {
            if (this->visitStatement(*s)) {
                return true;
            }
        }
        return false;
    }
    virtual bool visitBreak(const BreakStatement&) { return false; }
    virtual bool visitContinue(const ContinueStatement&) { return false; }
    virtual bool visitDiscard(const DiscardStatement&) { return false; }
    virtual bool visitDo(const DoStatement& doLoop) {
        return this->visitExpression(*doLoop.fTest) || this->visitStatement(*doLoop.fStatement);
    }
    virtual bool visitExpressionStmt(const ExpressionStatement& e) {
        return this->visitExpression(*e.fExpression);
    }
    virtual bool visitFor(const ForStatement& forLoop) {
        return this->visitStatement(*forLoop.fInitializer) ||
               this->visitExpression(*forLoop.fTest) ||
               this->visitExpression(*forLoop.fNext);
    }
    virtual bool visitIf(const IfStatement& ifStmt) {
        return this->visitExpression(*ifStmt.fTest) ||
               this->visitStatement(*ifStmt.fIfTrue) ||
               (ifStmt.fIfFalse && this->visitStatement(*ifStmt.fIfFalse));
    }
    virtual bool visitNop(const Nop&) { return false; }
    virtual bool visitReturn(const ReturnStatement& returnStmt) {
        return returnStmt.fExpression && this->visitExpression(*returnStmt.fExpression);
    }
    virtual bool visitSwitch(const SwitchStatement& switchStmt) {
        if (this->visitExpression(*switchStmt.fValue)) {
            return true;
        }
        for (const auto& c : switchStmt.fCases) {
            if (c->fValue && this->visitExpression(*c->fValue)) {
                return true;
            }
            for (const auto& s : c->fStatements) {
                if (this->visitStatement(*s)) {
                    return true;
                }
            }
        }
        return false;
    }
    virtual bool visitVarDeclaration(const VarDeclaration& var) {
        for (const auto& s : var.fSizes) {
            if (this->visitExpression(*s)) {
                return true;
            }
        }
        return var.fValue && this->visitExpression(*var.fValue);
    }
    virtual bool visitDeclarations(const VarDeclarationsStatement& vars) {
        return this->visitProgramElement(*vars.fDeclaration);
    }
    virtual bool visitWhile(const WhileStatement& whileLoop) {
        return this->visitExpression(*whileLoop.fTest) ||
               this->visitStatement(*whileLoop.fStatement);
    }

    // Expressions
    virtual bool visitBinary(const BinaryExpression& be) {
        return this->visitExpression(*be.fLeft) || this->visitExpression(*be.fRight);
    }
    virtual bool visitBool(const BoolLiteral&) { return false; }
    virtual bool visitConstructor(const Constructor& ctor) {
        for (const auto& arg : ctor.fArguments) {
            if (this->visitExpression(*arg)) {
                return true;
            }
        }
        return false;
    }
    virtual bool visitExternalCall(const ExternalFunctionCall& call) {
        for (const auto& arg : call.fArguments) {
            if (this->visitExpression(*arg)) {
                return true;
            }
        }
        return false;
    }
    virtual bool visitExternalValue(const ExternalValueReference&) { return false; }
    virtual bool visitInt(const IntLiteral&) { return false; }
    virtual bool visitFieldAccess(const FieldAccess&) { return false; }
    virtual bool visitFloat(const FloatLiteral&) { return false; }
    virtual bool visitFunctionReference(const FunctionReference&) { return false; }
    virtual bool visitFunctionCall(const FunctionCall& call) {
        for (const auto& arg : call.fArguments) {
            if (this->visitExpression(*arg)) {
                return true;
            }
        }
        return false;
    }
    virtual bool visitIndex(const IndexExpression& index) {
        return this->visitExpression(*index.fBase) || this->visitExpression(*index.fIndex);
    }
    virtual bool visitNull(const NullLiteral&) { return false; }
    virtual bool visitPrefix(const PrefixExpression& e) {
        return this->visitExpression(*e.fOperand);
    }
    virtual bool visitPostfix(const PostfixExpression& e) {
        return this->visitExpression(*e.fOperand);
    }
    virtual bool visitSetting(const Setting&) { return false; }
    virtual bool visitSwizzle(const Swizzle& swizzle) {
        return this->visitExpression(*swizzle.fBase);
    }
    virtual bool visitTernary(const TernaryExpression& ternary) {
        return this->visitExpression(*ternary.fTest) ||
               this->visitExpression(*ternary.fIfTrue) ||
               this->visitExpression(*ternary.fIfFalse);
    }
    virtual bool visitTypeReference(const TypeReference&) { return false; }
    virtual bool visitVarReference(const VariableReference&) { return false; }

private:
    const Program* fProgram = nullptr; // Only set while inside visit(Program)
};

}

#endif
