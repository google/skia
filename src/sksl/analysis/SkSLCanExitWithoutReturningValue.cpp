/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

namespace SkSL {
namespace {

class ReturnsOnAllPathsVisitor : public ProgramVisitor {
public:
    bool visitExpression(const Expression& expr) override {
        // We can avoid processing expressions entirely.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            // Returns, breaks, or continues will stop the scan, so only one of these should ever be
            // true.
            case Statement::Kind::kReturn:
                fFoundReturn = true;
                return true;

            case Statement::Kind::kBreak:
                fFoundBreak = true;
                return true;

            case Statement::Kind::kContinue:
                fFoundContinue = true;
                return true;

            case Statement::Kind::kIf: {
                const IfStatement& i = stmt.as<IfStatement>();
                ReturnsOnAllPathsVisitor trueVisitor;
                ReturnsOnAllPathsVisitor falseVisitor;
                trueVisitor.visitStatement(*i.ifTrue());
                if (i.ifFalse()) {
                    falseVisitor.visitStatement(*i.ifFalse());
                }
                // If either branch leads to a break or continue, we report the entire if as
                // containing a break or continue, since we don't know which side will be reached.
                fFoundBreak    = (trueVisitor.fFoundBreak    || falseVisitor.fFoundBreak);
                fFoundContinue = (trueVisitor.fFoundContinue || falseVisitor.fFoundContinue);
                // On the other hand, we only want to report returns that definitely happen, so we
                // require those to be found on both sides.
                fFoundReturn   = (trueVisitor.fFoundReturn   && falseVisitor.fFoundReturn);
                return fFoundBreak || fFoundContinue || fFoundReturn;
            }
            case Statement::Kind::kFor: {
                const ForStatement& f = stmt.as<ForStatement>();
                // We assume a for/while loop runs for at least one iteration; this isn't strictly
                // guaranteed, but it's better to be slightly over-permissive here than to fail on
                // reasonable code.
                ReturnsOnAllPathsVisitor forVisitor;
                forVisitor.visitStatement(*f.statement());
                // A for loop that contains a break or continue is safe; it won't exit the entire
                // function, just the loop. So we disregard those signals.
                fFoundReturn = forVisitor.fFoundReturn;
                return fFoundReturn;
            }
            case Statement::Kind::kDo: {
                const DoStatement& d = stmt.as<DoStatement>();
                // Do-while blocks are always entered at least once.
                ReturnsOnAllPathsVisitor doVisitor;
                doVisitor.visitStatement(*d.statement());
                // A do-while loop that contains a break or continue is safe; it won't exit the
                // entire function, just the loop. So we disregard those signals.
                fFoundReturn = doVisitor.fFoundReturn;
                return fFoundReturn;
            }
            case Statement::Kind::kBlock:
                // Blocks are definitely entered and don't imply any additional control flow.
                // If the block contains a break, continue or return, we want to keep that.
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kSwitch: {
                // Switches are the most complex control flow we need to deal with; fortunately we
                // already have good primitives for dissecting them. We need to verify that:
                // - a default case exists, so that every possible input value is covered
                // - every switch-case either (a) returns unconditionally, or
                //                            (b) falls through to another case that does
                const SwitchStatement& s = stmt.as<SwitchStatement>();
                bool foundDefault = false;
                bool fellThrough = false;
                for (const std::unique_ptr<Statement>& switchStmt : s.cases()) {
                    // The default case is indicated by a null value. A switch without a default
                    // case cannot definitively return, as its value might not be in the cases list.
                    const SwitchCase& sc = switchStmt->as<SwitchCase>();
                    if (sc.isDefault()) {
                        foundDefault = true;
                    }
                    // Scan this switch-case for any exit (break, continue or return).
                    ReturnsOnAllPathsVisitor caseVisitor;
                    caseVisitor.visitStatement(sc);

                    // If we found a break or continue, whether conditional or not, this switch case
                    // can't be called an unconditional return. Switches absorb breaks but not
                    // continues.
                    if (caseVisitor.fFoundContinue) {
                        fFoundContinue = true;
                        return false;
                    }
                    if (caseVisitor.fFoundBreak) {
                        return false;
                    }
                    // We just confirmed that there weren't any breaks or continues. If we didn't
                    // find an unconditional return either, the switch is considered fallen-through.
                    // (There might be a conditional return, but that doesn't count.)
                    fellThrough = !caseVisitor.fFoundReturn;
                }

                // If we didn't find a default case, or the very last case fell through, this switch
                // doesn't meet our criteria.
                if (fellThrough || !foundDefault) {
                    return false;
                }

                // We scanned the entire switch, found a default case, and every section either fell
                // through or contained an unconditional return.
                fFoundReturn = true;
                return true;
            }

            case Statement::Kind::kSwitchCase:
                // Recurse into the switch-case.
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kDiscard:
            case Statement::Kind::kExpression:
            case Statement::Kind::kInlineMarker:
            case Statement::Kind::kNop:
            case Statement::Kind::kVarDeclaration:
                // None of these statements could contain a return.
                break;
        }

        return false;
    }

    bool fFoundReturn = false;
    bool fFoundBreak = false;
    bool fFoundContinue = false;

    using INHERITED = ProgramVisitor;
};

}  // namespace

bool Analysis::CanExitWithoutReturningValue(const FunctionDeclaration& funcDecl,
                                            const Statement& body) {
    if (funcDecl.returnType().isVoid()) {
        return false;
    }
    ReturnsOnAllPathsVisitor visitor;
    visitor.visitStatement(body);
    return !visitor.fFoundReturn;
}

}  // namespace SkSL
