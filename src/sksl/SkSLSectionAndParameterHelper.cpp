/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSectionAndParameterHelper.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

SectionAndParameterHelper::SectionAndParameterHelper(const Program* program, ErrorReporter& errors)
    : fProgram(*program) {
    for (const auto& p : fProgram) {
        switch (p.fKind) {
            case ProgramElement::kVar_Kind: {
                const VarDeclarations& decls = (const VarDeclarations&) p;
                for (const auto& raw : decls.fVars) {
                    const VarDeclaration& decl = (VarDeclaration&) *raw;
                    if (IsParameter(*decl.fVar)) {
                        fParameters.push_back(decl.fVar);
                    }
                }
                break;
            }
            case ProgramElement::kSection_Kind: {
                const Section& s = (const Section&) p;
                if (IsSupportedSection(s.fName.c_str())) {
                    if (SectionRequiresArgument(s.fName.c_str()) && !s.fArgument.size()) {
                        errors.error(s.fOffset,
                                     ("section '@" + s.fName +
                                      "' requires one parameter").c_str());
                    }
                    if (!SectionAcceptsArgument(s.fName.c_str()) && s.fArgument.size()) {
                        errors.error(s.fOffset,
                                     ("section '@" + s.fName + "' has no parameters").c_str());
                    }
                } else {
                    errors.error(s.fOffset,
                                 ("unsupported section '@" + s.fName + "'").c_str());
                }
                if (!SectionPermitsDuplicates(s.fName.c_str()) &&
                        fSections.find(s.fName) != fSections.end()) {
                    errors.error(s.fOffset,
                                 ("duplicate section '@" + s.fName + "'").c_str());
                }
                fSections[s.fName].push_back(&s);
                break;
            }
            default:
                break;
        }
    }
}

bool SectionAndParameterHelper::hasCoordOverrides(const Variable& fp) {
    for (const auto& pe : fProgram) {
        if (this->hasCoordOverrides(pe, fp)) {
            return true;
        }
    }
    return false;
}

bool SectionAndParameterHelper::hasCoordOverrides(const ProgramElement& pe, const Variable& fp) {
    if (pe.fKind == ProgramElement::kFunction_Kind) {
        return this->hasCoordOverrides(*((const FunctionDefinition&) pe).fBody, fp);
    }
    return false;
}

bool SectionAndParameterHelper::hasCoordOverrides(const Expression& e, const Variable& fp) {
    switch (e.fKind) {
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& fc = (const FunctionCall&) e;
            const FunctionDeclaration& f = fc.fFunction;
            if (f.fBuiltin && f.fName == "sample" && fc.fArguments.size() >= 2 &&
                fc.fArguments.back()->fType == *fProgram.fContext->fFloat2_Type &&
                fc.fArguments[0]->fKind == Expression::kVariableReference_Kind &&
                &((VariableReference&) *fc.fArguments[0]).fVariable == &fp) {
                return true;
            }
            for (const auto& e : fc.fArguments) {
                if (this->hasCoordOverrides(*e, fp)) {
                    return true;
                }
            }
            return false;
        }
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) e;
            for (const auto& e : c.fArguments) {
                if (this->hasCoordOverrides(*e, fp)) {
                    return true;
                }
            }
            return false;
        }
        case Expression::kFieldAccess_Kind: {
            return this->hasCoordOverrides(*((const FieldAccess&) e).fBase, fp);
        }
        case Expression::kSwizzle_Kind:
            return this->hasCoordOverrides(*((const Swizzle&) e).fBase, fp);
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->hasCoordOverrides(*b.fLeft, fp) ||
                   this->hasCoordOverrides(*b.fRight, fp);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) e;
            return this->hasCoordOverrides(*idx.fBase, fp) ||
                   this->hasCoordOverrides(*idx.fIndex, fp);
        }
        case Expression::kPrefix_Kind:
            return this->hasCoordOverrides(*((const PrefixExpression&) e).fOperand, fp);
        case Expression::kPostfix_Kind:
            return this->hasCoordOverrides(*((const PostfixExpression&) e).fOperand, fp);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->hasCoordOverrides(*t.fTest, fp) ||
                   this->hasCoordOverrides(*t.fIfTrue, fp) ||
                   this->hasCoordOverrides(*t.fIfFalse, fp);
        }
        case Expression::kVariableReference_Kind:
            return false;
        case Expression::kAppendStage_Kind:
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
            return false;
    }
    SkASSERT(false);
    return false;
}

bool SectionAndParameterHelper::hasCoordOverrides(const Statement& s, const Variable& fp) {
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            for (const auto& child : ((const Block&) s).fStatements) {
                if (this->hasCoordOverrides(*child, fp)) {
                    return true;
                }
            }
            return false;
        }
        case Statement::kVarDeclaration_Kind: {
            const VarDeclaration& var = (const VarDeclaration&) s;
            if (var.fValue) {
                return hasCoordOverrides(*var.fValue, fp);
            }
            return false;
        }
        case Statement::kVarDeclarations_Kind: {
            const VarDeclarations& decls = *((const VarDeclarationsStatement&) s).fDeclaration;
            for (const auto& stmt : decls.fVars) {
                if (this->hasCoordOverrides(*stmt, fp)) {
                    return true;
                }
            }
            return false;
        }
        case Statement::kExpression_Kind:
            return this->hasCoordOverrides(*((const ExpressionStatement&) s).fExpression, fp);
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = (const ReturnStatement&) s;
            if (r.fExpression) {
                return this->hasCoordOverrides(*r.fExpression, fp);
            }
            return false;
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->hasCoordOverrides(*i.fTest, fp) ||
                   this->hasCoordOverrides(*i.fIfTrue, fp) ||
                   (i.fIfFalse && this->hasCoordOverrides(*i.fIfFalse, fp));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return this->hasCoordOverrides(*f.fInitializer, fp) ||
                   this->hasCoordOverrides(*f.fTest, fp) ||
                   this->hasCoordOverrides(*f.fNext, fp) ||
                   this->hasCoordOverrides(*f.fStatement, fp);
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->hasCoordOverrides(*w.fTest, fp) ||
                   this->hasCoordOverrides(*w.fStatement, fp);
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->hasCoordOverrides(*d.fTest, fp) ||
                   this->hasCoordOverrides(*d.fStatement, fp);
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& sw = (const SwitchStatement&) s;
            for (const auto& c : sw.fCases) {
                for (const auto& st : c->fStatements) {
                    if (this->hasCoordOverrides(*st, fp)) {
                        return true;
                    }
                }
            }
            return this->hasCoordOverrides(*sw.fValue, fp);
        }
        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
        case Statement::kGroup_Kind:
        case Statement::kNop_Kind:
            return false;
    }
    SkASSERT(false);
    return false;
}

}
