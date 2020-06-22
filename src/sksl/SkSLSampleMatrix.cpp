/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSampleMatrix.h"

#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

SampleMatrix SampleMatrix::merge(const SampleMatrix& other) {
    if (fKind == Kind::kVariable || other.fKind == Kind::kVariable) {
        *this = SampleMatrix::MakeVariable();
        return *this;
    }
    if (other.fKind == Kind::kConstantOrUniform) {
        if (fKind == other.fKind) {
            if (fExpression == other.fExpression) {
                return *this;
            }
            *this = SampleMatrix::MakeVariable();
            return *this;
        }
        SkASSERT(fKind == Kind::kNone);
        *this = other;
        return *this;
    }
    return *this;
}

struct SampleMatrixExtractor {
    SampleMatrixExtractor(const Program& program, const Variable& fp)
            : fProgram(program), fFP(fp) {}

    SampleMatrix getMatrix(const Expression&) const;
    SampleMatrix getMatrix(const Statement&) const;

    SampleMatrix getMatrix(const Expression* e) const {
        return e ? this->getMatrix(*e) : SampleMatrix();
    }
    SampleMatrix getMatrix(const Statement* s) const {
        return s ? this->getMatrix(*s) : SampleMatrix();
    }

    SampleMatrix getMatrix(const ProgramElement& pe) const {
        if (pe.fKind == ProgramElement::kFunction_Kind) {
            return this->getMatrix(*((const FunctionDefinition&) pe).fBody);
        }
        return SampleMatrix();
    }

    const Program& fProgram;
    const Variable& fFP;
};

SampleMatrix SampleMatrix::Make(const Program& program, const Variable& fp) {
    SampleMatrix result;
    SampleMatrixExtractor extractor(program, fp);
    for (const auto& pe : program) {
        result.merge(extractor.getMatrix(pe));
    }
    return result;
}

SampleMatrix SampleMatrixExtractor::getMatrix(const Expression& e) const {
    switch (e.fKind) {
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& fc = (const FunctionCall&) e;
            const FunctionDeclaration& f = fc.fFunction;
            if (f.fBuiltin && f.fName == "sample" && fc.fArguments.size() >= 2 &&
                fc.fArguments.back()->fType == *fProgram.fContext->fFloat3x3_Type &&
                fc.fArguments[0]->fKind == Expression::kVariableReference_Kind &&
                &((VariableReference&) *fc.fArguments[0]).fVariable == &fFP) {
                if (fc.fArguments.back()->isConstantOrUniform()) {
                    return SampleMatrix::MakeConstUniform(fc.fArguments.back()->description());
                } else {
                    return SampleMatrix::MakeVariable();
                }
            }
            SampleMatrix result;
            for (const auto& e : fc.fArguments) {
                result.merge(this->getMatrix(*e));
            }
            return result;
        }
        case Expression::kConstructor_Kind: {
            SampleMatrix result;
            const Constructor& c = (const Constructor&) e;
            for (const auto& e : c.fArguments) {
                result.merge(this->getMatrix(*e));
            }
            return result;
        }
        case Expression::kFieldAccess_Kind:
            return this->getMatrix(*((const FieldAccess&) e).fBase);
        case Expression::kSwizzle_Kind:
            return this->getMatrix(*((const Swizzle&) e).fBase);
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->getMatrix(*b.fLeft).merge(
                   this->getMatrix(*b.fRight));
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) e;
            return this->getMatrix(*idx.fBase).merge(
                   this->getMatrix(*idx.fIndex));
        }
        case Expression::kPrefix_Kind:
            return this->getMatrix(*((const PrefixExpression&) e).fOperand);
        case Expression::kPostfix_Kind:
            return this->getMatrix(*((const PostfixExpression&) e).fOperand);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->getMatrix(*t.fTest).merge(
                   this->getMatrix(*t.fIfTrue)).merge(
                   this->getMatrix(*t.fIfFalse));
        }
        case Expression::kVariableReference_Kind:
            return SampleMatrix();
        case Expression::kBoolLiteral_Kind:
        case Expression::kDefined_Kind:
        case Expression::kExternalFunctionCall_Kind:
        case Expression::kExternalValue_Kind:
        case Expression::kFloatLiteral_Kind:
        case Expression::kFunctionReference_Kind:
        case Expression::kIntLiteral_Kind:
        case Expression::kNullLiteral_Kind:
        case Expression::kSetting_Kind:
        case Expression::kTypeReference_Kind:
            return SampleMatrix();
    }
    SkASSERT(false);
    return SampleMatrix();
}

SampleMatrix SampleMatrixExtractor::getMatrix(const Statement& s) const {
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            SampleMatrix result;
            for (const auto& child : ((const Block&) s).fStatements) {
                result.merge(this->getMatrix(*child));
            }
            return result;
        }
        case Statement::kVarDeclaration_Kind:
            return this->getMatrix(((const VarDeclaration&) s).fValue.get());
        case Statement::kVarDeclarations_Kind: {
            const VarDeclarations& decls = *((const VarDeclarationsStatement&) s).fDeclaration;
            SampleMatrix result;
            for (const auto& stmt : decls.fVars) {
                result.merge(this->getMatrix(*stmt));
            }
            return result;
        }
        case Statement::kExpression_Kind:
            return this->getMatrix(*((const ExpressionStatement&) s).fExpression);
        case Statement::kReturn_Kind:
            return this->getMatrix(((const ReturnStatement&) s).fExpression.get());
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->getMatrix(*i.fTest).merge(
                   this->getMatrix(*i.fIfTrue)).merge(
                   this->getMatrix(i.fIfFalse.get()));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return this->getMatrix(f.fInitializer.get()).merge(
                   this->getMatrix(f.fTest.get()).merge(
                   this->getMatrix(f.fNext.get()).merge(
                   this->getMatrix(*f.fStatement))));
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->getMatrix(*w.fTest).merge(
                   this->getMatrix(*w.fStatement));
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->getMatrix(*d.fTest).merge(
                   this->getMatrix(*d.fStatement));
        }
        case Statement::kSwitch_Kind: {
            SampleMatrix result;
            const SwitchStatement& sw = (const SwitchStatement&) s;
            for (const auto& c : sw.fCases) {
                for (const auto& st : c->fStatements) {
                    result.merge(this->getMatrix(*st));
                }
            }
            return result.merge(this->getMatrix(*sw.fValue));
        }
        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
        case Statement::kNop_Kind:
            return SampleMatrix();
    }
    SkASSERT(false);
    return SampleMatrix();
}

} // namespace
