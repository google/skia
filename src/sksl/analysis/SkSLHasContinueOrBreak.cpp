/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"

namespace SkSL {

class Expression;

namespace Analysis {
namespace {

class HasContinueOrBreakVisitor : public ProgramVisitor {
public:
    HasContinueOrBreakVisitor() {}

    bool visitExpression(const Expression& expr) override {
        // We can avoid processing expressions entirely.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kContinue:
                fResult.fHasContinue = true;
                return fResult.fHasContinue && fResult.fHasBreak;

            case Statement::Kind::kBreak:
                fResult.fHasBreak = true;
                return fResult.fHasContinue && fResult.fHasBreak;

            case Statement::Kind::kFor:
            case Statement::Kind::kDo:
            case Statement::Kind::kSwitch: {
                // A continue or break inside of a for-statement or do-statement would not affect
                // the control flow of the outer statement; they would affect the inner loop
                // instead. A break inside of a switch-case statement escapes the switch, and SkSL
                // does not allow continues inside of a switch at all, so we don't need to visit
                // switches either.
                return false;
            }

            default:
                return ProgramVisitor::visitStatement(stmt);
        }
    }

    ContinueOrBreakInfo fResult;
};

}  // namespace

ContinueOrBreakInfo HasContinueOrBreak(const Statement& stmt) {
    HasContinueOrBreakVisitor visitor;
    visitor.visitStatement(stmt);
    return visitor.fResult;
}

}  // namespace Analysis
}  // namespace SkSL
