/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

class Expression;

namespace Analysis {
namespace {

class LoopControlFlowVisitor : public ProgramVisitor {
public:
    LoopControlFlowVisitor() {}

    bool visitExpression(const Expression& expr) override {
        // We can avoid processing expressions entirely.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kContinue:
                // A continue only affects the control flow of the loop if it's not nested inside
                // another looping structure. (Inside a switch, SkSL disallows continue entirely.)
                fResult.fHasContinue |= (fDepth == 0);
                break;

            case Statement::Kind::kBreak:
                // A break only affects the control flow of the loop if it's not nested inside
                // another loop/switch structure.
                fResult.fHasBreak |= (fDepth == 0);
                break;

            case Statement::Kind::kReturn:
                // A return will abort the loop's control flow no matter how deeply it is nested.
                fResult.fHasReturn = true;
                break;

            case Statement::Kind::kFor:
            case Statement::Kind::kDo:
            case Statement::Kind::kSwitch: {
                ++fDepth;
                bool done = ProgramVisitor::visitStatement(stmt);
                --fDepth;
                return done;
            }

            default:
                return ProgramVisitor::visitStatement(stmt);
        }

        // If we've already found everything we're hunting for, we can stop searching early.
        return fResult.fHasContinue && fResult.fHasBreak && fResult.fHasReturn;
    }

    LoopControlFlowInfo fResult;
    int fDepth = 0;
};

}  // namespace

LoopControlFlowInfo GetLoopControlFlowInfo(const Statement& stmt) {
    LoopControlFlowVisitor visitor;
    visitor.visitStatement(stmt);
    return visitor.fResult;
}

}  // namespace Analysis
}  // namespace SkSL
